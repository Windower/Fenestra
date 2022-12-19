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
    using System;
    using System.Collections;
    using System.Diagnostics;
    using System.Diagnostics.CodeAnalysis;
    using System.IO;
    using System.Linq;
    using System.Reflection;
    using System.Runtime.ExceptionServices;
    using System.Runtime.InteropServices;
    using System.Security;
    using System.Text;

    public static class CrashHandler
    {
        private const int BasicMiniDump = 0x00081965;
        private const int FullMiniDump = 0x00021926;

        public static void InstallCrashLogger()
        {
            AppDomain.CurrentDomain.UnhandledException += LogCrash;
            AppDomain.CurrentDomain.UnhandledException -= HandleCrash;
        }

        public static void InstallCrashHandler()
        {
            AppDomain.CurrentDomain.UnhandledException += HandleCrash;
            AppDomain.CurrentDomain.UnhandledException -= LogCrash;
        }

        public static string PrepareStackTrace(object data)
        {
            var stackTrace = new StringBuilder();

            if (data is Exception exception)
            {
                _ = stackTrace.Append(exception.GetType().FullName).Append(": ").AppendLine(exception.Message);
                if (exception.Data.Count > 0)
                {
                    _ = stackTrace.AppendLine("Data:");
                    foreach (var entry in exception.Data.Cast<DictionaryEntry>())
                    {
                        _ = stackTrace.Append("   ").Append(entry.Key ?? "<null>").Append(": ")
                            .Append(entry.Value ?? "<null>").AppendLine();
                    }
                }
                _ = stackTrace.AppendLine("Stack Trace:");
                _ = stackTrace.Append(exception.StackTrace);
            }
            else
            {
                _ = stackTrace.Append(data);
            }

            return stackTrace.ToString();
        }

        private static string GetSignature(object data)
        {
            var signature = new StringBuilder();

            if (data is Exception exception)
            {
                _ = signature.Append(exception.GetType().Name);
                _ = signature.Append('@');
                var target = exception.TargetSite;
                if (target == null)
                {
                    _ = signature.Append("<Unknown>");
                }
                else
                {
                    _ = signature.Append(target.Module.Name.EndsWith(".dll", StringComparison.OrdinalIgnoreCase) ?
                        target.Module.Name : target.Module.ScopeName);
                    _ = signature.Append('!');
                    _ = signature.Append(target.ReflectedType?.FullName ?? "<Unknown>");
                    _ = signature.Append('.').Append(target.Name);
                    var parameters = target.GetParameters();
                    _ = signature.Append('(').Append(string.Join(", ", parameters.Select(p => p.ParameterType.Name))).Append(')');
                }
            }
            else
            {
                _ = signature.Append(data.GetType().FullName);
            }

            return signature.ToString();
        }

        [SuppressMessage("Microsoft.Usage", "CA2202")]
        [HandleProcessCorruptedStateExceptions]
        [SecurityCritical]
        private static void LogCrash(object sender, UnhandledExceptionEventArgs e)
        {
            if (!Debugger.IsAttached)
            {
                var path = CrashReporter.CrashDumpPath;
                _ = Directory.CreateDirectory(path);
                using (var stream = Utilities.CreateTimestampedFileStream(path, ".md"))
                using (var writer = new StreamWriter(stream, new UTF8Encoding(), 1024, true))
                {
                    writer.Write(CrashReporter.PrepareCrashReport(PrepareStackTrace(e.ExceptionObject)));
                }

                // We're intentionally letting the crash happen so that
                // the Windows Error Reporting dialog appears. We could
                // stop it by calling Environment.Exit(-1), but then the
                // user is left with no indication of what happened.
            }
        }

        [HandleProcessCorruptedStateExceptions]
        [SecurityCritical]
        private static void HandleCrash(object sender, UnhandledExceptionEventArgs e)
        {
            if (!Debugger.IsAttached)
            {
                try
                {
                    var signature = GetSignature(e.ExceptionObject);
                    var stackTrace = PrepareStackTrace(e.ExceptionObject);
                    var dumpFile = Path.Combine(Path.GetTempPath(), "Windower", Guid.NewGuid() + ".dmp");

                    _ = Directory.CreateDirectory(Path.GetDirectoryName(dumpFile));
                    using (var stream = new FileStream(dumpFile, FileMode.Create))
                    using (var process = Process.GetCurrentProcess())
                    {
                        var exceptionInfo = default(NativeMethods.MINIDUMP_EXCEPTION_INFORMATION);
                        exceptionInfo.ThreadId = NativeMethods.GetCurrentThreadId();
                        exceptionInfo.ExceptionPointers = Marshal.GetExceptionPointers();
                        _ = NativeMethods.MiniDumpWriteDump(process.Handle, process.Id, stream.SafeFileHandle, BasicMiniDump,
                            ref exceptionInfo, IntPtr.Zero, IntPtr.Zero);
                    }

                    stackTrace = Convert.ToBase64String(Encoding.UTF8.GetBytes(stackTrace));

                    var path = new Uri(Assembly.GetEntryAssembly().EscapedCodeBase).LocalPath;
                    _ = Process.Start(path, "report-crash --signature \"" + signature +
                        "\" --stack-trace \"" + stackTrace + "\" \"" + dumpFile.Replace("\"", "\\\"") + "\"");

                    Environment.Exit(-1);
                }
                catch (Exception ex)
                {
                    LogCrash(null, new UnhandledExceptionEventArgs(ex, true));
                    throw;
                }
            }
        }
    }
}
