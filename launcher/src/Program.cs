/*
 * Copyright © Windower Dev Team
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files
 * (the "Software"),to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

namespace Windower
{
    using CommandLine;
    using Core;
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Diagnostics.CodeAnalysis;
    using System.IO;
    using System.IO.Pipes;
    using System.Linq;
    using System.Net;
    using System.Reflection;
    using System.Runtime.ExceptionServices;
    using System.Runtime.Serialization.Formatters.Binary;
    using System.Text;
    using System.Threading;
    using System.Threading.Tasks;
    using Windower.UI;

    using static System.FormattableString;

    /// <summary>
    /// The class containing the program entry point.
    /// </summary>
    [SuppressMessage("Microsoft.Maintainability", "CA1506")]
    public static class Program
    {
        public static bool IsMono => Type.GetType("Mono.Runtime") != null;
        public static string BuildTag => GetBuildTag();

        public static async Task<TResult> RemoteCallAsync<TResult>(Func<CancellationToken, TResult> method,
            CancellationToken token) =>
            (TResult)await RemoteCallAsync(false, method, token);

        public static async Task ElevateAsync<T1>(Action<T1, CancellationToken> method, T1 arg1, CancellationToken token) =>
            await RemoteCallAsync(true, method, token, arg1);

        public static async Task<TResult> ElevateAsync<T1, T2, T3, TResult>(Func<T1, T2, T3, CancellationToken, TResult> method,
            T1 arg1, T2 arg2, T3 arg3, CancellationToken token) =>
            (TResult)await RemoteCallAsync(true, method, token, arg1, arg2, arg3);

        /// <summary>
        /// The program entry point.
        /// </summary>
        private static void Main(string[] args)
        {
            CrashHandler.InstallCrashLogger();
            NativeMethods.AttachConsole(NativeMethods.ATTACH_PARENT_PROCESS);
            Shell.Initialize();
            Paths.Initialize();

            ServicePointManager.SecurityProtocol = SecurityProtocolType.Tls12;

            ParseInternalArguments(args);
        }

        private static string GetBuildTag()
        {
#if WINDOWER_RELEASE_BUILD
            const string tag = string.Empty;
#else
            const string tag = "Development Build";
#endif

            var attributes = typeof(Program).Assembly.GetCustomAttributes<AssemblyMetadataAttribute>();
            return attributes.FirstOrDefault(a => a.Key == "BuildTag")?.Value ?? tag;
        }

        private static void ParseInternalArguments(string[] args)
        {
            using (var parser = new Parser(s => s.HelpWriter = null))
            {
                parser.ParseArguments<RemoteCallOptions, ReportCrashOptions, UpdateCleanupOptions>(args)
                    .WithParsed<RemoteCallOptions>(o => ExecuteAndExit(o.ProcessId))
                    .WithParsed<ReportCrashOptions>(ReportCrash)
                    .WithParsed<UpdateCleanupOptions>(UpdateCleanup)
                    .WithNotParsed(o => ParseArguments(args));
            }
        }

        private static void ParseArguments(string[] args)
        {
            CrashHandler.InstallCrashHandler();

            if (args.Length == 0 && !IsMono)
            {
                UserInterface.Run();
            }
            else
            {
                if (!IsMono)
                {
                    // Windows doesn't block the console for non-console
                    // executables, so clear the current line so things
                    // don't look too out of place.
                    try
                    {
                        Console.CursorLeft = 0;
                        Console.Write(new string(' ', Console.BufferWidth));
                        Console.CursorLeft = 0;
                    }
                    catch (IOException) { }
                }

                Parser.Default.ParseArguments<LaunchOptions, SaveOptions, DeleteOptions, GetArgsOptions>(args)
                    .WithParsed<LaunchOptions>(Launch)
                    .WithParsed<SaveOptions>(SaveProfile)
                    .WithParsed<DeleteOptions>(DeleteProfile)
                    .WithParsed<GetArgsOptions>(GetArgs);

                if (!IsMono)
                {
                    // Print out a dummy prompt.
                    Console.Write(Invariant($"{Directory.GetCurrentDirectory()}>"));
                }
            }
        }

        private static void Launch(LaunchOptions options)
        {
            try
            {
                if (options.NoGui || IsMono)
                {
                    Updater.Update().GetAwaiter().GetResult();
                    Launcher.Launch(options.GetProfile(), CancellationToken.None);
                }
                else
                {
                    UserInterface.Run(options.GetProfile());
                }
            }
            catch (KeyNotFoundException e)
            {
                Console.Error.WriteLine(e.Message);
            }
        }

        private static void SaveProfile(SaveOptions options)
        {
            try
            {
                var profile = options.GetProfile();
                if (Launcher.ProfileManager.TryGetValue(profile.Name, out var oldProfile))
                {
                    if (options.Overwrite)
                    {
                        Launcher.ProfileManager.Remove(oldProfile);
                    }
                    else
                    {
                        Console.Error.WriteLine("A profile with the name \"{0}\" already exists. (Use --overwrite to replace it.)",
                            profile.Name);
                        return;
                    }
                }
                Launcher.ProfileManager.Add(profile);
                Launcher.ProfileManager.Save();
            }
            catch (KeyNotFoundException e)
            {
                Console.Error.WriteLine(e.Message);
            }
        }

        private static void DeleteProfile(DeleteOptions options)
        {
            try
            {
                var profile = Launcher.ProfileManager[options.ProfileName];
                Launcher.ProfileManager.Remove(profile);
                Launcher.ProfileManager.Save();
            }
            catch (KeyNotFoundException e)
            {
                Console.Error.WriteLine(e.Message);
            }
        }

        private static void GetArgs(GetArgsOptions options)
        {
            try
            {
                Console.WriteLine(Launcher.ProfileManager[options.ProfileName].ArgString);
            }
            catch (KeyNotFoundException e)
            {
                Console.Error.WriteLine(e.Message);
            }
        }

        private static void ReportCrash(ReportCrashOptions options)
        {
            if (!IsMono)
            {
                string stackTrace = null;
                if (!string.IsNullOrWhiteSpace(options.StackTrace))
                {
                    var encoded = options.StackTrace.Trim();
                    stackTrace = Encoding.UTF8.GetString(Convert.FromBase64String(encoded));
                }

                UserInterface.RunCrashReporter(options.Signature, options.CrashDumpPath, stackTrace);
            }
            else
            {
                // TODO: Implement crash reporter for Linux and macOS.
            }
        }

        private static void UpdateCleanup(UpdateCleanupOptions options) => Updater.CleanUp();

        [SuppressMessage("Microsoft.Design", "CA1031")]
        private static void ExecuteAndExit(int processId)
        {
            var name = Invariant($"Windower.RPC[{processId}]");
            using (var pipe = new NamedPipeClientStream(".", name, PipeDirection.InOut, PipeOptions.Asynchronous))
            {
                pipe.Connect();
                var formatter = new BinaryFormatter();
                var result = default(ResultDescriptor);
                try
                {
                    var call = (CallDescriptor)formatter.Deserialize(pipe);
                    var method = call.Method;
                    var assembly = Assembly.GetExecutingAssembly();
                    if (method.ReflectedType.Assembly != assembly)
                    {
                        throw new InvalidOperationException(
                            Invariant($"Method \"{method.Name}\" is not in assembly \"{assembly.FullName}\"."));
                    }

                    if (method.GetCustomAttribute(typeof(RemoteCallableAttribute)) == null)
                    {
                        throw new InvalidOperationException(
                            Invariant($"Method \"{method.Name}\" does not have \"{nameof(RemoteCallableAttribute)}\"."));
                    }

                    var cancellationSignalName = Invariant($"Windower.RPC.CancellationSignal[{processId}]");
                    var cancellationSignal = new EventWaitHandle(false, EventResetMode.ManualReset, cancellationSignalName);
                    var source = new CancellationTokenSource();
                    new Thread(() =>
                    {
                        cancellationSignal.WaitOne();
                        source.Cancel();
                    }).Start();
                    result.Result = call.Method.Invoke(null, call.Arguments.Concat(new object[] { source.Token }).ToArray());
                    cancellationSignal.Set();
                }
                catch (TargetInvocationException e)
                {
                    result.ThrewException = true;
                    result.Result = e.InnerException;
                }
                catch (Exception e)
                {
                    result.ThrewException = true;
                    result.Result = e;
                }
                formatter.Serialize(pipe, result);
                Environment.Exit(0);
            }
        }

        private static async Task<object> RemoteCallAsync(bool elevate, Delegate method, CancellationToken token,
            params object[] args)
        {
            var processId = Process.GetCurrentProcess().Id;
            var name = Invariant($"Windower.RPC[{processId}]");
            using (var pipe = new NamedPipeServerStream(name, PipeDirection.InOut, 1, PipeTransmissionMode.Byte,
                PipeOptions.Asynchronous))
            {
                var info = new ProcessStartInfo()
                {
                    FileName = new Uri(Assembly.GetEntryAssembly().CodeBase).LocalPath,
                    Arguments = Parser.Default.FormatCommandLine(new RemoteCallOptions { ProcessId = processId }),
                    Verb = elevate ? "runas" : null
                };
                using (var process = Process.Start(info))
                {
                    pipe.WaitForConnection();
                    var formatter = new BinaryFormatter();
                    var call = default(CallDescriptor);
                    call.Method = method.Method;
                    call.Arguments = args;
                    await Task.Run(() => formatter.Serialize(pipe, call));
                    var result = default(ResultDescriptor);
                    if (token.CanBeCanceled)
                    {
                        var completed = new EventWaitHandle(false, EventResetMode.ManualReset);
                        var canceller = Task.Run(() =>
                        {
                            if (WaitHandle.WaitAny(new[] { token.WaitHandle, completed }) == 0)
                            {
                                var cancellationSignalName = Invariant($"Windower.RPC.CancellationSignal[{processId}]");
                                var cancellationSignal = new EventWaitHandle(false, EventResetMode.ManualReset,
                                    cancellationSignalName);
                                cancellationSignal.Set();
                            }
                        });
                        var runner = Task.Run(() =>
                        {
                            result = (ResultDescriptor)formatter.Deserialize(pipe);
                            completed.Set();
                        });
                        await Task.Run(() => Task.WaitAll(runner, canceller));
                    }
                    else
                    {
                        result = (ResultDescriptor)await Task.Run(() => formatter.Deserialize(pipe));
                    }

                    if (result.ThrewException)
                    {
                        ExceptionDispatchInfo.Capture((Exception)result.Result).Throw();
                    }
                    return result.Result;
                }
            }
        }

        [Serializable]
        private struct CallDescriptor
        {
            public MethodInfo Method;
            public object[] Arguments;
        }

        [Serializable]
        private struct ResultDescriptor
        {
            public bool ThrewException;
            public object Result;
        }

        [Verb("remote-call")]
        private class RemoteCallOptions
        {
            [Value(0, Required = true)]
            public int ProcessId { get; set; }
        }

        [Verb("report-crash")]
        [SuppressMessage("Microsoft.Performance", "CA1812")]
        private class ReportCrashOptions
        {
            [Value(0, Required = true)]
            public string CrashDumpPath { get; set; }

            [Option("signature", Required = true)]
            public string Signature { get; set; }

            [Option("stack-trace")]
            public string StackTrace { get; set; }
        }
    }
}
