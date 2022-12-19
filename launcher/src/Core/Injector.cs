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
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.IO;
    using System.Linq;
    using System.Runtime.InteropServices;
    using System.Text;
    using System.Text.RegularExpressions;
    using System.Threading.Tasks;

    internal class Injector : IDisposable
    {
        /// <summary>
        /// The x86 op-code for an unconditional relative jump to itself.
        /// <code>jmp -2</code>
        /// </summary>
        private static readonly byte[] jumpToSelf = { 0xEB, 0xFE };

        /// <summary>
        /// The disposed flag
        /// </summary>
        private bool disposed = false;

        /// <summary>
        /// The process
        /// </summary>
        private Process process;

        /// <summary>
        /// The process handle
        /// </summary>
        private SafeWaitHandle processHandle;

        /// <summary>
        /// The thread handle
        /// </summary>
        private SafeWaitHandle threadHandle;

        /// <summary>
        /// The executable path
        /// </summary>
        private string path;

        /// <summary>
        /// The process initialization state
        /// </summary>
        private bool initialized;

        /// <summary>
        /// Initializes a new instance of the <see cref="Injector"/> class by creating and attaching to a new suspended
        /// process. The process will be resumed when this instance is disposed.
        /// </summary>
        /// <param name="path">The path to the executable file.</param>
        /// <param name="args">Arguments to pass to the executable.</param>
        public Injector(string path, params string[] args) :
            this(path, EscapeArguments(args))
        { }

        /// <summary>
        /// Initializes a new instance of the <see cref="Injector"/> class by creating and attaching to a new suspended
        /// process. The process will be resumed when this instance is disposed.
        /// </summary>
        /// <param name="path">The path to the executable file.</param>
        /// <param name="argString">Argument string to pass to the executable.</param>
        public Injector(string path, string argString)
        {
            path = Path.GetFullPath(path ?? throw new ArgumentNullException(nameof(path)));
            argString = string.Join(" ", EscapeArgument(path), argString ?? string.Empty).Trim();

            this.path = path;

            var directory = Path.GetDirectoryName(path);
            var info = default(NativeMethods.PROCESS_INFORMATION);
            var startup = default(NativeMethods.STARTUPINFO);
            startup.cb = (uint)Marshal.SizeOf(typeof(NativeMethods.STARTUPINFO));

            try
            {
                if (!NativeMethods.CreateProcess(path, argString, IntPtr.Zero, IntPtr.Zero, false, NativeMethods.CREATE_SUSPENDED,
                    IntPtr.Zero, directory, ref startup, out info))
                {
                    throw new Win32Exception();
                }
            }
            finally
            {
                try
                {
                    processHandle = new SafeWaitHandle(info.hProcess, true);
                }
                finally
                {
                    threadHandle = new SafeWaitHandle(info.hThread, true);
                }
            }

            process = Process.GetProcessById((int)info.dwProcessId);
            initialized = false;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="Injector"/> class by wrapping an existing
        /// <see cref="System.Diagnostics.Process"/>.
        /// </summary>
        /// <param name="process">The <see cref="System.Diagnostics.Process"/> to wrap.</param>
        public Injector(Process process)
        {
            processHandle = new SafeWaitHandle(process.Handle, false);
            threadHandle = new SafeWaitHandle(IntPtr.Zero, true);

            this.process = process;
            initialized = true;
        }

        /// <summary>
        /// Gets a copy of the <see cref="System.Diagnostics.Process"/> associated with this <see cref="Injector"/>.
        /// </summary>
        public Process Process => Process.GetProcessById(process.Id);

        /// <summary>
        /// Injects the specified DLL.
        /// </summary>
        /// <param name="dllPath">The path to the DLL.</param>
        /// <exception cref="ArgumentNullException"><paramref name="dllPath"/> is <value>null</value>.</exception>
        /// <exception cref="ObjectDisposedException">The instance has been disposed.</exception>
        /// <exception cref="InvalidOperationException">The LoadLibrary function could not be found</exception>
        /// <exception cref="Win32Exception">The DLL could not be injected.</exception>
        public async Task Inject(string dllPath)
        {
            if (dllPath == null)
            {
                throw new ArgumentNullException(nameof(dllPath));
            }

            if (disposed)
            {
                throw new ObjectDisposedException(null);
            }

            if (!initialized)
            {
                await RunProcessInitialization();
                initialized = true;
            }

            var function = GetRemoteFunctionAddress("kernel32.dll", "LoadLibraryW");
            if (function == IntPtr.Zero)
            {
                throw new InvalidOperationException("Unable to locate LoadLibrary function.");
            }

            var buffer = Encoding.Unicode.GetBytes(dllPath + '\0');
            using (var remoteBuffer = new SafeProcessMemoryHandle(processHandle, (uint)buffer.Length))
            {
                Write(processHandle, remoteBuffer, buffer);

                // This is an unusual way to return a SafeWaitHandle, but
                // it is required to work around an issue in Mono's
                // implementation.  I believe this can technically leak the
                // thread handle, but any situation which might cause this
                // to happen is going to bring down the entire process
                // anyway, and then the OS will clean up after us.
                var remoteThreadHandle = NativeMethods.CreateRemoteThread(processHandle, IntPtr.Zero, UIntPtr.Zero, function,
                    remoteBuffer, 0, IntPtr.Zero);
                using (var remoteThread = new SafeWaitHandle(remoteThreadHandle, true))
                {
                    if (remoteThread.IsInvalid)
                    {
                        throw new Win32Exception();
                    }

                    if (NativeMethods.WaitForSingleObject(remoteThread, NativeMethods.INFINITE) != NativeMethods.WAIT_OBJECT_0)
                    {
                        throw new Win32Exception();
                    }

                    if (!NativeMethods.GetExitCodeThread(remoteThread, out var hmodule))
                    {
                        throw new Win32Exception();
                    }

                    if (hmodule == 0)
                    {
                        throw new Win32Exception(6 /* ERROR_INVALID_HANDLE */, "Failed to inject " + Path.GetFileName(dllPath));
                    }
                }
            }
        }

        /// <summary>
        /// Performs application-defined tasks associated with freeing, releasing, or resetting unmanaged resources.
        /// </summary>
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Releases unmanaged and, optionally, managed resources.
        /// </summary>
        /// <param name="disposing">
        ///   <c>true</c> to release both managed and unmanaged resources; <c>false</c> to release only unmanaged resources.
        /// </param>
        protected virtual void Dispose(bool disposing)
        {
            if (!disposed && disposing)
            {
                try
                {
                    Resume();
                }
                catch
                {
                    if (!process.HasExited)
                    {
                        process.Kill();
                    }

                    throw;
                }
                finally
                {
                    threadHandle.Dispose();
                    processHandle.Dispose();
                    process.Dispose();
                }
            }

            disposed = true;
        }

        /// <summary>
        /// Runs the remote process's initialization.
        /// </summary>
        private async Task RunProcessInitialization()
        {
            var entrypoint = await GetEntrypoint();
            var access = SetMemoryAccess(entrypoint, jumpToSelf.Length, NativeMethods.PAGE_EXECUTE_READWRITE);
            try
            {
                var original = Read(processHandle, entrypoint, (uint)jumpToSelf.Length);

                Write(processHandle, entrypoint, jumpToSelf);
                try
                {
                    Resume();

                    try
                    {
                        while (GetInstructionPointer() != entrypoint)
                        {
                            await Task.Delay(10);
                        }
                    }
                    finally
                    {
                        Suspend();
                    }
                }
                finally
                {
                    try
                    {
                        Write(processHandle, entrypoint, original);
                    }
                    catch
                    {
                        process.Kill();
                        Resume();
                    }
                }
            }
            finally
            {
                SetMemoryAccess(entrypoint, jumpToSelf.Length, access);
            }
        }

        /// <summary>
        /// Gets a pointer to the process's entrypoint.
        /// </summary>
        /// <returns>A pointer to the process's entrypoint.</returns>
        /// <exception cref="System.ComponentModel.Win32Exception">Could not locate the process's main module.</exception>
        private async Task<IntPtr> GetEntrypoint()
        {
            using (var stream = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.ReadWrite))
            {
                var dosheader = await ReadAsync<NativeMethods.IMAGE_DOS_HEADER>(stream);
                stream.Seek(dosheader.e_lfanew, SeekOrigin.Begin);
                var ntheaders = await ReadAsync<NativeMethods.IMAGE_NT_HEADERS>(stream);

                return new IntPtr(ntheaders.OptionalHeader.ImageBase + ntheaders.OptionalHeader.AddressOfEntryPoint);
            }
        }

        /// <summary>
        /// Suspends the main thread.
        /// </summary>
        /// <exception cref="System.ComponentModel.Win32Exception">The process could not be suspended.</exception>
        private void Suspend()
        {
            if (!threadHandle.IsInvalid && NativeMethods.SuspendThread(threadHandle) == 0xFFFFFFFF)
            {
                throw new Win32Exception();
            }
        }

        /// <summary>
        /// Resumes the main thread.
        /// </summary>
        /// <exception cref="System.ComponentModel.Win32Exception">The process could not be resumed.</exception>
        private void Resume()
        {
            if (!threadHandle.IsInvalid && NativeMethods.ResumeThread(threadHandle) == 0xFFFFFFFF)
            {
                throw new Win32Exception();
            }
        }

        /// <summary>
        /// Sets access rights for the given memory region within the process.
        /// </summary>
        /// <param name="address">The start address of the memory region.</param>
        /// <param name="length">The length of the memory region.</param>
        /// <param name="access">The access.</param>
        /// <returns>The region's previous access rights.</returns>
        /// <exception cref="System.ComponentModel.Win32Exception">The access for the region could not be modified.</exception>
        private uint SetMemoryAccess(IntPtr address, int length, uint access)
        {
            if (!NativeMethods.VirtualProtectEx(processHandle, address, (UIntPtr)length, access, out access))
            {
                throw new Win32Exception();
            }

            return access;
        }

        /// <summary>
        /// Gets the main thread's instruction pointer.
        /// </summary>
        /// <returns>The thread's instruction pointer.</returns>
        private IntPtr GetInstructionPointer()
        {
            var ptr = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(NativeMethods.WOW64_CONTEXT)) + 0xF);
            try
            {
                if (ptr != IntPtr.Zero)
                {
                    Suspend();

                    var aligned = new IntPtr((ptr.ToInt64() + 0xF) & ~0xF);
                    var context = default(NativeMethods.WOW64_CONTEXT);
                    context.ContextFlags = NativeMethods.CONTEXT_CONTROL;
                    Marshal.StructureToPtr(context, aligned, false);
                    NativeMethods.Wow64GetThreadContext(threadHandle, aligned);
                    context = (NativeMethods.WOW64_CONTEXT)Marshal.PtrToStructure(aligned, typeof(NativeMethods.WOW64_CONTEXT));
                    return new IntPtr((int)context.Eip);
                }
            }
            finally
            {
                if (ptr != IntPtr.Zero)
                {
                    Marshal.FreeHGlobal(ptr);
                }

                Resume();
            }

            return IntPtr.Zero;
        }

        /// <summary>
        /// Gets the address of an exported function from the remote process.
        /// </summary>
        /// <param name="module">Name of the module exporting the function.</param>
        /// <param name="function">Name of the function.</param>
        /// <returns>The function address.</returns>
        private IntPtr GetRemoteFunctionAddress(string module, string function)
        {
            var remoteBase = GetModule(module);

            var dosheader = Read<NativeMethods.IMAGE_DOS_HEADER>(processHandle, remoteBase);
            var ntheaders = Read<NativeMethods.IMAGE_NT_HEADERS>(processHandle, remoteBase + dosheader.e_lfanew);

            var exportsOffset = ntheaders.OptionalHeader.DataDirectory[NativeMethods.IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
            var exports = Read<NativeMethods.IMAGE_EXPORT_DIRECTORY>(processHandle, remoteBase + (int)exportsOffset);

            var nameLength = (uint)Encoding.ASCII.GetByteCount(function) + 1;

            for (var i = 0; i < exports.NumberOfNames; i++)
            {
                var nameOffset = Read<uint>(processHandle, remoteBase + (int)exports.AddressOfNames + 4 * i);
                var buffer = Read(processHandle, remoteBase + (int)nameOffset, nameLength);
                if (Array.IndexOf(buffer, (byte)0) == function.Length)
                {
                    var name = Encoding.ASCII.GetString(buffer, 0, function.Length);
                    if (name == function)
                    {
                        var ordinal = Read<ushort>(processHandle, remoteBase + (int)exports.AddressOfNameOrdinals + 2 * i);
                        var functionOffset = Read<uint>(processHandle, remoteBase + (int)exports.AddressOfFunctions + 4 * ordinal);
                        return remoteBase + (int)functionOffset;
                    }
                }
            }

            return IntPtr.Zero;
        }

        private IntPtr GetModule(string name)
        {
            var count = 0;
            IntPtr[] modules;
            do
            {
                modules = new IntPtr[count];
                uint required;
                try
                {
                    if (!NativeMethods.EnumProcessModulesEx(processHandle, modules, (uint)(modules.Length * IntPtr.Size),
                        out required, NativeMethods.LIST_MODULES_32BIT))
                    {
                        throw new Win32Exception();
                    }
                }
                catch (EntryPointNotFoundException)
                {
                    if (!NativeMethods.EnumProcessModules(processHandle, modules, (uint)(modules.Length * IntPtr.Size),
                        out required))
                    {
                        throw new Win32Exception();
                    }
                }

                count = (int)required / IntPtr.Size;
            }
            while (count > modules.Length);

            for (var i = 0; i < count; i++)
            {
                var buffer = new StringBuilder(260);
                if (NativeMethods.GetModuleFileNameEx(processHandle, modules[i], buffer, (uint)buffer.Capacity) != 0)
                {
                    if (string.Equals(Path.GetFileName(buffer.ToString()), name, StringComparison.OrdinalIgnoreCase))
                    {
                        return modules[i];
                    }
                }
            }

            return IntPtr.Zero;
        }

        /// <summary>
        /// Escapes strings for passing as arguments on the command line.
        /// </summary>
        /// <param name="argument">The string to escape.</param>
        private static string EscapeArguments(IEnumerable<string> args)
        {
            var quotedArgs =
                from a in args ?? new string[0]
                where !string.IsNullOrEmpty(a)
                select EscapeArgument(a);
            return string.Join(" ", quotedArgs);
        }

        /// <summary>
        /// Escapes a string for passing as an argument on the command line.
        /// </summary>
        /// <param name="argument">The string to escape.</param>
        private static string EscapeArgument(string argument)
        {
            if (argument.Any(c => "\"\t ".Contains(c)))
            {
                return FormattableString.Invariant($"\"{Regex.Replace(argument, @"(\\*)(\\$|\"")", @"$1$1\$2")}\"");
            }

            return argument;
        }

        /// <summary>
        /// Reads a structure from a stream.
        /// </summary>
        /// <typeparam name="T">The <see cref="System.Type"/> to read.</typeparam>
        /// <param name="stream">The stream to read from.</param>
        private static async Task<T> ReadAsync<T>(Stream stream)
        {
            var buffer = new byte[Marshal.SizeOf(typeof(T))];
            var count = 0;
            do
            {
                count += await stream.ReadAsync(buffer, count, buffer.Length);
            }
            while (count < buffer.Length);

            var handle = GCHandle.Alloc(buffer, GCHandleType.Pinned);
            try
            {
                return (T)Marshal.PtrToStructure(handle.AddrOfPinnedObject(), typeof(T));
            }
            finally
            {
                handle.Free();
            }
        }

        /// <summary>
        /// Reads from the specified address in the process.
        /// </summary>
        /// <param name="process">The process's handle.</param>
        /// <param name="address">The address.</param>
        /// <returns>The read data.</returns>
        /// <exception cref="System.ComponentModel.Win32Exception">The specified address could not be read.</exception>
        private static T Read<T>(SafeWaitHandle process, IntPtr address)
        {
            var size = Marshal.SizeOf(typeof(T));
            var buffer = Marshal.AllocHGlobal(size);
            try
            {
                if (buffer != IntPtr.Zero && !NativeMethods.ReadProcessMemory(process, address, buffer, (UIntPtr)size, IntPtr.Zero))
                {
                    throw new Win32Exception();
                }

                return (T)Marshal.PtrToStructure(buffer, typeof(T));
            }
            finally
            {
                if (buffer != IntPtr.Zero)
                {
                    Marshal.FreeHGlobal(buffer);
                }
            }
        }

        /// <summary>
        /// Reads from the specified address in the process.
        /// </summary>
        /// <param name="process">The process's handle.</param>
        /// <param name="address">The address.</param>
        /// <param name="length">The number of bytes to read.</param>
        /// <returns>The read data.</returns>
        /// <exception cref="System.ComponentModel.Win32Exception">The specified address could not be read.</exception>
        private static byte[] Read(SafeWaitHandle process, IntPtr address, uint length)
        {
            var buffer = new byte[length];

            if (!NativeMethods.ReadProcessMemory(process, address, buffer, (UIntPtr)buffer.Length, IntPtr.Zero))
            {
                throw new Win32Exception();
            }

            return buffer;
        }

        /// <summary>
        /// Writes to the specified address in the process.
        /// </summary>
        /// <param name="process">The process's handle.</param>
        /// <param name="address">The address.</param>
        /// <param name="buffer">The data to write.</param>
        /// <exception cref="System.ComponentModel.Win32Exception">
        ///   The data could not be written to the specified address.
        /// </exception>
        private static void Write(SafeWaitHandle process, IntPtr address, byte[] buffer)
        {
            if (!NativeMethods.WriteProcessMemory(process, address, buffer, (UIntPtr)buffer.Length, IntPtr.Zero))
            {
                throw new Win32Exception();
            }
        }

        /// <summary>
        /// Writes to the specified address in the process.
        /// </summary>
        /// <param name="process">The process's handle.</param>
        /// <param name="address">The address.</param>
        /// <param name="buffer">The data to write.</param>
        /// <exception cref="System.ComponentModel.Win32Exception">
        ///   The data could not be written to the specified address.
        /// </exception>
        private static void Write(SafeWaitHandle process, SafeProcessMemoryHandle address, byte[] buffer)
        {
            if (!NativeMethods.WriteProcessMemory(process, address, buffer, (UIntPtr)buffer.Length, IntPtr.Zero))
            {
                throw new Win32Exception();
            }
        }
    }
}
