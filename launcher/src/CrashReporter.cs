namespace Windower
{
    using Core;
    using Microsoft.Win32;
    using System;
    using System.Collections.Generic;
    using System.Collections.Immutable;
    using System.Diagnostics;
    using System.Diagnostics.CodeAnalysis;
    using System.IO;
    using System.IO.Compression;
    using System.Linq;
    using System.Reflection;
    using System.Runtime.CompilerServices;
    using System.Runtime.InteropServices;
    using System.Security;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Threading.Tasks;

    [SuppressMessage("Microsoft.Maintainability", "CA1506")]
    public static class CrashReporter
    {
        private static readonly Lazy<string> LazyCrashDumpPath =
            new Lazy<string>(() => Paths.ExpandPath(Path.Combine(Paths.GlobalUserPath, "crash")));
        private static readonly Lazy<IImmutableList<Tuple<string, string>>> LazyEnvironmentData =
            new Lazy<IImmutableList<Tuple<string, string>>>(GetEnvironmentData);

        public static string CrashDumpPath => LazyCrashDumpPath.Value;

        private static IImmutableList<Tuple<string, string>> EnvironmentData => LazyEnvironmentData.Value;

        public static bool EncryptionEnabled { get; } = false;

        public static string PrepareCrashReport(string exception) => PrepareCrashReport(exception, null);

        public static (FileStream stream, string file) PrepareCrashDump(string dump, string exception,
            IProgress<ProgressDetail> progress) => PrepareCrashDumpAsync(dump, exception, progress).GetAwaiter().GetResult();

        [SuppressMessage("Microsoft.Design", "CA1006")]
        public static async Task<(FileStream stream, string file)> PrepareCrashDumpAsync(string dump, string exception,
            IProgress<ProgressDetail> progress)
        {
            if (dump == null)
            {
                return (null, null);
            }

            var path = Path.Combine(Path.GetTempPath(), "Windower");
            Directory.CreateDirectory(path);

            FileStream stream = null;
            try
            {
                stream = new FileStream(Path.Combine(path, Guid.NewGuid().ToString()), FileMode.Create, FileAccess.ReadWrite,
                    FileShare.Delete | FileShare.Read);
                using (var zip = new ZipArchive(stream, ZipArchiveMode.Create, true))
                {
                    string crashDumpError = null;
                    try
                    {
                        using (var dumpStream = new FileStream(dump, FileMode.Open, FileAccess.Read,
                            FileShare.Delete | FileShare.Read))
                        {
                            File.Delete(dump);
                            var dumpEntry = zip.CreateEntry("crash.dmp", CompressionLevel.Optimal);
                            using (var zipStream = dumpEntry.Open())
                            {
                                await CopyStreamAsync(dumpStream, zipStream, progress);
                            }
                        }
                    }
                    catch (IOException e)
                    {
                        crashDumpError = CrashHandler.PrepareStackTrace(e);
                    }

                    progress.Report(0, 0);
                    var reportEntry = zip.CreateEntry("report.md", CompressionLevel.Optimal);
                    using (var zipStream = reportEntry.Open())
                    using (var writer = new StreamWriter(zipStream, new UTF8Encoding(), 1024, true))
                    {
                        await writer.WriteAsync(PrepareCrashReport(exception, null));
                        if (!string.IsNullOrWhiteSpace(crashDumpError))
                        {
                            var report = new StringBuilder();
                            AppendExceptionReport(report, crashDumpError, "Error preparing crash dump");
                            await writer.WriteAsync(report.ToString());
                        }
                    }
                }
                await stream.FlushAsync();

                stream.Position = 0;
                using (var streamCopy = await Utilities.CreateTimestampedFileStreamAsync(CrashDumpPath, ".zip"))
                {
                    await CopyStreamAsync(stream, streamCopy, null);
                    File.Delete(stream.Name);
                    stream.Position = 0;
                    return (stream, streamCopy.Name);
                }
            }
            catch
            {
                stream?.Dispose();
                throw;
            }
        }

        private static string PrepareCrashReport(string exception, string description)
        {
            var report = new StringBuilder();

            if (description != null)
            {
                // If the user opens a fenced code block but doesn't close
                // it, it breaks the rest of the report. We could be
                // smarter about this and only sanitize broken code blocks
                // but for now we'll just sanitize all of them.
                description = string.Join(Environment.NewLine,
                    description.Split(new[] { "\r\n", "\n" }, StringSplitOptions.None)
                    .Select(l => l.TrimStart().StartsWith("```", StringComparison.Ordinal) ? '\\' + l.TrimStart() : l));

                _ = report.AppendLine(description);
                _ = report.AppendLine();
                _ = report.AppendLine("* * *");
                _ = report.AppendLine();
            }

            _ = report.AppendLine("###### Environment");
            AppendTable(report, EnvironmentData);

            AppendExceptionReport(report, exception, "Exception");

            //if (crashDump != null)
            //{
            //    var gistUrl = crashDump.HtmlUrl;
            //    var dumpFile = crashDump.Files.FirstOrDefault().Value;
            //    report.AppendLine();
            //    if (dumpFile != null)
            //    {
            //        report.Append("[Crash Dump (");
            //        report.Append(GetFileSize(dumpFile.Size)).Append(")](");
            //        report.Append(dumpFile.RawUrl.OriginalString).Append(") | ");
            //    }
            //    report.Append("[Gist](").Append(gistUrl.OriginalString).Append(")");
            //}

            return report.ToString();
        }

        private static void AppendTable(StringBuilder builder, IEnumerable<Tuple<string, string>> values)
        {
            var marks = new Regex(@"[*_`[~\\]");

            var tuples = values.Select(p => Tuple.Create(
                "**" + marks.Replace(p.Item1, @"\$&") + "**",
                marks.Replace(p.Item2, @"\$&"))).ToArray();
            var keyLength = tuples.Max(p => p.Item1.Length) + 1;
            var valueLength = tuples.Max(p => p.Item2.Length) + 1;

            builder.Append('|').Append(' ', keyLength + 1);
            builder.Append('|').Append(' ', valueLength + 1);
            builder.Append('|').AppendLine();
            builder.Append('|').Append('-', keyLength).Append(':');
            builder.Append('|').Append(':').Append('-', valueLength);
            builder.Append('|').AppendLine();
            foreach (var pair in tuples)
            {
                builder.Append('|').Append(pair.Item1.PadLeft(keyLength)).Append(' ');
                builder.Append('|').Append(' ').Append(pair.Item2.PadRight(valueLength, ' '));
                builder.Append('|').AppendLine();
            }
        }

        private static void AppendExceptionReport(StringBuilder report, string exception, string header)
        {
            if (exception != null)
            {
                // unlikely to occur, but breaks the code block if it does.
                exception = string.Join(Environment.NewLine,
                    exception.Split(new[] { "\r\n", "\n" }, StringSplitOptions.RemoveEmptyEntries)
                    .Where(l => l.Trim() != "```"));
                report.AppendLine();
                report.Append("###### ").AppendLine(header);
                report.AppendLine("```");
                report.AppendLine(exception);
                report.AppendLine("```");
            }
        }

        private static IImmutableList<Tuple<string, string>> GetEnvironmentData()
        {
            var environment = ImmutableArray<Tuple<string, string>>.Empty;

            environment = environment.Add(Tuple.Create("Operating System", GetOperatingSystem()));
            var wineVersion = GetWineVersion();
            if (wineVersion != null)
            {
                environment = environment.Add(Tuple.Create("Wine Version", wineVersion));
                environment = environment.Add(Tuple.Create("Host Operating System", GetHost()));
            }
            environment = environment.Add(Tuple.Create(".NET Framework", GetFramework()));
            environment = environment.Add(Tuple.Create("Launcher Version", GetLauncherVersion()));
            environment = environment.Add(Tuple.Create("Core Version", GetCoreVersion()));

            return environment;
        }

        private static string GetOperatingSystem()
        {
            if (Environment.OSVersion.Platform == PlatformID.Win32NT)
            {
                using (var key = Registry.LocalMachine.OpenSubKey(@"SOFTWARE\Microsoft\Windows NT\CurrentVersion"))
                {
                    var name = key.GetValue("ProductName").ToString().Trim();
                    var servicePack = key.GetValue("CSDVersion")?.ToString().Trim();

                    if (!string.IsNullOrEmpty(name))
                    {
                        var version = Environment.OSVersion.Version.ToString();
                        try
                        {
                            var temp = default(NativeMethods.RTL_OSVERSIONINFOW);
                            temp.dwOSVersionInfoSize = (uint)Marshal.SizeOf(typeof(NativeMethods.RTL_OSVERSIONINFOW));
                            if (NativeMethods.RtlGetVersion(ref temp) == NativeMethods.STATUS_SUCCESS)
                            {
                                version = temp.dwMajorVersion + "." + temp.dwMinorVersion + "." +
                                    temp.dwBuildNumber;
                                if (!string.IsNullOrWhiteSpace(temp.szCSDVersion))
                                {
                                    version += " " + temp.szCSDVersion;
                                }
                            }
                        }
                        catch (EntryPointNotFoundException) { }

                        return (name.StartsWith("Microsoft", StringComparison.Ordinal) ? string.Empty : "Microsoft ") + name +
                            (Environment.Is64BitOperatingSystem ? " 64-bit" : " 32-bit") +
                            (string.IsNullOrEmpty(servicePack) ? string.Empty : " " + servicePack) +
                            " (" + version + ")";
                    }
                }
            }

            return Environment.OSVersion.VersionString;
        }

        private static string GetWineVersion()
        {
            try
            {
                return NativeMethods.wine_get_version();
            }
            catch (EntryPointNotFoundException) { }
            return null;
        }

        private static string GetHost()
        {
            try
            {
                NativeMethods.wine_get_host_version(out var sysname, out var release);
                return sysname + " " + release;
            }
            catch (EntryPointNotFoundException) { }
            return "Unknown";
        }

        private static string GetFramework()
        {
            var type = Type.GetType("Mono.Runtime");
            if (type != null)
            {
                var getDisplayName = type.GetMethod("GetDisplayName", BindingFlags.NonPublic | BindingFlags.Static);
                if (getDisplayName != null)
                {
                    return "Mono " + getDisplayName.Invoke(null, null);
                }
                return "Mono (Unknown Version)";
            }

            using (var key = Registry.LocalMachine.OpenSubKey(@"SOFTWARE\Microsoft\NET Framework Setup\NDP\v4\Full"))
            {
                if (key != null)
                {
                    var release = key.GetValue("Release") as int? ?? 0;
                    if (release > 394748)
                    {
                        return "Microsoft .NET Framework 4.6.2 or later";
                    }
                    if (release >= 394747)
                    {
                        return "Microsoft .NET Framework 4.6.2";
                    }
                    else if (release >= 394254)
                    {
                        return "Microsoft .NET Framework 4.6.1";
                    }
                    else if (release >= 393295)
                    {
                        return "Microsoft .NET Framework 4.6";
                    }
                    else if (release >= 379893)
                    {
                        return "Microsoft .NET Framework 4.5.2";
                    }
                    else if (release >= 378675)
                    {
                        return "Microsoft .NET Framework 4.5.1";
                    }
                    else if (release >= 378389)
                    {
                        return "Microsoft .NET Framework 4.5";
                    }
                }
            }

            return "Microsoft .NET Framework " + Environment.Version;
        }

        private static string GetLauncherVersion()
        {
            var version = typeof(CrashReporter).Assembly.GetName().Version.ToString();
            var tag = Program.BuildTag;
            return string.IsNullOrWhiteSpace(tag) ? version : version + " (" + tag + ")";
        }

        private static string GetCoreVersion()
        {
            FileVersionInfo info;
            try
            {
                info = FileVersionInfo.GetVersionInfo(Launcher.CorePath);
            }
            catch (FileNotFoundException)
            {
                return "Unknown";
            }
            var version = info.ProductMajorPart + "." + info.ProductMinorPart + "." +
                info.ProductBuildPart + "." + info.ProductPrivatePart;
            var tag = info.SpecialBuild;
            return string.IsNullOrWhiteSpace(tag) ? version : version + " (" + tag + ")";
        }

        private static string GetAutomaticReportTag(string signature) =>
            "<!-- Automated Crash Report: " + signature.Replace("--", "") + " -->";

        private static string GetFileSize(long size)
        {
            if (size >= 1000000)
            {
                return (size / 1000000) + "." + (size / 100000 % 10) + " MB";
            }
            if (size >= 1000)
            {
                return (size / 1000) + "." + (size / 100 % 10) + " KB";
            }
            return size + " B";
        }

        private static async Task CopyStreamAsync(this Stream source, Stream destination, IProgress<ProgressDetail> progress)
        {
            if (source == null)
            {
                throw new ArgumentNullException(nameof(source));
            }

            if (!source.CanRead)
            {
                throw new ArgumentException("Source stream must support reading", nameof(source));
            }

            if (destination == null)
            {
                throw new ArgumentNullException(nameof(destination));
            }

            if (!destination.CanWrite)
            {
                throw new ArgumentException("Destination stream must support reading", nameof(destination));
            }

            progress?.Report(ProgressDetail.Create(0, 0));

            var readBuffer = new byte[65536];
            var writeBuffer = new byte[65536];
            Task write = null;
            int bytesRead;

            var total = source.CanSeek ? source.Length : 0;
            long current = 0;
            do
            {
                bytesRead = await source.ReadAsync(readBuffer, 0, readBuffer.Length);
                if (write != null)
                {
                    await write;
                    progress?.Report(ProgressDetail.Create(current, total));
                }

                if (bytesRead > 0)
                {
                    var temp = writeBuffer;
                    writeBuffer = readBuffer;
                    readBuffer = temp;

                    write = destination.WriteAsync(writeBuffer, 0, bytesRead);
                    current += bytesRead;
                }
            }
            while (bytesRead > 0);
        }

        private static bool VerifyAssemblyStrongName(Assembly assembly)
        {
            if (assembly.Location != null)
            {
                try
                {
                    var clrStrongName = (IClrStrongName)RuntimeEnvironment.GetRuntimeInterfaceAsObject(
                        new Guid("B79B0ACD-F5CD-409b-B5A5-A16244610B92"), new Guid("9FD93CCF-3280-4391-B3A9-96E1CDE77C8D"));
                    clrStrongName.StrongNameSignatureVerificationEx(assembly.Location, true, out bool verified);
                    return verified;
                }
                catch (COMException) { }
            }

            return false;
        }

        [ComImport]
        [ComConversionLoss, Guid("9FD93CCF-3280-4391-B3A9-96E1CDE77C8D")]
        [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
        [SecurityCritical]
        private interface IClrStrongName
        {
            [MethodImpl(MethodImplOptions.PreserveSig | MethodImplOptions.InternalCall)]
            int GetHashFromAssemblyFile([MarshalAs(UnmanagedType.LPStr)] [In] string pszFilePath,
                [MarshalAs(UnmanagedType.U4)] [In] [Out] ref int piHashAlg,
                [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 3)] [Out] byte[] pbHash,
                [MarshalAs(UnmanagedType.U4)] [In] int cchHash, [MarshalAs(UnmanagedType.U4)] out int pchHash);

            [MethodImpl(MethodImplOptions.PreserveSig | MethodImplOptions.InternalCall)]
            int GetHashFromAssemblyFileW([MarshalAs(UnmanagedType.LPWStr)] [In] string pwzFilePath,
                [MarshalAs(UnmanagedType.U4)] [In] [Out] ref int piHashAlg,
                [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 3)] [Out] byte[] pbHash,
                [MarshalAs(UnmanagedType.U4)] [In] int cchHash, [MarshalAs(UnmanagedType.U4)] out int pchHash);

            [MethodImpl(MethodImplOptions.PreserveSig | MethodImplOptions.InternalCall)]
            int GetHashFromBlob([In] IntPtr pbBlob, [MarshalAs(UnmanagedType.U4)] [In] int cchBlob,
                [MarshalAs(UnmanagedType.U4)] [In] [Out] ref int piHashAlg,
                [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 4)] [Out] byte[] pbHash,
                [MarshalAs(UnmanagedType.U4)] [In] int cchHash, [MarshalAs(UnmanagedType.U4)] out int pchHash);

            [MethodImpl(MethodImplOptions.PreserveSig | MethodImplOptions.InternalCall)]
            int GetHashFromFile([MarshalAs(UnmanagedType.LPStr)] [In] string pszFilePath,
                [MarshalAs(UnmanagedType.U4)] [In] [Out] ref int piHashAlg,
                [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 3)] [Out] byte[] pbHash,
                [MarshalAs(UnmanagedType.U4)] [In] int cchHash, [MarshalAs(UnmanagedType.U4)] out int pchHash);

            [MethodImpl(MethodImplOptions.PreserveSig | MethodImplOptions.InternalCall)]
            int GetHashFromFileW([MarshalAs(UnmanagedType.LPWStr)] [In] string pwzFilePath,
                [MarshalAs(UnmanagedType.U4)] [In] [Out] ref int piHashAlg,
                [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 3)] [Out] byte[] pbHash,
                [MarshalAs(UnmanagedType.U4)] [In] int cchHash, [MarshalAs(UnmanagedType.U4)] out int pchHash);

            [MethodImpl(MethodImplOptions.PreserveSig | MethodImplOptions.InternalCall)]
            int GetHashFromHandle([In] IntPtr hFile, [MarshalAs(UnmanagedType.U4)] [In] [Out] ref int piHashAlg,
                [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 3)] [Out] byte[] pbHash,
                [MarshalAs(UnmanagedType.U4)] [In] int cchHash, [MarshalAs(UnmanagedType.U4)] out int pchHash);

            [MethodImpl(MethodImplOptions.PreserveSig | MethodImplOptions.InternalCall)]
            [return: MarshalAs(UnmanagedType.U4)]
            int StrongNameCompareAssemblies([MarshalAs(UnmanagedType.LPWStr)] [In] string pwzAssembly1,
                [MarshalAs(UnmanagedType.LPWStr)] [In] string pwzAssembly2, [MarshalAs(UnmanagedType.U4)] out int dwResult);

            [MethodImpl(MethodImplOptions.PreserveSig | MethodImplOptions.InternalCall)]
            int StrongNameFreeBuffer([In] IntPtr pbMemory);

            [MethodImpl(MethodImplOptions.PreserveSig | MethodImplOptions.InternalCall)]
            int StrongNameGetBlob([MarshalAs(UnmanagedType.LPWStr)] [In] string pwzFilePath,
                [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 2)] [Out] byte[] pbBlob,
                [MarshalAs(UnmanagedType.U4)] [In] [Out] ref int pcbBlob);

            [MethodImpl(MethodImplOptions.PreserveSig | MethodImplOptions.InternalCall)]
            int StrongNameGetBlobFromImage([In] IntPtr pbBase, [MarshalAs(UnmanagedType.U4)] [In] int dwLength,
                [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 3)] [Out] byte[] pbBlob,
                [MarshalAs(UnmanagedType.U4)] [In] [Out] ref int pcbBlob);

            [MethodImpl(MethodImplOptions.PreserveSig | MethodImplOptions.InternalCall)]
            int StrongNameGetPublicKey([MarshalAs(UnmanagedType.LPWStr)] [In] string pwzKeyContainer,
                [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 2)] [In] byte[] pbKeyBlob,
                [MarshalAs(UnmanagedType.U4)] [In] int cbKeyBlob, out IntPtr ppbPublicKeyBlob,
                [MarshalAs(UnmanagedType.U4)] out int pcbPublicKeyBlob);

            [MethodImpl(MethodImplOptions.PreserveSig | MethodImplOptions.InternalCall)]
            [return: MarshalAs(UnmanagedType.U4)]
            int StrongNameHashSize([MarshalAs(UnmanagedType.U4)] [In] int ulHashAlg,
                [MarshalAs(UnmanagedType.U4)] out int cbSize);

            [MethodImpl(MethodImplOptions.PreserveSig | MethodImplOptions.InternalCall)]
            int StrongNameKeyDelete([MarshalAs(UnmanagedType.LPWStr)] [In] string pwzKeyContainer);

            [MethodImpl(MethodImplOptions.PreserveSig | MethodImplOptions.InternalCall)]
            int StrongNameKeyGen([MarshalAs(UnmanagedType.LPWStr)] [In] string pwzKeyContainer,
                [MarshalAs(UnmanagedType.U4)] [In] int dwFlags, out IntPtr ppbKeyBlob,
                [MarshalAs(UnmanagedType.U4)] out int pcbKeyBlob);

            [MethodImpl(MethodImplOptions.PreserveSig | MethodImplOptions.InternalCall)]
            int StrongNameKeyGenEx([MarshalAs(UnmanagedType.LPWStr)] [In] string pwzKeyContainer,
                [MarshalAs(UnmanagedType.U4)] [In] int dwFlags, [MarshalAs(UnmanagedType.U4)] [In] int dwKeySize,
                out IntPtr ppbKeyBlob, [MarshalAs(UnmanagedType.U4)] out int pcbKeyBlob);

            [MethodImpl(MethodImplOptions.PreserveSig | MethodImplOptions.InternalCall)]
            int StrongNameKeyInstall([MarshalAs(UnmanagedType.LPWStr)] [In] string pwzKeyContainer,
                [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 2)] [In] byte[] pbKeyBlob,
                [MarshalAs(UnmanagedType.U4)] [In] int cbKeyBlob);

            [MethodImpl(MethodImplOptions.PreserveSig | MethodImplOptions.InternalCall)]
            int StrongNameSignatureGeneration([MarshalAs(UnmanagedType.LPWStr)] [In] string pwzFilePath,
                [MarshalAs(UnmanagedType.LPWStr)] [In] string pwzKeyContainer,
                [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 3)] [In] byte[] pbKeyBlob,
                [MarshalAs(UnmanagedType.U4)] [In] int cbKeyBlob, [In] [Out] IntPtr ppbSignatureBlob,
                [MarshalAs(UnmanagedType.U4)] out int pcbSignatureBlob);

            [MethodImpl(MethodImplOptions.PreserveSig | MethodImplOptions.InternalCall)]
            int StrongNameSignatureGenerationEx([MarshalAs(UnmanagedType.LPWStr)] [In] string wszFilePath,
                [MarshalAs(UnmanagedType.LPWStr)] [In] string wszKeyContainer,
                [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 3)] [In] byte[] pbKeyBlob,
                [MarshalAs(UnmanagedType.U4)] [In] int cbKeyBlob, [In] [Out] IntPtr ppbSignatureBlob,
                [MarshalAs(UnmanagedType.U4)] out int pcbSignatureBlob, [MarshalAs(UnmanagedType.U4)] [In] int dwFlags);

            [MethodImpl(MethodImplOptions.PreserveSig | MethodImplOptions.InternalCall)]
            int StrongNameSignatureSize([MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)] [In] byte[] pbPublicKeyBlob,
                [MarshalAs(UnmanagedType.U4)] [In] int cbPublicKeyBlob, [MarshalAs(UnmanagedType.U4)] out int pcbSize);

            [MethodImpl(MethodImplOptions.PreserveSig | MethodImplOptions.InternalCall)]
            [return: MarshalAs(UnmanagedType.U4)]
            int StrongNameSignatureVerification([MarshalAs(UnmanagedType.LPWStr)] [In] string pwzFilePath,
                [MarshalAs(UnmanagedType.U4)] [In] int dwInFlags, [MarshalAs(UnmanagedType.U4)] out int dwOutFlags);

            [MethodImpl(MethodImplOptions.PreserveSig | MethodImplOptions.InternalCall)]
            [return: MarshalAs(UnmanagedType.U1)]
            bool StrongNameSignatureVerificationEx([MarshalAs(UnmanagedType.LPWStr)] [In] string pwzFilePath,
                [MarshalAs(UnmanagedType.U1)] [In] bool fForceVerification, [MarshalAs(UnmanagedType.U1)] out bool fWasVerified);

            [MethodImpl(MethodImplOptions.PreserveSig | MethodImplOptions.InternalCall)]
            [return: MarshalAs(UnmanagedType.U4)]
            int StrongNameSignatureVerificationFromImage([In] IntPtr pbBase, [MarshalAs(UnmanagedType.U4)] [In] int dwLength,
                [MarshalAs(UnmanagedType.U4)] [In] int dwInFlags, [MarshalAs(UnmanagedType.U4)] out int dwOutFlags);

            [MethodImpl(MethodImplOptions.PreserveSig | MethodImplOptions.InternalCall)]
            int StrongNameTokenFromAssembly([MarshalAs(UnmanagedType.LPWStr)] [In] string pwzFilePath,
                out IntPtr ppbStrongNameToken, [MarshalAs(UnmanagedType.U4)] out int pcbStrongNameToken);

            [MethodImpl(MethodImplOptions.PreserveSig | MethodImplOptions.InternalCall)]
            int StrongNameTokenFromAssemblyEx([MarshalAs(UnmanagedType.LPWStr)] [In] string pwzFilePath,
                out IntPtr ppbStrongNameToken, [MarshalAs(UnmanagedType.U4)] out int pcbStrongNameToken,
                out IntPtr ppbPublicKeyBlob, [MarshalAs(UnmanagedType.U4)] out int pcbPublicKeyBlob);

            [MethodImpl(MethodImplOptions.PreserveSig | MethodImplOptions.InternalCall)]
            int StrongNameTokenFromPublicKey([MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)] [In] byte[] pbPublicKeyBlob,
                [MarshalAs(UnmanagedType.U4)] [In] int cbPublicKeyBlob, out IntPtr ppbStrongNameToken,
                [MarshalAs(UnmanagedType.U4)] out int pcbStrongNameToken);
        }
    }
}
