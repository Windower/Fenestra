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
    using System.Diagnostics.CodeAnalysis;
    using System.Globalization;
    using System.IO;
    using System.IO.MemoryMappedFiles;
    using System.Linq;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Xml.Linq;

    using static System.FormattableString;

    /// <summary>
    /// Represents a settings communications channel with an external process.
    /// </summary>
    public class SettingsChannel : IDisposable
    {
        /// <summary>
        /// The disposed flag
        /// </summary>
        private bool disposed = false;

        /// <summary>
        /// The shared memory file handle
        /// </summary>
        private MemoryMappedFile file;

        /// <summary>
        /// The completion flag event
        /// </summary>
        private EventWaitHandle flag;

        /// <summary>
        /// Initializes a new instance of the <see cref="SettingsChannel"/> class.
        /// </summary>
        [SuppressMessage("Microsoft.Design", "CA1006")]
        public SettingsChannel(Process process, IEnumerable<KeyValuePair<string, object>> settings)
        {
            if (process == null)
            {
                throw new ArgumentNullException(nameof(process));
            }

            if (settings == null)
            {
                throw new ArgumentNullException(nameof(settings));
            }

            var dataName = Invariant($"Windower.Settings[{process.Id:X8}].Data");
            var flagName = Invariant($"Windower.Settings[{process.Id:X8}].Flag");

            var document = new XDocument(
                new XElement("settings",
                    from pair in settings
                    select new XElement(pair.Key, Unwrap(pair.Value))));

            using (var buffer = new MemoryStream())
            {
                document.Save(buffer, SaveOptions.DisableFormatting);

                file = MemoryMappedFile.CreateNew(dataName, buffer.Length + sizeof(long));
                using (var stream = file.CreateViewStream(0, 0))
                {
                    stream.Write(BitConverter.GetBytes(buffer.Length), 0, sizeof(long));
                    buffer.WriteTo(stream);
                }
            }

            flag = new EventWaitHandle(false, EventResetMode.ManualReset, flagName);
        }

        /// <summary>
        /// Creates an asynchronous task that completes when the external process has signaled that it is finished loading
        /// settings.
        /// </summary>
        public Task FinishAsync(CancellationToken token) =>
            Task.Run(() =>
            {
                do
                {
                    token.ThrowIfCancellationRequested();
                }
                while (!flag.WaitOne(100));
            }, token);

        /// <summary>
        /// Performs application-defined tasks associated with freeing, releasing, or resetting unmanaged resources.
        /// </summary>
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Releases unmanaged and - optionally - managed resources.
        /// </summary>
        /// <param name="disposing">
        /// <value>true</value> to release both managed and unmanaged resources; <value>false</value> to release only
        /// unmanaged resources.
        /// </param>
        protected virtual void Dispose(bool disposing)
        {
            if (!disposed && disposing)
            {
                file.Dispose();
                flag.Dispose();
            }

            disposed = true;
        }

        private static object Unwrap(object value)
        {
            if (value is Enum e)
            {
                return Convert.ChangeType(e, Enum.GetUnderlyingType(e.GetType()), CultureInfo.InvariantCulture);
            }

            return value;
        }
    }
}
