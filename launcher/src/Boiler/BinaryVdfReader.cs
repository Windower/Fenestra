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
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Diagnostics.CodeAnalysis;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Text;

    [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "Vdf")]
    public static class BinaryVdfReader
    {
        private class VdfString : IVdfValue
        {
            public VdfString(string value) => Value = value;

            public string Value { get; }

            public bool HasSubValues { get; } = false;

            public int Count { get; } = 0;

            public IVdfValue this[string key] => null;

            public IEnumerable<string> Keys
            {
                get { yield break; }
            }

            public IEnumerable<IVdfValue> Values
            {
                get { yield break; }
            }

            public bool ContainsKey(string key) => false;

            public bool TryGetValue(string key, out IVdfValue value)
            {
                value = null;
                return false;
            }

            public IEnumerator<KeyValuePair<string, IVdfValue>> GetEnumerator() =>
                Enumerable.Empty<KeyValuePair<string, IVdfValue>>().GetEnumerator();

            IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
        }

        private class VdfDictionary : Dictionary<string, IVdfValue>, IVdfValue
        {
            public VdfDictionary(Stream input, Encoding encoding)
            {
                while (true)
                {
                    switch (input.ReadByte())
                    {
                        default: throw new InvalidOperationException();
                        case -1: throw new EndOfStreamException();
                        case 8: return;
                        case 0:
                            Add(ReadString(input, encoding), new VdfDictionary(input, encoding));
                            break;
                        case 1:
                            Add(ReadString(input, encoding), new VdfString(ReadString(input, encoding)));
                            break;
                        case 2:
                            {
                                var key = ReadString(input, encoding);
                                var buffer = new byte[4];
                                input.Read(buffer, 0, buffer.Length);
                                var value = BitConverter.ToInt32(buffer, 0);
                                Add(key, new VdfString(value.ToString(CultureInfo.InvariantCulture)));
                            }
                            break;
                    }
                }
            }

            public string Value { get; } = null;

            public bool HasSubValues { get; } = true;

            private static string ReadString(Stream input, Encoding encoding)
            {
                var buffer = new List<byte>();
                int read;
                while ((read = input.ReadByte()) != -1 && read != 0)
                {
                    buffer.Add((byte)read);
                }
                return encoding.GetString(buffer.ToArray(), 0, buffer.Count);
            }
        }

        public static IVdfValue Load(Stream input) => Load(input, Encoding.UTF8);

        public static IVdfValue Load(Stream input, Encoding encoding) => new VdfDictionary(input, encoding);
    }
}
