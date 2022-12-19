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
    using System.IO;
    using System.Linq;
    using System.Text;

    [SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "Vdf")]
    public static class TextVdfReader
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
            public VdfDictionary(IEnumerator<Token> tokens)
            {
                while (tokens.MoveNext())
                {
                    var t = tokens.Current;
                    if (t.IsString)
                    {
                        var key = t.Value;
                        if (tokens.MoveNext())
                        {
                            t = tokens.Current;
                            if (t.IsString)
                            {
                                Add(key, new VdfString(t.Value));
                                continue;
                            }
                            else if (t.Value == "{")
                            {
                                Add(key, new VdfDictionary(tokens));
                                continue;
                            }
                        }
                    }
                    else if (t.Value == "}")
                    {
                        break;
                    }
                    throw new InvalidOperationException();
                }
            }

            public string Value { get; } = null;

            public bool HasSubValues { get; } = true;
        }

        public static IVdfValue Load(Stream input) => Load(input, Encoding.UTF8);

        public static IVdfValue Load(Stream input, Encoding encoding) => Load(new StreamReader(input, encoding));

        public static IVdfValue Load(StreamReader input) => new VdfDictionary(Tokens(input).GetEnumerator());

        private struct Token
        {
            public Token(string value, bool isString = true)
            {
                IsString = isString;
                Value = value;
            }

            public bool IsString { get; }
            public string Value { get; }
        }

        private static IEnumerable<Token> Tokens(StreamReader input)
        {
            var token = new StringBuilder();

            var state = 0;
            int read;
            while ((read = input.Read()) != -1)
            {
                var c = (char)read;
                if (c == '\r')
                {
                    continue;
                }
                switch(state)
                {
                    case 0:
                        switch(c)
                        {
                            case '\\': state = 2; break;
                            case '"': state = 3; break;
                            case '[': state = 5; break;
                            case '/': state = 6; break;
                            default:
                                if (!char.IsWhiteSpace(c))
                                {
                                    state = 1;
                                }
                                break;
                            case '{':
                            case '}':
                                yield return new Token(c.ToString(), false);
                                token.Clear();
                                break;
                        }
                        break;
                    case 1:
                        switch (c)
                        {
                            case '\\': state = 2; break;
                            default:
                                if (char.IsWhiteSpace(c))
                                {
                                    state = 0;
                                    yield return new Token(token.ToString());
                                    token.Clear();
                                }
                                else
                                {
                                    token.Append(c);
                                }
                                break;
                            case '{':
                            case '}':
                                state = 0;
                                yield return new Token(token.ToString());
                                yield return new Token(c.ToString(), false);
                                token.Clear();
                                break;
                            case '/':
                                state = 5;
                                yield return new Token(token.ToString());
                                token.Clear();
                                break;
                            case '[':
                                state = 6;
                                yield return new Token(token.ToString());
                                token.Clear();
                                break;
                        }
                        break;
                    case 2:
                        switch(c)
                        {
                            default: throw new InvalidOperationException();
                            case '\\': token.Append('\\'); break;
                            case 'n': token.Append('\n'); break;
                            case 't': token.Append('\t'); break;
                            case '"': token.Append('"'); break;
                        }
                        state = 1;
                        break;
                    case 3:
                        switch (c)
                        {
                            default: token.Append(c); break;
                            case '\\': state = 4; break;
                            case '\n': throw new InvalidOperationException();
                            case '"':
                                state = 0;
                                yield return new Token(token.ToString());
                                token.Clear();
                                break;
                        }
                        break;
                    case 4:
                        switch (c)
                        {
                            default: throw new InvalidOperationException();
                            case '\\': token.Append('\\'); break;
                            case 'n': token.Append('\n'); break;
                            case 't': token.Append('\t'); break;
                            case '"': token.Append('"'); break;
                        }
                        state = 3;
                        break;
                    case 5:
                        switch (c)
                        {
                            case ']': state = 0; break;
                            case '/':
                            case '\n': throw new InvalidOperationException();
                        }
                        break;
                    case 6:
                        switch (c)
                        {
                            case '\n': state = 0; break;
                        }
                        break;
                }
            }
        }
    }
}
