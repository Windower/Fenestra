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
    using CommandLine;
    using Core;
    using System.Diagnostics.CodeAnalysis;

    [Verb("launch")]
    public class LaunchOptions : ProfileOptions
    {
        Maybe<string> baseProfileName;

        [Option("no-gui")]
        public bool NoGui { get; set; }

        [Value(0, MetaName = "profile")]
        public string BaseProfileName
        {
            get => baseProfileName.Default(null);
            set => baseProfileName = string.IsNullOrWhiteSpace(value) ? new Maybe<string>() : value.Trim();
        }

        [SuppressMessage("Microsoft.Design", "CA1024")]
        public Profile GetProfile() =>
            GetProfile(baseProfileName.Bind(x =>
                {
                    if (NoGui)
                    {
                        return Launcher.ProfileManager[x];
                    }

                    return Launcher.ProfileManager.TryGetValue(x, out var profile) ? profile : new Maybe<Profile>();
                }).Default());
    }
}
