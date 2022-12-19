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
    using Boiler;
    using Microsoft.Win32;
    using PlayOnline;
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Diagnostics.CodeAnalysis;
    using System.IO;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Security.AccessControl;
    using System.Security.Principal;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Runtime.ExceptionServices;

    using static System.FormattableString;

    public static class Launcher
    {
        public static string CorePath { get; } = GetCorePath();

        public static ProfileManager ProfileManager { get; } = new ProfileManager();

        [RemoteCallable]
        public static bool CheckDirectPlay(CancellationToken token)
        {
            try
            {
                _ = Marshal.ReleaseComObject(new DirectPlay8ClientStub());
                return true;
            }
            catch (UnauthorizedAccessException) { }
            return false;
        }

        public static Task<bool> CheckDirectPlayAsync() =>
            Program.RemoteCallAsync(CheckDirectPlay, CancellationToken.None);

        public static bool InstallDirectPlay()
        {
            var info = new ProcessStartInfo();
            var systemPath = Environment.Is64BitOperatingSystem && !Environment.Is64BitProcess ?
                Path.Combine(Environment.GetEnvironmentVariable("SystemRoot"), "SysNative") :
                Environment.GetFolderPath(Environment.SpecialFolder.System);
            var path = Path.Combine(systemPath, "Dism.exe");
            info.FileName = "cmd";
            info.Arguments = Invariant($"/C {path} /Online /Enable-Feature /FeatureName:DirectPlay /All");
            info.Verb = "runas";
            info.WindowStyle = ProcessWindowStyle.Hidden;
            try
            {
                Process.Start(info).WaitForExit();
            }
            catch (Win32Exception)
            {
                return false;
            }
            return CheckDirectPlay(CancellationToken.None);
        }

        public static async Task<bool> InstallDirectPlayAsync() => await Task.Run(() => InstallDirectPlay());

        public static Profile Resolve(Profile profile)
        {
            if (profile.Region != null && ((Region)profile.Region).IsInstalled())
            {
                return profile;
            }

            if (!profile.UseSteam)
            {
                var region = ClientInfo.InstalledRegions.Cast<Region?>().FirstOrDefault(r => r?.GetPOLInstallDirectory() != null);
                if (region != null)
                {
                    return profile.With(Region: region);
                }
            }

            var steamRegion =
                ClientInfo.InstalledRegions.Cast<Region?>().FirstOrDefault(r => r?.GetInstalledSteamAppIds().Any() == true) ??
                Enum.GetValues(typeof(Region)).Cast<Region?>().FirstOrDefault(r => r?.GetOwnedSteamAppIds().Any() == true);
            return steamRegion != null ? profile.With(UseSteam: true, Region: steamRegion) : profile;
        }

        [RemoteCallable]
        public static void Launch(Profile profile, CancellationToken token)
        {
            try
            {
                LaunchAsync(profile, null, token).Wait();
            }
            catch (AggregateException e)
            {
                ExceptionDispatchInfo.Capture(e.Flatten().InnerException).Throw();
            }
        }

        public static async Task LaunchAsync(Profile profile, IProgress<ProgressDetail<LaunchStatus>> progress,
            CancellationToken token)
        {
            profile = Resolve(profile);
            if (CheckPermissions(profile))
            {
                Process process = null;
                try
                {
                    var status = profile.Region?.IsInstalled() == true ? LaunchStatus.Launching : LaunchStatus.Installing;
                    progress?.Report(ProgressDetail.Create(status));
                    using (var injector = await CreateInjectorAsync(profile, token))
                    {
                        process = injector.Process;

                        using (var settings = new SettingsChannel(injector.Process, profile.Settings))
                        {
                            await injector.Inject(CorePath);

                            progress?.Report(ProgressDetail.Create(LaunchStatus.TransferringSettings));
                            await settings.FinishAsync(token);
                        }
                    }

                    progress?.Report(ProgressDetail.Create(LaunchStatus.WaitingForWindow));
                    try
                    {
                        _ = process.WaitForInputIdle();
                        if (!Program.IsMono)
                        {
                            while (process.MainWindowHandle == IntPtr.Zero)
                            {
                                await Task.Delay(100, token);
                                process.Refresh();
                            }
                        }
                    }
                    catch (InvalidOperationException) { }

                    process = null;
                }
                catch (OperationCanceledException)
                {
                    if (process != null)
                    {
                        try
                        {
                            process.Kill();
                        }
                        catch (Win32Exception) { }
                        catch (InvalidOperationException) { }
                    }
                    throw;
                }
                finally
                {
                    process?.Dispose();
                }
            }
            else
            {
                var status = profile.Region?.IsInstalled() == true ? LaunchStatus.Elevating : LaunchStatus.Installing;
                progress?.Report(ProgressDetail.Create(status));
                await Program.ElevateAsync(Launch, profile, token);
            }

            progress?.Report(ProgressDetail.Create(1, 1, LaunchStatus.Complete));
        }

        public static bool IsAdministrator()
        {
            var identity = WindowsIdentity.GetCurrent();
            var principal = new WindowsPrincipal(identity);
            return principal.IsInRole(WindowsBuiltInRole.Administrator);
        }

        public static bool IsElevationRequired(Profile profile)
        {
            profile = Resolve(profile);
            if (!IsAdministrator())
            {
                if (profile.RunAsAdmin)
                {
                    return true;
                }

                if (!((Region)profile.Region).IsInstalled())
                {
                    return true;
                }

                var directories = GetDirectories(profile);
                return !directories.Any() || !directories.All(p => CanWrite(p));
            }
            return false;
        }

        public static void FixAccessControl(Profile profile) => FixAccessControlAsync(profile).Wait();

        public static Task FixAccessControlAsync(Profile profile)
        {
            profile = Resolve(profile);
            return FixAccessControlAsync(GetDirectories(profile).Where(p => !CanWrite(p)).ToArray());
        }

        [ComImport]
        [Guid("743F1DC6-5ABA-429F-8BDF-C54D03253DC2")]
        private class DirectPlay8ClientStub
        {
        };

        [RemoteCallable]
        private static void FixAccessControl(string[] paths, CancellationToken token) => FixAccessControlAsync(paths).Wait();

        private static async Task FixAccessControlAsync(string[] paths)
        {
            if (IsAdministrator())
            {
                foreach (var p in paths)
                {
                    SetAccessControl(p);
                }
            }
            else
            {
                await Program.ElevateAsync(FixAccessControl, paths.ToArray(), CancellationToken.None);
            }
        }

        private static string GetCorePath()
        {
            var path = new Uri(typeof(Launcher).Assembly.EscapedCodeBase).LocalPath;
            return Path.Combine(Path.GetDirectoryName(path), "core.dll");
        }

        private static bool CheckPermissions(Profile profile)
        {
            if (!IsElevationRequired(profile))
            {
                foreach (var path in GetPaths(profile))
                {
                    SetAsInvokerFlag(path);
                }
                return true;
            }
            return false;
        }

        private static async Task<Injector> CreateInjectorAsync(Profile profile, CancellationToken token)
        {
            var region = (Region)profile.Region;
            if (profile.UseSteam && Steam.IsInstalled)
            {
                var appId = region.GetInstalledSteamAppIds().Cast<long?>().FirstOrDefault();
                if (appId != null)
                {
                    var installDirs =
                        from r in ClientInfo.InstalledRegions
                        let dir = r.GetPOLInstallDirectory()
                        where !string.IsNullOrEmpty(dir)
                        select dir;

                    var steamDirs =
                        from r in ClientInfo.InstalledRegions
                        from id in r.GetInstalledSteamAppIds()
                        let path = Steam.GetInstalledGame(id)?.InstallDirectory
                        where path != null
                        select path;

                    installDirs = installDirs.Union(steamDirs).ToList();

                    var process = Steam.LaunchAsync((long)appId, p =>
                    {
                        if (installDirs.Any())
                        {
                            var executable = p.MainModule.FileName;
                            return installDirs.Any(path => executable.StartsWith(path));
                        }
                        return false;
                    }, token);

                    if (!installDirs.Any())
                    {
                        ;
                        process = Task.WhenAll(process, WaitForInstallAsync(token)).ContinueWith(t => process.IsCompleted ? process.Result : null, token);
                    }

                    var temp = await process;
                    token.ThrowIfCancellationRequested();
                    return new Injector(temp);
                }

                appId = region.GetOwnedSteamAppIds().Cast<long?>().FirstOrDefault();
                if (appId != null)
                {
                    return new Injector(await Steam.LaunchAsync((long)appId,
                        (p) => Path.GetFileName(p.MainModule.FileName).Equals("pol.exe", StringComparison.OrdinalIgnoreCase), token));
                }
            }
            else if (profile.Executable != null)
            {
                var args = profile.ExecutableArgs;
                args = args.Replace("{region}", profile.Region.ToString().ToLowerInvariant());
                args = args.Trim();
                return new Injector(profile.Executable, args);
            }
            else
            {
                var dir = region.GetPOLInstallDirectory();
                if (dir != null)
                {
                    // Passing /game eAZcFcB turns on the FFXI quick start button.
                    return new Injector(Path.Combine(dir, "pol.exe"), "/game", "eAZcFcB");
                }
            }

            throw new InvalidOperationException();
        }

        private static async Task WaitForInstallAsync(CancellationToken token)
        {
            IEnumerable<string> installDirs;
            do
            {
                await Task.Delay(TimeSpan.FromSeconds(1), token);

                var dirs =
                    from r in ClientInfo.InstalledRegions
                    from id in r.GetInstalledSteamAppIds()
                    let path = Steam.GetInstalledGame(id)?.InstallDirectory
                    where path != null
                    select path;

                installDirs = dirs.ToList();
            }
            while (!installDirs.Any() && !token.IsCancellationRequested);
        }

        private static bool CanWrite(string path)
        {
            try
            {
                while (true)
                {
                    try
                    {
                        var tempPath = Path.Combine(path, Path.GetRandomFileName());
                        using (new FileStream(tempPath, FileMode.CreateNew, FileAccess.ReadWrite, FileShare.None, 1,
                            FileOptions.DeleteOnClose))
                        { }
                        return true;
                    }
                    catch (IOException e)
                    {
                        if (e.HResult != unchecked((int)0x80070050)) // ERROR_FILE_EXISTS
                        {
                            throw;
                        }
                    }
                }
            }
            catch (UnauthorizedAccessException) { }
            return false;
        }

        private static IList<string> GetDirectories(Profile profile)
        {
            if (profile.Region?.IsInstalled() == true)
            {
                string pol;
                string ffxi;

                var region = (Region)profile.Region;

                if (profile.UseSteam && region.GetInstalledSteamAppIds().Any() == true)
                {
                    pol = Region.JP.GetPOLInstallDirectory();
                    ffxi = Region.JP.GetFF11InstallDirectory();
                    if (pol != null && ffxi != null)
                    {
                        return new List<string> { pol, ffxi };
                    }

                    pol = Region.NA.GetPOLInstallDirectory();
                    ffxi = Region.NA.GetFF11InstallDirectory();
                    if (pol != null && ffxi != null)
                    {
                        return new List<string> { pol, ffxi };
                    }
                }

                pol = region.GetPOLInstallDirectory();
                ffxi = region.GetFF11InstallDirectory();

                if (pol != null && ffxi != null)
                {
                    return new List<string> { pol, ffxi };
                }
            }

            return new List<string>();
        }

        private static IList<string> GetPaths(Profile profile)
        {
            string pol;
            string ffxi;

            var region = (Region)profile.Region;
            if (profile.UseSteam && region.GetInstalledSteamAppIds().Any())
            {
                pol = Region.JP.GetPOLInstallDirectory();
                ffxi = Region.JP.GetFF11InstallDirectory();
                if (pol != null && ffxi != null)
                {
                    return new List<string>
                    {
                        Path.Combine(pol, "pol.exe"),
                        Path.Combine(pol, "util", "startpol.exe"),
                        Path.Combine(ffxi, "polboot.exe"),
                    };
                }

                pol = Region.NA.GetPOLInstallDirectory();
                ffxi = Region.NA.GetFF11InstallDirectory();
                if (pol != null && ffxi != null)
                {
                    return new List<string>
                    {
                        Path.Combine(pol, "pol.exe"),
                        Path.Combine(pol, "util", "startpol.exe"),
                        Path.Combine(ffxi, "polboot.exe"),
                    };
                }

                pol = region.GetPOLInstallDirectory();
                ffxi = region.GetFF11InstallDirectory();
                if (pol != null && ffxi != null)
                {
                    return new List<string>
                    {
                        Path.Combine(pol, "pol.exe"),
                        Path.Combine(pol, "util", "startpol.exe"),
                        Path.Combine(ffxi, "polboot.exe"),
                    };
                }
            }

            pol = region.GetPOLInstallDirectory();
            if (pol != null)
            {
                return new List<string>
                {
                    Path.Combine(pol, "pol.exe"),
                    Path.Combine(pol, "util", "startpol.exe"),
                };
            }

            pol = region.GetPOLInstallDirectory();
            return pol != null
                ? new List<string>
                {
                    Path.Combine(pol, "pol.exe"),
                    Path.Combine(pol, "util", "startpol.exe"),
                }
                : (IList<string>)new List<string>();
        }

        private static void SetAsInvokerFlag(string path)
        {
            using (var hkcu = RegistryKey.OpenBaseKey(RegistryHive.CurrentUser, RegistryView.Registry64))
            using (var key = hkcu.CreateSubKey(@"SOFTWARE\Microsoft\Windows NT\CurrentVersion\AppCompatFlags\Layers"))
            {
                var oldValue = key.GetValue("path")?.ToString() ?? "";

                var flags =
                    from f in oldValue.Split((char[])null, StringSplitOptions.RemoveEmptyEntries)
                    where f != "RUNASADMINISTRATOR" && f != "RUNASINVOKER"
                    select f;
                flags = flags.Concat(new[] { "RUNASINVOKER" });

                var newValue = string.Join(" ", flags);

                if (newValue != oldValue)
                {
                    key.SetValue(path, newValue, RegistryValueKind.String);
                }
            }
        }

        private static void SetAccessControl(string path)
        {
            if (!string.IsNullOrWhiteSpace(path))
            {
                var info = new DirectoryInfo(path.Trim());
                var security = info.GetAccessControl();
                var rule = new FileSystemAccessRule(new SecurityIdentifier(WellKnownSidType.BuiltinUsersSid, null),
                    FileSystemRights.Modify | FileSystemRights.DeleteSubdirectoriesAndFiles,
                    InheritanceFlags.ContainerInherit | InheritanceFlags.ObjectInherit, PropagationFlags.None,
                    AccessControlType.Allow);
                security.AddAccessRule(rule);
                info.SetAccessControl(security);
            }
        }
    }
}
