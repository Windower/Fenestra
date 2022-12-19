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
    using System.Collections.Generic;
    using System.Runtime.InteropServices;
    using System.Linq;
    using System;
    using System.ComponentModel;
    using System.Collections.Immutable;

    public static class DisplayInfo
    {
        public static IImmutableList<DisplayDevice> Devices { get; } = GetDisplayDevices().ToImmutableArray();
        public static DisplayDevice PrimaryDevice { get; } = (from d in Devices where d.IsPrimary select d).FirstOrDefault();

        private static IEnumerable<DisplayDevice> GetDisplayDevices()
        {
            IEnumerable<DisplayDevice> devices;
            try
            {
                devices = GetDisplayDevicesWDDM();
            }
            catch (Win32Exception e)
            {
                if (e.ErrorCode != NativeMethods.ERROR_NOT_SUPPORTED)
                {
                    throw;
                }
                devices = GetDisplayDevicesFallback();
            }

            foreach (var d in devices)
            {
                var resolutions = new HashSet<Dimension>();

                var mode = default(NativeMethods.DEVMODE);
                for (uint i = 0; NativeMethods.EnumDisplaySettings(d.DeviceName, i, ref mode); i++)
                {
                    resolutions.Add(new Dimension((int)mode.dmPelsWidth, (int)mode.dmPelsHeight));
                }

                var temp = d.With(
                    CurrentDpi: GetDpi(d.CurrentPosition),
                    AvailableResolutions: resolutions
                        .OrderByDescending(x => (ulong)x.Height << 32 | (uint)x.Width)
                        .ToImmutableList());

                yield return temp;
            }
        }

        private static IEnumerable<DisplayDevice> GetDisplayDevicesWDDM()
        {
            var result = NativeMethods.GetDisplayConfigBufferSizes(NativeMethods.QDC_ONLY_ACTIVE_PATHS,
                out uint pathCount, out uint modeCount);
            if (result != NativeMethods.ERROR_SUCCESS)
            {
                throw new Win32Exception(result);
            }

            var paths = new NativeMethods.DISPLAYCONFIG_PATH_INFO[pathCount];
            var modes = new NativeMethods.DISPLAYCONFIG_MODE_INFO[modeCount];
            result = NativeMethods.QueryDisplayConfig(NativeMethods.QDC_ONLY_ACTIVE_PATHS, ref pathCount, paths,
                ref modeCount, modes, IntPtr.Zero);
            if (result != NativeMethods.ERROR_SUCCESS)
            {
                throw new Win32Exception(result);
            }

            foreach (var p in paths)
            {
                var source = default(NativeMethods.DISPLAYCONFIG_SOURCE_DEVICE_NAME);
                source.header.type = NativeMethods.DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
                source.header.size = (uint)Marshal.SizeOf(typeof(NativeMethods.DISPLAYCONFIG_SOURCE_DEVICE_NAME));
                source.header.adapterId = p.sourceInfo.adapterId;
                source.header.id = p.sourceInfo.id;
                result = NativeMethods.DisplayConfigGetDeviceInfo(ref source);
                if (result != NativeMethods.ERROR_SUCCESS)
                {
                    throw new Win32Exception(result);
                }

                var target = default(NativeMethods.DISPLAYCONFIG_TARGET_DEVICE_NAME);
                target.header.type = NativeMethods.DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
                target.header.size = (uint)Marshal.SizeOf(typeof(NativeMethods.DISPLAYCONFIG_TARGET_DEVICE_NAME));
                target.header.adapterId = p.targetInfo.adapterId;
                target.header.id = p.targetInfo.id;

                result = NativeMethods.DisplayConfigGetDeviceInfo(ref target);
                if (result != NativeMethods.ERROR_SUCCESS)
                {
                    throw new Win32Exception(result);
                }

                if (p.sourceInfo.modeInfoIdx != NativeMethods.DISPLAYCONFIG_PATH_MODE_IDX_INVALID)
                {
                    if (modes[p.sourceInfo.modeInfoIdx].infoType == NativeMethods.DISPLAYCONFIG_MODE_INFO_TYPE_SOURCE)
                    {
                        var mode = modes[p.sourceInfo.modeInfoIdx].sourceMode;

                        var isPrimary = mode.position.x == 0 && mode.position.y == 0;
                        var deviceName = source.viewGdiDeviceName;
                        var friendlyName = target.monitorFriendlyDeviceName;
                        var currentResolution = new Dimension((int)mode.width, (int)mode.height);
                        var currentPosition = new Point(mode.position.x, mode.position.y);
                        yield return new DisplayDevice(deviceName, friendlyName, isPrimary,
                            currentResolution, currentPosition, 96, null);
                    }
                }
            }
        }

        private static IEnumerable<DisplayDevice> GetDisplayDevicesFallback()
        {
            var info = default(NativeMethods.DISPLAY_DEVICE);
            info.cb = (uint)Marshal.SizeOf(typeof(NativeMethods.DISPLAY_DEVICE));
            for (uint i = 0; NativeMethods.EnumDisplayDevices(null, i, ref info, 0); i++)
            {
                if ((info.StateFlags & NativeMethods.DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) != 0)
                {
                    var mode = default(NativeMethods.DEVMODE);
                    if (NativeMethods.EnumDisplaySettings(info.DeviceName, NativeMethods.ENUM_CURRENT_SETTINGS, ref mode))
                    {
                        var isPrimary = (info.StateFlags & NativeMethods.DISPLAY_DEVICE_PRIMARY_DEVICE) != 0;
                        var deviceName = info.DeviceName;
                        var friendlyName = info.DeviceName;
                        if (NativeMethods.EnumDisplayDevices(info.DeviceName, 0, ref info, 0))
                        {
                            friendlyName = info.DeviceString;
                        }
                        var currentResolution = new Dimension((int)mode.dmPelsWidth, (int)mode.dmPelsHeight);
                        var currentPosition = new Point(mode.dmPosition.x, mode.dmPosition.y);
                        yield return new DisplayDevice(deviceName, friendlyName, isPrimary,
                            currentResolution, currentPosition, 96, null);
                    }
                }
            }
        }

        private static int GetDpi(Point position)
        {
            try
            {
                var monitor = NativeMethods.MonitorFromPoint(new NativeMethods.POINT { x = position.X, y = position.Y },
                    NativeMethods.MONITOR_DEFAULTTOPRIMARY);
                Marshal.ThrowExceptionForHR(NativeMethods.GetDpiForMonitor(monitor, NativeMethods.MDT_EFFECTIVE_DPI,
                    out uint dpiX, out uint dpiY));
                return (int)dpiX;
            }
            catch (EntryPointNotFoundException) { }
            catch (DllNotFoundException) { }

            return GetSystemDpi();
        }

        private static int GetSystemDpi()
        {
            using (var dc = NativeMethods.GetDC(IntPtr.Zero))
            {
                if (!dc.IsInvalid)
                {
                    return NativeMethods.GetDeviceCaps(dc, NativeMethods.LOGPIXELSX);
                }
            }
            return 96;
        }
    }
}
