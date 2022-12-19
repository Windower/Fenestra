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

namespace Windower.Core
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Net;
    using System.Reflection;
    using System.Text.RegularExpressions;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Xml.Linq;

    internal static partial class Updater
    {
        private static readonly Lazy<string> LazyUserAgent = new Lazy<string>(GetUserAgent);
        private static readonly Lazy<string> LazyUserAgentName = new Lazy<string>(GetUserAgentName);
        private static readonly Lazy<string> LazyUserAgentVersion = new Lazy<string>(GetUserAgentVersion);

#if WINDOWER_RELEASE_BUILD
        private const string updateUrl = "https://files.windower.net/5/core/release/";
#else
        private const string updateUrl = "https://files.windower.net/5/core/test/";
#endif

        public static string UserAgent => LazyUserAgent.Value;

        public static string UserAgentName => LazyUserAgentName.Value;

        public static string UserAgentVersion => LazyUserAgentVersion.Value;

        public static Task Update()
        {
            return Update(null);
        }

        public static async Task Update(IProgress<ProgressDetail<UpdateStatus>> progress)
        {
#if DEBUG
            await Task.FromResult(false);
#else
            progress?.Report(ProgressDetail.Create(0, 0, UpdateStatus.Checking));
            var info = await GetUpdateInfo();

            var launcherVersion = new Version((string)info?.Element("launcher")?.Attribute("version") ?? "0.0.0.0");
            var coreVersion = new Version((string)info?.Element("core")?.Attribute("version") ?? "0.0.0.0");

            var updateLauncher = GetLauncherVersion() < launcherVersion;
            var updateCore = GetCoreVersion() < coreVersion;

            var launcherSize = (long?)info?.Element("launcher")?.Attribute("size") ?? 0;
            var coreSize = (long?)info?.Element("core")?.Attribute("size") ?? 0;
            var totalSize = (updateLauncher ? launcherSize : 0) + (updateCore ? coreSize : 0);
            var downloaded = 0L;

            string launcherTempPath = null;
            string coreTempPath = null;

            if (updateLauncher)
            {
                progress?.Report(ProgressDetail.Create(downloaded, totalSize, UpdateStatus.DownloadingLauncher));
                launcherTempPath = await DownloadToTemporaryFile(new Uri(updateUrl + "windower.exe"),
                    progress == null ? null : new Progress<long>(p =>
                    {
                        progress?.Report(ProgressDetail.Create(downloaded + p, totalSize, UpdateStatus.DownloadingLauncher));
                    }));
                downloaded += launcherSize;
            }

            if (updateCore)
            {
                progress?.Report(ProgressDetail.Create(downloaded, totalSize, UpdateStatus.DownloadingCore));
                coreTempPath = await DownloadToTemporaryFile(new Uri(updateUrl + "core.dll"),
                    progress == null ? null : new Progress<long>(p =>
                    {
                        progress?.Report(ProgressDetail.Create(downloaded + p, totalSize, UpdateStatus.DownloadingCore));
                    }));
                downloaded += coreSize;
            }

            progress?.Report(ProgressDetail.Create(0, 0, UpdateStatus.Installing));
            if (await InstallUpdatesAsync(launcherTempPath, coreTempPath))
            {
                progress?.Report(ProgressDetail.Create(0, 0, UpdateStatus.Restarting));
                Process.Start(new Uri(typeof(Launcher).Assembly.EscapedCodeBase).LocalPath,
                    EscapeArguments(Environment.GetCommandLineArgs().Skip(1)));
                Environment.Exit(0);
            }
#endif
        }

        public static void CleanUp()
        {
            try
            {
                var tempPath = Paths.ExpandPath(Paths.GlobalTempPath);
                Directory.Delete(tempPath, true);
                Directory.CreateDirectory(tempPath);
            }
            catch (DirectoryNotFoundException) { }
            catch (UnauthorizedAccessException) { }

            try
            {
                var launcherPath = new Uri(typeof(Launcher).Assembly.EscapedCodeBase).LocalPath;
                var deletePendingPath = Path.Combine(Path.GetDirectoryName(launcherPath), "delete_pending");
                var start = DateTime.Now;
                while ((DateTime.Now - start) < TimeSpan.FromMinutes(1) && Directory.Exists(deletePendingPath))
                {
                    try
                    {
                        Directory.Delete(deletePendingPath, true);
                    }
                    catch (UnauthorizedAccessException) { }
                    Thread.Sleep(1000);
                }
            }
            catch (DirectoryNotFoundException) { }
        }

        private static Version GetLauncherVersion() => Assembly.GetEntryAssembly().GetName().Version;

        private static Version GetCoreVersion()
        {
            FileVersionInfo info;
            try
            {
                info = FileVersionInfo.GetVersionInfo(Launcher.CorePath);
            }
            catch (FileNotFoundException)
            {
                return new Version();
            }
            return new Version(info.ProductMajorPart, info.ProductMinorPart, info.ProductBuildPart, info.ProductPrivatePart);
        }

        private static async Task<XElement> GetUpdateInfo()
        {
            var versionPath = Paths.ExpandPath(Path.Combine(Paths.GlobalSettingsPath, "version.xml"));

            XDocument document = null;
            try
            {
                document = XDocument.Load(versionPath);
            }
            catch (FileNotFoundException) { }
            catch (DirectoryNotFoundException) { }

            DateTime lastUpdated = DateTime.MinValue;
            var timestamp = (string)document?.Root?.Attribute("timestamp");
            if (timestamp == null)
            {
                lastUpdated = DateTime.TryParseExact(timestamp, "O", CultureInfo.InvariantCulture,
                    DateTimeStyles.AdjustToUniversal, out var result) ? result : DateTime.MinValue;
            }

            var request = WebRequest.CreateHttp(new Uri(updateUrl + "version.xml"));
            request.UserAgent = UserAgent;
            request.IfModifiedSince = lastUpdated;
            request.AutomaticDecompression = DecompressionMethods.Deflate | DecompressionMethods.GZip;
            try
            {
                using (var response = (HttpWebResponse)await request.GetResponseAsync())
                {
                    using (var stream = response.GetResponseStream())
                    {
                        var temp = XDocument.Load(stream);
                        temp.Root.SetAttributeValue("timestamp", response.LastModified.ToString("O"));
                        Directory.CreateDirectory(Path.GetDirectoryName(versionPath));
                        temp.Save(versionPath);
                        document = temp;
                    }
                }
            }
            catch (WebException) { }

            return document?.Root;
        }

        private static async Task<string> DownloadToTemporaryFile(Uri url, IProgress<long> progress)
        {
            var request = WebRequest.CreateHttp(url);
            request.UserAgent = UserAgent;
            request.AutomaticDecompression = DecompressionMethods.Deflate | DecompressionMethods.GZip;
            try
            {
                using (var response = (HttpWebResponse)await request.GetResponseAsync())
                using (var inputStream = response.GetResponseStream())
                {
                    var path = Paths.ExpandPath(Paths.GlobalTempPath);
                    Directory.CreateDirectory(path);
                    path = Path.Combine(path, Path.GetRandomFileName());
                    using (var outputStream = new FileStream(path, FileMode.CreateNew))
                    {
                        await inputStream.CopyToAsync(outputStream, progress);
                        return path;
                    }
                }
            }
            catch (WebException) { }

            return null;
        }

        private static async Task<bool> InstallUpdatesAsync(string launcherTempPath, string coreTempPath)
        {
            if (launcherTempPath != null || coreTempPath != null)
            {
                try
                {
                    return InstallUpdates(launcherTempPath, coreTempPath, false, CancellationToken.None);
                }
                catch (UnauthorizedAccessException)
                {
                    if (Launcher.IsAdministrator())
                    {
                        throw;
                    }

                    return await Program.ElevateAsync(InstallUpdates, launcherTempPath, coreTempPath, true, CancellationToken.None);
                }
            }
            return false;
        }

        [RemoteCallable]
        private static bool InstallUpdates(string launcherTempPath, string coreTempPath, bool elevated, CancellationToken token)
        {
            var launcherPath = new Uri(typeof(Launcher).Assembly.EscapedCodeBase).LocalPath;
            var installRoot = Path.GetDirectoryName(launcherPath);

            var restartRequired = false;

            try
            {
                var filesToDeletePath = Path.Combine(installRoot, "delete_pending");
                var info = Directory.CreateDirectory(filesToDeletePath);
                info.Attributes |= FileAttributes.Hidden;

                if (launcherTempPath != null)
                {
                    File.Move(launcherPath, Path.Combine(filesToDeletePath, Path.GetRandomFileName()));
                    File.Move(launcherTempPath, launcherPath);
                    restartRequired = true;
                }

                if (coreTempPath != null)
                {
                    var corePath = Path.Combine(Path.GetDirectoryName(launcherPath), "core.dll");
                    try
                    {
                        File.Move(corePath, Path.Combine(filesToDeletePath, Path.GetRandomFileName()));
                    }
                    catch (FileNotFoundException) { }
                    File.Move(coreTempPath, corePath);
                }
            }
            catch (IOException) { }

            Process.Start(new ProcessStartInfo() { FileName = launcherPath, Arguments = "update-cleanup" });

            return restartRequired;
        }

        private static string GetUserAgent() => UserAgentName + '/' + UserAgentVersion;

        private static string GetUserAgentName() =>
            Assembly.GetEntryAssembly().GetCustomAttributes<AssemblyProductAttribute>().FirstOrDefault()?.Product ??
            Assembly.GetEntryAssembly().GetName().Name;

        private static string GetUserAgentVersion() => GetLauncherVersion().ToString(2);

        private static string EscapeArguments(IEnumerable<string> args)
        {
            var quotedArgs =
                from a in args ?? new string[0]
                where !string.IsNullOrEmpty(a)
                select EscapeArgument(a);
            return string.Join(" ", quotedArgs);
        }

        private static string EscapeArgument(string argument)
        {
            if (argument.Any(c => "\"\t ".Contains(c)))
            {
                return FormattableString.Invariant($"\"{Regex.Replace(argument, @"(\\*)(\\$|\"")", @"$1$1\$2")}\"");
            }

            return argument;
        }
    }
}
