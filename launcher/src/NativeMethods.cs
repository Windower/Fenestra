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
    using Microsoft.Win32.SafeHandles;
    using System;
    using System.Diagnostics.CodeAnalysis;
    using System.Runtime.InteropServices;

    internal static class NativeMethods
    {
        internal const int STATUS_SUCCESS = 0;

        internal const int E_NOTIMPL = unchecked((int) 0x80004001);

        internal const uint ATTACH_PARENT_PROCESS = unchecked((uint)-1);

        internal static readonly Guid FOLDERID_SavedGames = new Guid("4C5C32FF-BB9D-43b0-B5B4-2D72E54EAAA4");

        [DllImport("ntdll.dll", CallingConvention = CallingConvention.Winapi)]
        internal static extern int RtlGetVersion(ref RTL_OSVERSIONINFOW lpVersionInformation);

        [SuppressMessage("Microsoft.Interoperability", "CA1400")]
        [DllImport("ntdll.dll", CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.LPStr)]
        internal static extern string wine_get_version();

        [SuppressMessage("Microsoft.Interoperability", "CA1400")]
        [DllImport("ntdll.dll", CallingConvention = CallingConvention.Cdecl)]
        [return: MarshalAs(UnmanagedType.LPStr)]
        internal static extern void wine_get_host_version([MarshalAs(UnmanagedType.LPStr)]out string sysname,
            [MarshalAs(UnmanagedType.LPStr)]out string release);

        [DllImport("kernel32.dll", CallingConvention = CallingConvention.Winapi)]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool AttachConsole(uint dwProcessId);

        [DllImport("kernel32.dll", CallingConvention = CallingConvention.Winapi)]
        internal static extern uint GetCurrentThreadId();

        [DllImport("kernel32.dll", CallingConvention = CallingConvention.Winapi, SetLastError = true, CharSet = CharSet.Unicode)]
        internal static extern IntPtr LoadLibrary(string lpFileName);

        [DllImport("shell32.dll", CallingConvention = CallingConvention.Winapi)]
        internal static extern int SetCurrentProcessExplicitAppUserModelID([MarshalAs(UnmanagedType.LPWStr)] string AppID);

        [DllImport("shell32.dll", CallingConvention = CallingConvention.Winapi)]
        internal static extern int SHGetKnownFolderPath(ref Guid rfid, int dwFlags, IntPtr hToken,
            out SafeCoTaskMemHandle ppszPath);

        [DllImport("dbghelp.dll", CallingConvention = CallingConvention.Winapi)]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool MiniDumpWriteDump(IntPtr hProcess, int ProcessId, SafeFileHandle hFile, int DumpType,
            ref MINIDUMP_EXCEPTION_INFORMATION ExceptionParam, IntPtr UserStreamParam, IntPtr CallackParam);

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        internal struct RTL_OSVERSIONINFOW
        {
            public uint dwOSVersionInfoSize;
            public uint dwMajorVersion;
            public uint dwMinorVersion;
            public uint dwBuildNumber;
            public uint dwPlatformId;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
            public string szCSDVersion;
        }

        [SuppressMessage("Microsoft.Design", "CA1049")]
        [StructLayout(LayoutKind.Sequential)]
        internal struct MINIDUMP_EXCEPTION_INFORMATION
        {
            public uint ThreadId;
            [SuppressMessage("Microsoft.Reliability", "CA2006")]
            public IntPtr ExceptionPointers;
            [MarshalAs(UnmanagedType.Bool)]
            public bool ClientPointers;
        }
    }
}
