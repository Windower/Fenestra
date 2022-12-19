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

namespace Windower.PlayOnline
{
    using Microsoft.Win32;
    using System.Collections.Generic;
    using System.Linq;
    using Boiler;
    using System.IO;
    using System;
    using System.Collections.Immutable;

    internal static class ExtensionMethods
    {
        private static readonly IImmutableDictionary<Region, string> InstallKeys = ImmutableDictionary<Region, string>.Empty
            .Add(Region.JP, @"SOFTWARE\PlayOnline\InstallFolder")
            .Add(Region.NA, @"SOFTWARE\PlayOnlineUS\InstallFolder")
            .Add(Region.EU, @"SOFTWARE\PlayOnlineEU\InstallFolder");

        private static readonly IImmutableDictionary<Region, IImmutableList<long>> KnownAppIds =
            ImmutableDictionary<Region, IImmutableList<long>>.Empty
            .Add(Region.JP, ImmutableArray<long>.Empty)
            .Add(Region.NA, new long[] { 23360, 39250, 230330 }.ToImmutableArray())
            .Add(Region.EU, new long[] { 23390, 39260, 230350 }.ToImmutableArray());

        private static IImmutableDictionary<Region, bool> Installed = ImmutableDictionary<Region, bool>.Empty;
        private static IImmutableDictionary<Region, bool> Owned = ImmutableDictionary<Region, bool>.Empty;
        private static IImmutableDictionary<Region, string> POLInstallDirectory = ImmutableDictionary<Region, string>.Empty;
        private static IImmutableDictionary<Region, string> FF11InstallDirectory = ImmutableDictionary<Region, string>.Empty;
        private static IImmutableDictionary<Region, IImmutableList<long>> InstalledSteamAppIds =
            ImmutableDictionary<Region, IImmutableList<long>>.Empty;
        private static IImmutableDictionary<Region, IImmutableList<long>> OwnedSteamAppIds =
            ImmutableDictionary<Region, IImmutableList<long>>.Empty;

        public static bool IsInstalled(this Region region)
        {
            if (!Installed.TryGetValue(region, out var result))
            {
                result = IsInstalledInternal(region);
                Installed = Installed.Add(region, result);
            }
            return result;
        }

        public static bool IsOwned(this Region region)
        {
            if (!Owned.TryGetValue(region, out var result))
            {
                result = IsOwnedInternal(region);
                Owned = Owned.Add(region, result);
            }
            return result;
        }

        public static string GetPOLInstallDirectory(this Region region)
        {
            if (!POLInstallDirectory.TryGetValue(region, out string result))
            {
                result = GetPOLInstallDirectoryInternal(region);
                POLInstallDirectory = POLInstallDirectory.Add(region, result);
            }
            return result;
        }

        public static string GetFF11InstallDirectory(this Region region)
        {
            if (!FF11InstallDirectory.TryGetValue(region, out string result))
            {
                result = GetFF11InstallDirectoryInternal(region);
                FF11InstallDirectory = FF11InstallDirectory.Add(region, result);
            }
            return result;
        }

        public static IImmutableList<long> GetInstalledSteamAppIds(this Region region)
        {
            if (!InstalledSteamAppIds.TryGetValue(region, out var result))
            {
                result = GetInstalledSteamAppIdsInternal(region).ToImmutableArray();
                InstalledSteamAppIds = InstalledSteamAppIds.Add(region, result);
            }
            return result;
        }

        public static IImmutableList<long> GetOwnedSteamAppIds(this Region region)
        {
            if (!OwnedSteamAppIds.TryGetValue(region, out var result))
            {
                result = GetOwnedSteamAppIdsInternal(region).ToImmutableArray();
                OwnedSteamAppIds = OwnedSteamAppIds.Add(region, result);
            }
            return result;
        }

        private static bool IsInstalledInternal(Region region) =>
            region.GetPOLInstallDirectory() != null && region.GetFF11InstallDirectory() != null ||
            region.GetInstalledSteamAppIds().Any();

        private static bool IsOwnedInternal(Region region) => IsInstalled(region) || region.GetOwnedSteamAppIds().Any();

        private static string GetPOLInstallDirectoryInternal(this Region region)
        {
            using (var machine = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry32))
            using (var key = machine.OpenSubKey(InstallKeys[region]))
            {
                return key?.GetValue("0001") != null
                    ? key.GetValue("1000") as string
                    : null;
            }
        }

        private static string GetFF11InstallDirectoryInternal(this Region region)
        {
            using (var machine = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry32))
            using (var key = machine.OpenSubKey(InstallKeys[region]))
            {
                return key?.GetValue("1000") != null
                    ? key.GetValue("0001") as string
                    : null;
            }
        }

        private static IEnumerable<long> GetInstalledSteamAppIdsInternal(Region region)
        {
            if (Steam.IsInstalled)
            {
                var knownIds = KnownAppIds[region];
                var regionPath = region.GetPOLInstallDirectory() ?? "<not-installed>";

                foreach (var g in Steam.InstalledGames)
                {
                    if (knownIds.Contains(g.AppId))
                    {
                        yield return g.AppId;
                    }
                    else
                    {
                        var steamPath = Path.GetFullPath(g.InstallDirectory);
                        if (steamPath.StartsWith(regionPath, StringComparison.OrdinalIgnoreCase))
                        {
                            yield return g.AppId;
                        }
                    }
                }

                var steamId = Steam.SteamId;
                if (steamId != null)
                {
                    foreach (var g in Steam.GetNonSteamGames((int)steamId))
                    {
                        var steamPath = Path.GetFullPath(g.Target);
                        if (steamPath.StartsWith(regionPath, StringComparison.OrdinalIgnoreCase))
                        {
                            yield return g.AppId;
                        }
                    }
                }
            }
        }

        private static IEnumerable<long> GetOwnedSteamAppIdsInternal(Region region)
        {
            var knownIds = KnownAppIds[region];

            foreach (var g in Steam.OwnedGames)
            {
                if (knownIds.Contains(g.AppId))
                {
                    yield return g.AppId;
                }
            }
        }
    }
}
