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
    using System.Diagnostics.CodeAnalysis;
    using System.IO;
    using System.Threading.Tasks;

    internal static class Utilities
    {
        [SuppressMessage("Microsoft.Reliability", "CA2000")]
        public static FileStream CreateTimestampedFileStream(string directory, string extension) =>
            CreateTimestampedFileStreamAsync(directory, extension).GetAwaiter().GetResult();

        public static async Task<FileStream> CreateTimestampedFileStreamAsync(string directory, string extension)
        {
            Directory.CreateDirectory(directory);
            while (true)
            {
                var filename = Path.ChangeExtension(DateTime.UtcNow.ToString("yyyy-MM-ddTHH-mm-ss-fffffffK"), extension);
                var path = Path.Combine(directory, filename);
                try
                {
                    return new FileStream(path, FileMode.CreateNew, FileAccess.Write, FileShare.Read | FileShare.Delete);
                }
                catch (IOException) when (File.Exists(path)) { }
                await Task.Yield();
            }
        }
    }
}
