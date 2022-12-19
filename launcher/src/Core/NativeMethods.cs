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
    using Microsoft.Win32.SafeHandles;
    using System;
    using System.Runtime.ConstrainedExecution;
    using System.Runtime.InteropServices;
    using System.Security;
    using System.Text;

    /// <summary>
    /// Contains native Win32 methods, structures, and constants.
    /// </summary>
    internal static class NativeMethods
    {
        /// <summary>
        /// The primary thread of the new process is created in a suspended state, and does not run until the
        /// ResumeThread function is called.
        /// </summary>
        /// <remarks>
        /// See <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/ms684863.aspx#CREATE_SUSPENDED">MSDN</a> for more
        /// details.
        /// </remarks>
        internal const uint CREATE_SUSPENDED = 0x00000004;

        /// <summary>
        /// <para>Allocates memory charges (from the overall size of memory and the paging files on disk) for the specified
        /// reserved memory pages. The function also guarantees that when the caller later initially accesses the memory, the
        /// contents will be zero. Actual physical pages are not allocated unless/until the virtual addresses are actually
        /// accessed.</para>
        /// <para>To reserve and commit pages in one step, call <see cref="VirtualAllocEx"/> with <c><see cref="MEM_COMMIT"/> |
        /// <see cref="MEM_RESERVE"/></c>.</para>
        /// <para>The function fails if you attempt to commit a page that has not been reserved. The resulting error code is
        /// <c>ERROR_INVALID_ADDRESS.</c></para>
        /// <para>An attempt to commit a page that is already committed does not cause the function to fail. This means that you
        /// can commit pages without first determining the current commitment state of each page.</para>
        /// </summary>
        /// <remarks>
        /// See <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/aa366890.aspx#MEM_COMMIT">MSDN</a> for more
        /// details.
        /// </remarks>
        internal const uint MEM_COMMIT = 0x1000;

        /// <summary>
        /// <para>Reserves a range of the process's virtual address space without allocating any actual physical
        /// storage in memory or in the paging file on disk.</para>
        /// <para>You commit reserved pages by calling <see cref="VirtualAllocEx"/> again with <see cref="MEM_COMMIT"/>. To reserve
        /// and commit pages in one step, call <see cref="VirtualAllocEx"/> with <c><see cref="MEM_COMMIT"/> |
        /// <see cref="MEM_RESERVE"/></c>.</para>
        /// <para>Other memory allocation functions, such as malloc and LocalAlloc, cannot use reserved memory until it
        /// has been released.</para>
        /// </summary>
        /// <remarks>
        /// See <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/aa366890.aspx#MEM_RESERVE">MSDN</a> for more
        /// details.
        /// </remarks>
        internal const uint MEM_RESERVE = 0x2000;

        /// <summary>
        /// <para>Releases the specified region of pages. After this operation, the pages are in the free state.</para>
        /// <para>If you specify this value, <c>dwSize</c> must be <c>0</c> (zero), and <c>lpAddress</c> must point to the base
        /// address returned by the <c>VirtualAlloc</c> function when the region is reserved. The function fails if either of these
        /// conditions is not met.</para>
        /// <para>If any pages in the region are committed currently, the function first decommits, and then releases them.</para>
        /// <para>The function does not fail if you attempt to release pages that are in different states, some reserved and some
        /// committed. This means that you can release a range of pages without first determining the current commitment state.
        /// </para>
        /// <para>Do not use this value with <c>MEM_DECOMMIT</c>.</para>
        /// </summary>
        /// <remarks>
        /// See <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/aa366892.aspx#MEM_RELEASE">MSDN</a> for more
        /// details.
        /// </remarks>
        internal const uint MEM_RELEASE = 0x8000;

        /// <summary>
        /// Enables read-only or read/write access to the committed region of pages. If Data Execution Prevention is enabled,
        /// attempting to execute code in the committed region results in an access violation.
        /// </summary>
        /// <remarks>
        /// See <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/aa366786.aspx#PAGE_READWRITE">MSDN</a> for more
        /// details.
        /// </remarks>
        internal const uint PAGE_READWRITE = 0x04;

        /// <summary>
        /// Enables execute, read-only, or read/write access to the committed region of pages.
        /// </summary>
        /// <remarks>
        /// See <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/aa366786.aspx#PAGE_EXECUTE_WRITECOPY">MSDN</a> for
        /// more details.
        /// </remarks>
        internal const uint PAGE_EXECUTE_READWRITE = 0x40;

        /// <summary>
        /// SS:SP, CS:IP, FLAGS, BP. This constant is not documented on MSDN.
        /// </summary>
        /// <remarks>See winnt.h for more details.</remarks>
        internal const uint CONTEXT_CONTROL = 0x00010001;

        /// <summary>
        /// Infinite timeout
        /// </summary>
        /// <remarks>
        /// See <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/ms687032.aspx">MSDN</a> for more details.
        /// </remarks>
        internal const uint INFINITE = 0xFFFFFFFF;

        /// <summary>
        /// The state of the specified object is signaled.
        /// </summary>
        /// <remarks>
        /// See <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/ms687032.aspx">MSDN</a> for more details.
        /// </remarks>
        internal const uint WAIT_OBJECT_0 = 0x00000000;

        internal const int ERROR_SUCCESS = 0;
        internal const int ERROR_NOT_SUPPORTED = 50;

        internal const uint QDC_ONLY_ACTIVE_PATHS = 0x00000002;

        internal const uint DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME = 1;
        internal const uint DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME = 2;

        internal const uint DISPLAYCONFIG_PATH_MODE_IDX_INVALID = 0xFFFFFFFF;

        internal const uint DISPLAYCONFIG_MODE_INFO_TYPE_SOURCE = 1;

        internal const uint DISPLAY_DEVICE_ATTACHED_TO_DESKTOP = 0x00000001;
        internal const uint DISPLAY_DEVICE_PRIMARY_DEVICE = 0x00000004;

        internal const uint ENUM_CURRENT_SETTINGS = unchecked((uint)-1);

        internal const int LOGPIXELSX = 88;
        internal const uint MONITOR_DEFAULTTOPRIMARY = 0x00000001;
        internal const int MDT_EFFECTIVE_DPI = 0;

        internal const uint LIST_MODULES_32BIT = 0x1;

        internal const int IMAGE_DIRECTORY_ENTRY_EXPORT = 0;

        /// <summary>
        /// <para>Creates a new process and its primary thread. The new process runs in the security context of the calling
        /// process.</para>
        /// <para>If the calling process is impersonating another user, the new process uses the token for the calling process, not
        /// the impersonation token.To run the new process in the security context of the user represented by the impersonation
        /// token, use the <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/ms682429.aspx">CreateProcessAsUser</a>
        /// or <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/ms682431.aspx">CreateProcessWithLogonW</a>
        /// function.</para>
        /// </summary>
        /// <remarks>
        /// See <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/ms682425.aspx">MSDN</a> for more details.
        /// </remarks>
        [DllImport("kernel32.dll", CallingConvention = CallingConvention.Winapi, CharSet = CharSet.Unicode, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool CreateProcess([MarshalAs(UnmanagedType.LPWStr)] string lpApplicationName,
            [MarshalAs(UnmanagedType.LPWStr)] string lpCommandLine, IntPtr lpProcessAttributes, IntPtr lpThreadAttributes,
            [MarshalAs(UnmanagedType.Bool)] bool bInheritHandles, uint dwCreationFlags, IntPtr lpEnvironment,
            [MarshalAs(UnmanagedType.LPWStr)] string lpCurrentDirectory, [In] ref STARTUPINFO lpStartupInfo,
            out PROCESS_INFORMATION lpProcessInformation);

        /// <summary>
        /// <para>Suspends the specified thread.</para>
        /// <para>A 64-bit application can suspend a WOW64 thread using the
        /// <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/ms687400.aspx">Wow64SuspendThread</a> function.</para>
        /// </summary>
        /// <remarks>
        /// See <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/ms686345.aspx">MSDN</a> for more details.
        /// </remarks>
        [DllImport("kernel32.dll", CallingConvention = CallingConvention.Winapi, SetLastError = true)]
        internal static extern uint SuspendThread(SafeWaitHandle hThread);

        /// <summary>
        /// Decrements a thread's suspend count. When the suspend count is decremented to zero, the execution of the thread is
        /// resumed.
        /// </summary>
        /// <remarks>
        /// See <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/ms685086.aspx">MSDN</a> for more details.
        /// </remarks>
        [DllImport("kernel32.dll", CallingConvention = CallingConvention.Winapi, SetLastError = true)]
        internal static extern uint ResumeThread(SafeWaitHandle hThread);

        /// <summary>
        /// <para>Reserves, commits, or changes the state of a region of memory within the virtual address space of a specified
        /// process. The function initializes the memory it allocates to zero.</para>
        /// <para>To specify the NUMA node for the physical memory, see
        /// <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/aa366891.aspx">VirtualAllocExNuma</a>.</para>
        /// </summary>
        /// <remarks>
        /// See <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/aa366890.aspx">MSDN</a> for more details.
        /// </remarks>
        [DllImport("kernel32.dll", CallingConvention = CallingConvention.Winapi, SetLastError = true)]
        internal static extern IntPtr VirtualAllocEx(SafeWaitHandle hProcess, IntPtr lpAddress, UIntPtr dwSize,
            uint flAllocationType, uint flProtect);

        /// <summary>
        /// Releases, decommits, or releases and decommits a region of memory within the virtual address space of a specified
        /// process.
        /// </summary>
        /// <remarks>
        /// See <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/aa366894.aspx">MSDN</a> for more details.
        /// </remarks>
        [SuppressUnmanagedCodeSecurity]
        [ReliabilityContract(Consistency.WillNotCorruptState, Cer.MayFail)]
        [DllImport("kernel32.dll", CallingConvention = CallingConvention.Winapi, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool VirtualFreeEx(SafeWaitHandle hProcess, IntPtr lpAddress, UIntPtr dwSize, uint dwFreeType);

        /// <summary>
        /// Changes the protection on a region of committed pages in the virtual address space of a specified process.
        /// </summary>
        /// <remarks>
        /// See <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/aa366899.aspx">MSDN</a> for more details.
        /// </remarks>
        [DllImport("kernel32.dll", CallingConvention = CallingConvention.Winapi, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool VirtualProtectEx(SafeWaitHandle hProcess, IntPtr lpAddress, UIntPtr dwSize, uint flNewProtect,
            out uint lpflOldProtect);

        /// <summary>
        /// Reads data from an area of memory in a specified process. The entire area to be read must be accessible or the
        /// operation fails.
        /// </summary>
        /// <remarks>
        /// See <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/ms680553.aspx">MSDN</a> for more details.
        /// </remarks>
        [DllImport("kernel32.dll", CallingConvention = CallingConvention.Winapi, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool ReadProcessMemory(SafeWaitHandle hProcess, IntPtr lpBaseAddress, [Out] byte[] lpBuffer,
            UIntPtr nSize, IntPtr lpNumberOfBytesRead);

        /// <summary>
        /// Reads data from an area of memory in a specified process. The entire area to be read must be accessible or the
        /// operation fails.
        /// </summary>
        /// <remarks>
        /// See <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/ms680553.aspx">MSDN</a> for more details.
        /// </remarks>
        [DllImport("kernel32.dll", CallingConvention = CallingConvention.Winapi, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool ReadProcessMemory(SafeWaitHandle hProcess, IntPtr lpBaseAddress, IntPtr lpBuffer,
            UIntPtr nSize, IntPtr lpNumberOfBytesRead);

        /// <summary>
        /// Writes data to an area of memory in a specified process. The entire area to be written to must be accessible or the
        /// operation fails.
        /// </summary>
        /// <remarks>
        /// See <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/ms681674.aspx">MSDN</a> for more details.
        /// </remarks>
        [DllImport("kernel32.dll", CallingConvention = CallingConvention.Winapi, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool WriteProcessMemory(SafeWaitHandle hProcess, IntPtr lpBaseAddress, byte[] lpBuffer,
            UIntPtr nSize, IntPtr lpNumberOfBytesWritten);

        /// <summary>
        /// Writes data to an area of memory in a specified process. The entire area to be written to must be accessible or the
        /// operation fails.
        /// </summary>
        /// <remarks>
        /// See <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/ms681674.aspx">MSDN</a> for more details.
        /// </remarks>
        [DllImport("kernel32.dll", CallingConvention = CallingConvention.Winapi, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool WriteProcessMemory(SafeWaitHandle hProcess, SafeProcessMemoryHandle lpBaseAddress,
            byte[] lpBuffer, UIntPtr nSize, IntPtr lpNumberOfBytesWritten);

        /// <summary>
        /// <para>Retrieves the context of the specified thread.</para>
        /// <para>A 64-bit application can retrieve the context of a WOW64 thread using the
        /// <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/ms681665.aspx">Wow64GetThreadContext</a> function.
        /// </para>
        /// </summary>
        /// <remarks>
        /// See <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/ms679362.aspx">MSDN</a> for more details.
        /// </remarks>
        [DllImport("kernel32.dll", CallingConvention = CallingConvention.Winapi, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool Wow64GetThreadContext(SafeWaitHandle hThread, IntPtr lpContext);

        [DllImport("psapi.dll", CallingConvention = CallingConvention.Winapi, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool EnumProcessModulesEx(SafeWaitHandle hProcess, [Out] IntPtr[] lphModule, uint cb,
            out uint lpcbNeeded, uint dwFilterFlag);

        [DllImport("psapi.dll", CallingConvention = CallingConvention.Winapi, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool EnumProcessModules(SafeWaitHandle hProcess, [Out] IntPtr[] lphModule, uint cb,
            out uint lpcbNeeded);

        [DllImport("psapi.dll", CallingConvention = CallingConvention.Winapi, CharSet = CharSet.Unicode, SetLastError = true)]
        internal static extern uint GetModuleFileNameEx(SafeWaitHandle hProcess, IntPtr hModule, [Out] StringBuilder lpBaseName,
            uint nSize);

        /// <summary>
        /// <para>Retrieves a module handle for the specified module. The module must have been loaded by the calling process.
        /// </para>
        /// <para>To avoid the race conditions described in the Remarks section, use the
        /// <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/ms683200.aspx">GetModuleHandleEx</a> function.</para>
        /// </summary>
        /// <remarks>
        /// See <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/ms683199.aspx">MSDN</a> for more details.
        /// </remarks>
        //[DllImport("kernel32.dll", CallingConvention = CallingConvention.Winapi, CharSet = CharSet.Unicode, SetLastError = true)]
        //internal static extern IntPtr GetModuleHandle([MarshalAs(UnmanagedType.LPWStr)]string lpModuleName);

        /// <summary>
        /// Retrieves the address of an exported function or variable from the specified dynamic-link library (DLL).
        /// </summary>
        /// <remarks>
        /// See <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/ms683212.aspx">MSDN</a> for more details.
        /// </remarks>
        //[DllImport("kernel32.dll", CallingConvention = CallingConvention.Winapi, CharSet = CharSet.Ansi, BestFitMapping = false,
        //    ThrowOnUnmappableChar = true, SetLastError = true)]
        //internal static extern IntPtr GetProcAddress(IntPtr hModule, [MarshalAs(UnmanagedType.LPStr)]string lpProcName);

        /// <summary>
        /// <para>Creates a thread that runs in the virtual address space of another process.</para>
        /// <para>Use the <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/dd405484.aspx">CreateRemoteThreadEx</a>
        /// function to create a thread that runs in the virtual address space of another process and optionally specify extended
        /// attributes.</para>
        /// </summary>
        /// <remarks>
        /// See <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/ms682437.aspx">MSDN</a> for more details.
        /// </remarks>
        [DllImport("kernel32.dll", CallingConvention = CallingConvention.Winapi, SetLastError = true)]
        internal static extern IntPtr CreateRemoteThread(SafeWaitHandle hProcess, IntPtr lpThreadAttributes, UIntPtr dwStackSize,
            IntPtr lpStartAddress, SafeProcessMemoryHandle lpParameter, uint dwCreationFlags, IntPtr lpThreadId);

        /// <summary>
        /// Waits until the specified object is in the signaled state or the time-out interval elapses.
        /// </summary>
        /// <remarks>
        /// See <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/ms687032.aspx">MSDN</a> for more details.
        /// </remarks>
        [DllImport("kernel32.dll", CallingConvention = CallingConvention.Winapi, SetLastError = true)]
        internal static extern uint WaitForSingleObject(SafeWaitHandle hHandle, uint dwMilliseconds);

        /// <summary>
        /// Retrieves the termination status of the specified thread.
        /// </summary>
        /// <remarks>
        /// See <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/ms683190.aspx">MSDN</a> for more details.
        /// </remarks>
        [DllImport("kernel32.dll", CallingConvention = CallingConvention.Winapi, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool GetExitCodeThread(SafeWaitHandle hThread, out uint lpExitCode);

        [DllImport("user32.dll", CallingConvention = CallingConvention.Winapi)]
        internal static extern int GetDisplayConfigBufferSizes(uint flags, out uint numPathArrayElements,
            out uint numModeInfoArrayElements);

        [DllImport("user32.dll", CallingConvention = CallingConvention.Winapi)]
        internal static extern int QueryDisplayConfig(uint flags, ref uint numPathArrayElements,
            [Out] DISPLAYCONFIG_PATH_INFO[] PathInfoArray, ref uint numModeInfoArrayElements,
            [Out] DISPLAYCONFIG_MODE_INFO[] ModeInfoArray, IntPtr currentTopologyId);

        [DllImport("user32.dll", CallingConvention = CallingConvention.Winapi, CharSet = CharSet.Unicode)]
        internal static extern int DisplayConfigGetDeviceInfo(ref DISPLAYCONFIG_SOURCE_DEVICE_NAME requestPacket);

        [DllImport("user32.dll", CallingConvention = CallingConvention.Winapi, CharSet = CharSet.Unicode)]
        internal static extern int DisplayConfigGetDeviceInfo(ref DISPLAYCONFIG_TARGET_DEVICE_NAME requestPacket);

        [DllImport("user32.dll", CallingConvention = CallingConvention.Winapi, CharSet = CharSet.Unicode, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool EnumDisplayDevices(string lpDevice, uint iDevNum, ref DISPLAY_DEVICE lpDisplayDevice,
            uint dwFlags);

        [DllImport("user32.dll", CallingConvention = CallingConvention.Winapi, CharSet = CharSet.Unicode, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool EnumDisplaySettings(string lpszDeviceName, uint iModeNum, ref DEVMODE lpDevMode);

        [DllImport("user32.dll", CallingConvention = CallingConvention.Winapi)]
        internal static extern SafeDeviceContextHandle GetDC(IntPtr hWnd);

        [SuppressUnmanagedCodeSecurity]
        [ReliabilityContract(Consistency.WillNotCorruptState, Cer.MayFail)]
        [DllImport("user32.dll", CallingConvention = CallingConvention.Winapi)]
        internal static extern int ReleaseDC(IntPtr hWnd, IntPtr hDC);

        [DllImport("user32.dll", CallingConvention = CallingConvention.Winapi)]
        internal static extern IntPtr MonitorFromPoint(POINT pt, uint dwFlags);

        [DllImport("gdi32.dll", CallingConvention = CallingConvention.Winapi)]
        internal static extern int GetDeviceCaps(SafeDeviceContextHandle hdc, int nIndex);

        [DllImport("shcore.dll", CallingConvention = CallingConvention.Winapi)]
        internal static extern int GetDpiForMonitor(IntPtr hmonitor, int dpiType, out uint dpiX, out uint dpiY);

        /// <summary>
        /// Specifies the window station, desktop, standard handles, and appearance of the main window for a process at creation
        /// time.
        /// </summary>
        /// <remarks>
        /// See <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/ms686331.aspx">MSDN</a> for more details.
        /// </remarks>
        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        internal struct STARTUPINFO
        {
            public uint cb;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string lpReserved;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string lpDesktop;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string lpTitle;
            public uint dwX;
            public uint dwY;
            public uint dwXSize;
            public uint dwYSize;
            public uint dwXCountChars;
            public uint dwYCountChars;
            public uint dwFillAttribute;
            public uint dwFlags;
            public ushort wShowWindow;
            public ushort cbReserved2;
            public IntPtr lpReserved2;
            public IntPtr hStdInput;
            public IntPtr hStdOutput;
            public IntPtr hStdError;
        }

        /// <summary>
        /// Contains information about a newly created process and its primary thread. It is used with the
        /// <see cref="CreateProcess"/>, <c>CreateProcessAsUser</c>, <c>CreateProcessWithLogonW</c>, or
        /// <c>CreateProcessWithTokenW</c> function.
        /// </summary>
        /// <remarks>
        /// See <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/ms684873.aspx">MSDN</a> for more details.
        /// </remarks>
        [StructLayout(LayoutKind.Sequential)]
        internal struct PROCESS_INFORMATION
        {
            public IntPtr hProcess;
            public IntPtr hThread;
            public uint dwProcessId;
            public uint dwThreadId;
        }

        /// <summary>
        /// This structure is not documented on MSDN.
        /// </summary>
        /// <remarks>See winnt.h for more details.</remarks>
        [StructLayout(LayoutKind.Sequential)]
        internal struct IMAGE_DOS_HEADER
        {
            public ushort e_magic;
            public ushort e_cblp;
            public ushort e_cp;
            public ushort e_crlc;
            public ushort e_cparhdr;
            public ushort e_minalloc;
            public ushort e_maxalloc;
            public ushort e_ss;
            public ushort e_sp;
            public ushort e_csum;
            public ushort e_ip;
            public ushort e_cs;
            public ushort e_lfarlc;
            public ushort e_ovno;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
            public ushort[] e_res;
            public ushort e_oemid;
            public ushort e_oeminfo;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 10)]
            public ushort[] e_res2;
            public int e_lfanew;
        }

        /// <summary>
        /// Represents the PE header format.
        /// </summary>
        /// <remarks>
        /// See <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/ms680336.aspx">MSDN</a> for more details.
        /// </remarks>
        [StructLayout(LayoutKind.Sequential)]
        internal struct IMAGE_NT_HEADERS
        {
            public uint Signature;
            public IMAGE_FILE_HEADER FileHeader;
            public IMAGE_OPTIONAL_HEADER OptionalHeader;
        }

        /// <summary>
        /// Represents the COFF header format.
        /// </summary>
        /// <remarks>
        /// See <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/ms680313.aspx">MSDN</a> for more details.
        /// </remarks>
        [StructLayout(LayoutKind.Sequential)]
        internal struct IMAGE_FILE_HEADER
        {
            public ushort Machine;
            public ushort NumberOfSections;
            public uint TimeDateStamp;
            public uint PointerToSymbolTable;
            public uint NumberOfSymbols;
            public ushort SizeOfOptionalHeader;
            public ushort Characteristics;
        }

        /// <summary>
        /// Represents the optional header format.
        /// </summary>
        /// <remarks>
        /// See <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/ms680339.aspx">MSDN</a> for more details.
        /// </remarks>
        [StructLayout(LayoutKind.Sequential)]
        internal struct IMAGE_OPTIONAL_HEADER
        {
            public ushort Magic;
            public byte MajorLinkerVersion;
            public byte MinorLinkerVersion;
            public uint SizeOfCode;
            public uint SizeOfInitializedData;
            public uint SizeOfUninitializedData;
            public uint AddressOfEntryPoint;
            public uint BaseOfCode;
            public uint BaseOfData;
            public uint ImageBase;
            public uint SectionAlignment;
            public uint FileAlignment;
            public ushort MajorOperatingSystemVersion;
            public ushort MinorOperatingSystemVersion;
            public ushort MajorImageVersion;
            public ushort MinorImageVersion;
            public ushort MajorSubsystemVersion;
            public ushort MinorSubsystemVersion;
            public uint Win32VersionValue;
            public uint SizeOfImage;
            public uint SizeOfHeaders;
            public uint CheckSum;
            public ushort Subsystem;
            public ushort DllCharacteristics;
            public uint SizeOfStackReserve;
            public uint SizeOfStackCommit;
            public uint SizeOfHeapReserve;
            public uint SizeOfHeapCommit;
            public uint LoaderFlags;
            public uint NumberOfRvaAndSizes;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
            public IMAGE_DATA_DIRECTORY[] DataDirectory;
        }

        /// <summary>
        /// Represents the data directory.
        /// </summary>
        /// <remarks>
        /// See <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/ms680305.aspx">MSDN</a> for more details.
        /// </remarks>
        [StructLayout(LayoutKind.Sequential)]
        internal struct IMAGE_DATA_DIRECTORY
        {
            public uint VirtualAddress;
            public uint Size;
        }

        /// <summary>
        /// Represents the export directory. Refer to winnt.h for the definition of this structure.
        /// </summary>
        [StructLayout(LayoutKind.Sequential)]
        internal struct IMAGE_EXPORT_DIRECTORY
        {
            public uint Characteristics;
            public uint TimeDateStamp;
            public ushort MajorVersion;
            public ushort MinorVersion;
            public uint Name;
            public uint Base;
            public uint NumberOfFunctions;
            public uint NumberOfNames;
            public uint AddressOfFunctions;
            public uint AddressOfNames;
            public uint AddressOfNameOrdinals;
        }

        /// <summary>
        /// Contains processor-specific register data. The system uses CONTEXT structures to perform various internal
        /// operations. Refer to the header file winnt.h for definitions of this structure for each processor
        /// architecture.
        /// </summary>
        /// <remarks>
        /// See <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/ms679284.aspx">MSDN</a> for more details.
        /// </remarks>
        [StructLayout(LayoutKind.Sequential)]
        internal struct WOW64_CONTEXT
        {
            public uint ContextFlags;
            public uint Dr0;
            public uint Dr1;
            public uint Dr2;
            public uint Dr3;
            public uint Dr6;
            public uint Dr7;
            public WOW64_FLOATING_SAVE_AREA FloatSave;
            public uint SegGs;
            public uint SegFs;
            public uint SegEs;
            public uint SegDs;
            public uint Edi;
            public uint Esi;
            public uint Ebx;
            public uint Edx;
            public uint Ecx;
            public uint Eax;
            public uint Ebp;
            public uint Eip;
            public uint SegCs;
            public uint EFlags;
            public uint Esp;
            public uint SegSs;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 512)]
            public byte[] ExtendedRegisters;
        }

        /// <summary>
        /// This structure is not documented on MSDN.
        /// </summary>
        /// <remarks>See winnt.h for more details.</remarks>
        [StructLayout(LayoutKind.Sequential)]
        internal struct WOW64_FLOATING_SAVE_AREA
        {
            public uint ControlWord;
            public uint StatusWord;
            public uint TagWord;
            public uint ErrorOffset;
            public uint ErrorSelector;
            public uint DataOffset;
            public uint DataSelector;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 80)]
            public byte[] RegisterArea;
            public uint Cr0NpxState;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct LUID
        {
            public uint LowPart;
            public int HighPart;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct POINT
        {
            public int x;
            public int y;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct POINTL
        {
            public int x;
            public int y;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct DISPLAYCONFIG_RATIONAL
        {
            public uint Numerator;
            public uint Denominator;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct DISPLAYCONFIG_PATH_INFO
        {
            public DISPLAYCONFIG_PATH_SOURCE_INFO sourceInfo;
            public DISPLAYCONFIG_PATH_TARGET_INFO targetInfo;
            public uint flags;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct DISPLAYCONFIG_PATH_SOURCE_INFO
        {
            public LUID adapterId;
            public uint id;
            public uint modeInfoIdx;
            public uint statusFlags;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct DISPLAYCONFIG_PATH_TARGET_INFO
        {
            public LUID adapterId;
            public uint id;
            public uint modeInfoIdx;
            public uint outputTechnology;
            public uint rotation;
            public uint scaling;
            public DISPLAYCONFIG_RATIONAL refreshRate;
            public uint scanLineOrdering;
            public bool targetAvailable;
            public uint statusFlags;
        }

        [StructLayout(LayoutKind.Sequential, Size = 64)]
        internal struct DISPLAYCONFIG_MODE_INFO
        {
            public uint infoType;
            public uint id;
            public LUID adapterId;
            public DISPLAYCONFIG_SOURCE_MODE sourceMode;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct DISPLAYCONFIG_SOURCE_MODE
        {
            public uint width;
            public uint height;
            public uint pixelFormat;
            public POINTL position;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct DISPLAYCONFIG_DEVICE_INFO_HEADER
        {
            public uint type;
            public uint size;
            public LUID adapterId;
            public uint id;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        internal struct DISPLAYCONFIG_SOURCE_DEVICE_NAME
        {
            public DISPLAYCONFIG_DEVICE_INFO_HEADER header;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
            public string viewGdiDeviceName;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        internal struct DISPLAYCONFIG_TARGET_DEVICE_NAME
        {
            public DISPLAYCONFIG_DEVICE_INFO_HEADER header;
            public DISPLAYCONFIG_TARGET_DEVICE_NAME_FLAGS flags;
            public uint outputTechnology;
            public ushort edidManufactureId;
            public ushort edidProductCodeId;
            public uint connectorInstance;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 64)]
            public string monitorFriendlyDeviceName;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
            public string monitorDevicePath;
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct DISPLAYCONFIG_TARGET_DEVICE_NAME_FLAGS
        {
            public uint value;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        internal struct DISPLAY_DEVICE
        {
            public uint cb;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
            public string DeviceName;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
            public string DeviceString;
            public uint StateFlags;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
            public string DeviceID;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
            public string DeviceKey;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        internal struct DEVMODE
        {
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
            public string dmDeviceName;
            public ushort dmSpecVersion;
            public ushort dmDriverVersion;
            public ushort dmSize;
            public ushort dmDriverExtra;
            public uint dmFields;
            public POINTL dmPosition;
            public uint dmDisplayOrientation;
            public uint dmDisplayFixedOutput;
            public short dmColor;
            public short dmDuplex;
            public short dmYResolution;
            public short dmTTOption;
            public short dmCollate;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
            public string dmFormName;
            public ushort dmLogPixels;
            public uint dmBitsPerPel;
            public uint dmPelsWidth;
            public uint dmPelsHeight;
            public uint dmDisplayFlags;
            public uint dmDisplayFrequency;
            public uint dmICMMethod;
            public uint dmICMIntent;
            public uint dmMediaType;
            public uint dmDitherType;
            public uint dmReserved1;
            public uint dmReserved2;
            public uint dmPanningWidth;
            public uint dmPanningHeight;
        }
    }
}
