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

namespace Boiler
{
    using Microsoft.Win32;
    using System;
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Diagnostics.CodeAnalysis;
    using System.IO;
    using System.Linq;
    using System.Security;
    using System.Threading;
    using System.Threading.Tasks;

    public static class Steam
    {
        public static bool IsInstalled
        {
            get
            {
                var path = SteamDirectory;
                return path != null && Directory.Exists(path);
            }
        }

        public static string SteamDirectory
        {
            get
            {
                using (var hklm = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry32))
                using (var steam = hklm.OpenSubKey(@"SOFTWARE\Valve\Steam"))
                {
                    return steam?.GetValue("InstallPath")?.ToString();
                }
            }
        }

        public static bool IsSteamRunning
        {
            get
            {
                using (var hklm = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry32))
                using (var steam = hklm.OpenSubKey(@"SOFTWARE\Valve\Steam"))
                {
                    if (steam.GetValue("SteamPID") is int pid)
                    {
                        return Process.GetProcesses().Any(p => p.Id == pid && p.ProcessName.Equals("steam",
                            StringComparison.OrdinalIgnoreCase));
                    }
                }

                return false;
            }
        }

        public static int? SteamId
        {
            get
            {
                if (IsInstalled)
                {
                    if (IsSteamRunning)
                    {
                        return LastLoggedInSteamId;
                    }

                    var id = AutoLogInSteamId;
                    if (id == null)
                    {
                        var ids = SteamIds.Take(2).ToArray();
                        if (ids.Length == 1)
                        {
                            id = ids[0];
                        }
                    }

                    return id;
                }

                return null;
            }
        }

        [SuppressMessage("Microsoft.Naming", "CA1726:UsePreferredTerms", MessageId = "LogIn")]
        public static string AutoLogInUserName
        {
            get
            {
                if (IsInstalled)
                {
                    IVdfValue data = null;
                    try
                    {
                        var path = Path.Combine(SteamDirectory, "config", "SteamAppData.vdf");
                        using (var stream = new FileStream(path, FileMode.Open, FileAccess.Read))
                        {
                            data = VdfReader.Load(stream);
                        }
                    }
                    catch (IOException) { }
                    catch (SecurityException) { }
                    catch (UnauthorizedAccessException) { }
                    catch (NotSupportedException) { }

                    if (data != null && data.TryGetValue("SteamAppData", out var settings))
                    {
                        if (settings.TryGetValue("RememberPassword", out var temp) && temp.Value == "1" &&
                            settings.TryGetValue("AutoLoginUser", out temp))
                        {
                            return temp.Value;
                        }
                    }
                }

                return null;
            }
        }

        [SuppressMessage("Microsoft.Naming", "CA1726:UsePreferredTerms", MessageId = "LogIn")]
        public static int? AutoLogInSteamId
        {
            get
            {
                if (IsInstalled)
                {
                    IVdfValue data = null;
                    try
                    {
                        var path = Path.Combine(SteamDirectory, "config", "loginusers.vdf");
                        using (var stream = new FileStream(path, FileMode.Open, FileAccess.Read))
                        {
                            data = VdfReader.Load(stream);
                        }
                    }
                    catch (IOException) { }
                    catch (SecurityException) { }
                    catch (UnauthorizedAccessException) { }
                    catch (NotSupportedException) { }

                    if (data != null && data.TryGetValue("users", out var users))
                    {
                        var name = AutoLogInUserName;
                        foreach (var p in users)
                        {
                            if (ulong.TryParse(p.Key, out var id) && p.Value.TryGetValue("AccountName", out var temp) &&
                                temp.Value == name)
                            {
                                return (int)id;
                            }
                        }
                    }
                }

                return null;
            }
        }

        public static int? LastLoggedInSteamId
        {
            get
            {
                if (IsInstalled)
                {
                    IVdfValue data = null;
                    try
                    {
                        var path = Path.Combine(SteamDirectory, "config", "loginusers.vdf");
                        using (var stream = new FileStream(path, FileMode.Open, FileAccess.Read))
                        {
                            data = VdfReader.Load(stream);
                        }
                    }
                    catch (IOException) { }
                    catch (SecurityException) { }
                    catch (UnauthorizedAccessException) { }
                    catch (NotSupportedException) { }

                    int? steamId = null;
                    if (data != null && data.TryGetValue("users", out var users))
                    {
                        long max = 0;
                        foreach (var p in users)
                        {
                            if (long.TryParse(p.Key, out long id) && p.Value.TryGetValue("Timestamp", out var temp)
                                && long.TryParse(temp.Value, out long timestamp) && timestamp > max)
                            {
                                max = timestamp;
                                steamId = (int)id;
                            }
                        }
                    }

                    return steamId;
                }

                return null;
            }
        }

        public static IEnumerable<int> SteamIds
        {
            get
            {
                if (IsInstalled)
                {
                    IVdfValue data = null;
                    try
                    {
                        var path = Path.Combine(SteamDirectory, "config", "loginusers.vdf");
                        using (var stream = new FileStream(path, FileMode.Open, FileAccess.Read))
                        {
                            data = VdfReader.Load(stream);
                        }
                    }
                    catch (IOException) { }
                    catch (SecurityException) { }
                    catch (UnauthorizedAccessException) { }
                    catch (NotSupportedException) { }

                    if (data != null && data.TryGetValue("users", out var users))
                    {
                        foreach (var p in users)
                        {
                            if (long.TryParse(p.Key, out long id))
                            {
                                yield return (int)id;
                            }
                        }
                    }
                }
            }
        }

        public static IEnumerable<string> LibraryFolders
        {
            get
            {
                if (IsInstalled)
                {
                    var root = SteamDirectory;

                    yield return root;

                    IVdfValue data = null;
                    try
                    {
                        var path = Path.Combine(root, "steamapps", "libraryfolders.vdf");
                        using (var stream = new FileStream(path, FileMode.Open, FileAccess.Read))
                        {
                            data = VdfReader.Load(stream);
                        }
                    }
                    catch (IOException) { }
                    catch (SecurityException) { }
                    catch (UnauthorizedAccessException) { }
                    catch (NotSupportedException) { }

                    if (data != null && data.TryGetValue("LibraryFolders", out var folders))
                    {
                        foreach (var p in folders)
                        {
                            if (long.TryParse(p.Key, out long dummy) && !string.IsNullOrWhiteSpace(p.Value.Value))
                            {
                                yield return p.Value.Value;
                            }
                        }
                    }
                }
            }
        }

        public static IEnumerable<SteamGame> InstalledGames
        {
            get
            {
                if (IsInstalled)
                {
                    foreach (var file in InstalledAppManifests)
                    {
                        IVdfValue data = null;
                        try
                        {
                            using (var stream = new FileStream(file, FileMode.Open, FileAccess.Read))
                            {
                                data = VdfReader.Load(stream);
                            }
                        }
                        catch (IOException) { }
                        catch (SecurityException) { }
                        catch (UnauthorizedAccessException) { }
                        catch (NotSupportedException) { }

                        if (data != null && data.TryGetValue("AppState", out var app))
                        {
                            if (app.TryGetValue("appid", out var appIdValue) && long.TryParse(appIdValue.Value, out long appId) &&
                                app.TryGetValue("name", out var name) && app.TryGetValue("installdir", out var installDir))
                            {
                                var installPath = installDir.Value;
                                if (!string.IsNullOrWhiteSpace(installPath))
                                {
                                    installPath = Path.Combine(Path.GetDirectoryName(file), "common", installPath);
                                }
                                yield return new SteamGame(appId, name.Value, installPath);
                            }
                        }
                    }
                }
            }
        }

        public static IEnumerable<SteamGame> OwnedGames
        {
            get
            {
                if (IsInstalled)
                {
                    var root = SteamDirectory;
                    foreach (var id in SteamIds)
                    {
                        IVdfValue data = null;
                        try
                        {
                            var path = Path.Combine(root, "userdata", id.ToString(), "config", "localconfig.vdf");
                            using (var stream = new FileStream(path, FileMode.Open, FileAccess.Read))
                            {
                                data = VdfReader.Load(stream);
                            }
                        }
                        catch (IOException) { }
                        catch (SecurityException) { }
                        catch (UnauthorizedAccessException) { }
                        catch (NotSupportedException) { }

                        if (data != null && data.TryGetValue("UserLocalConfigStore", out var localConfigStore))
                        {
                            var temp = localConfigStore;
                            var appsFound =
                                temp.TryGetValue("Software", out temp) &&
                                temp.TryGetValue("Valve", out temp) &&
                                temp.TryGetValue("Steam", out temp) &&
                                temp.TryGetValue("Apps", out temp);

                            if (appsFound)
                            {
                                foreach (var app in temp)
                                {
                                    if (long.TryParse(app.Key, out long appId))
                                    {
                                        yield return new SteamGame(appId, null, null);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        public static IEnumerable<NonSteamGame> GetNonSteamGames(int steamId)
        {
            if (IsInstalled)
            {
                IVdfValue data = null;
                try
                {
                    var path = Path.Combine(SteamDirectory, "userdata", steamId.ToString(), "config", "shortcuts.vdf");
                    using (var stream = new FileStream(path, FileMode.Open, FileAccess.Read))
                    {
                        data = VdfReader.Load(stream);
                    }
                }
                catch (IOException) { }
                catch (SecurityException) { }
                catch (UnauthorizedAccessException) { }
                catch (NotSupportedException) { }

                if (data != null && data.TryGetValue("shortcuts", out var shortcuts))
                {
                    foreach (var p in shortcuts)
                    {
                        if (long.TryParse(p.Key, out var dummy) && p.Value.HasSubValues)
                        {
                            if (p.Value.TryGetValue("AppName", out var name) && p.Value.TryGetValue("exe", out var target))
                            {
                                yield return new NonSteamGame(name.Value, target.Value);
                            }
                        }
                    }
                }
            }
        }

        public static IGame GetInstalledGame(long appId)
        {
            if (IsInstalled)
            {
                var games = InstalledGames.Cast<IGame>();
                var steamId = SteamId;
                if (steamId != null)
                {
                    games = games.Union(GetNonSteamGames((int)steamId).Cast<IGame>());
                }
                return games.FirstOrDefault(g => g.AppId == appId);
            }

            return null;
        }

        public static Task<Process> LaunchAsync(long appId, CancellationToken token)
        {
            if (IsInstalled)
            {
                var path = GetPath(appId);
                return LaunchAsync(appId, p => p.MainModule.FileName.StartsWith(path, StringComparison.OrdinalIgnoreCase), token);
            }

            return null;
        }

        public static async Task<Process> LaunchAsync(long appId, Predicate<Process> predicate, CancellationToken token)
        {
            if (IsInstalled)
            {
                var temp = await Task.Run(() =>
                {
                    Process process = null;

                    NativeMethods.WinEventProc callback =
                        (IntPtr hook, uint e, IntPtr hwnd, int obj, int child, uint thread, uint time) =>
                        {
                            if (NativeMethods.GetWindowThreadProcessId(hwnd, out uint pid) != 0)
                            {
                                try
                                {
                                    var tempProcess = Process.GetProcessById((int)pid);
                                    if (predicate(tempProcess))
                                    {
                                        process = tempProcess;
                                    }
                                }
                                catch (Win32Exception) { }
                            }
                        };

                    using (NativeMethods.SetWinEventHook(NativeMethods.EVENT_OBJECT_CREATE, NativeMethods.EVENT_OBJECT_CREATE,
                        IntPtr.Zero, callback, 0, 0, NativeMethods.WINEVENT_OUTOFCONTEXT))
                    {
                        Process.Start("steam://rungameid/" + appId);

                        while (process == null && !token.IsCancellationRequested)
                        {
                            while (NativeMethods.PeekMessage(out var msg, IntPtr.Zero, 0, 0, NativeMethods.PM_REMOVE))
                            {
                                NativeMethods.TranslateMessage(ref msg);
                                NativeMethods.DispatchMessage(ref msg);
                            }

                            if (process == null && !token.IsCancellationRequested)
                            {
                                var handle = token.WaitHandle.SafeWaitHandle;
                                var result = NativeMethods.MsgWaitForMultipleObjects(1, ref handle, false,
                                    NativeMethods.INFINITE, NativeMethods.QS_ALLINPUT);
                                switch (result)
                                {
                                    case NativeMethods.WAIT_OBJECT_0 + 0:
                                    case NativeMethods.WAIT_OBJECT_0 + 1:
                                        break;
                                    case NativeMethods.WAIT_ABANDONED:
                                        throw new AbandonedMutexException();
                                    case NativeMethods.WAIT_TIMEOUT:
                                        throw new TimeoutException();
                                    case NativeMethods.WAIT_FAILED:
                                        throw new Win32Exception();
                                    default:
                                        throw new InvalidOperationException();
                                }
                            }
                        }
                    }

                    GC.KeepAlive(callback);

                    return process;
                }, token);
                token.ThrowIfCancellationRequested();
                return temp;
            }

            return null;
        }

        private static IEnumerable<string> InstalledAppManifests
        {
            get
            {
                foreach (var folder in LibraryFolders)
                {
                    IEnumerable<string> files = null;
                    try
                    {
                        files = Directory.EnumerateFiles(Path.Combine(folder, "steamapps"), "appmanifest_*.acf");
                    }
                    catch (IOException) { }
                    catch (SecurityException) { }
                    catch (UnauthorizedAccessException) { }

                    if (files != null)
                    {
                        foreach (var file in files)
                        {
                            yield return file;
                        }
                    }
                }
            }
        }

        private static string GetPath(long appId)
        {
            foreach (var g in InstalledGames)
            {
                if (g.AppId == appId)
                {
                    return g.InstallDirectory;
                }
            }

            foreach (var u in SteamIds)
            {
                foreach (var g in GetNonSteamGames(u))
                {
                    if (g.AppId == appId)
                    {
                        return g.Target.Replace("\"", "");
                    }
                }
            }

            return string.Empty;
        }
    }
}
