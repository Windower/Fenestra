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

    [Verb("save")]
    public class SaveOptions : ProfileOptions
    {
        Maybe<string> baseProfileName;

        [Value(0, MetaName = "name", Required = true)]
        public string ProfileName { get; set; }

        [Option("overwrite")]
        public bool Overwrite { get; set; }

        [Option("based-on")]
        public string BaseProfileName
        {
            get => baseProfileName.Default(null);
            set => baseProfileName = string.IsNullOrWhiteSpace(value) ? new Maybe<string>() : value.Trim();
        }

        [SuppressMessage("Microsoft.Design", "CA1024")]
        public Profile GetProfile() => GetProfile(baseProfileName.Bind(x => Launcher.ProfileManager[x]).Default(Profile.Default))
                .With(Name: ProfileName);
    }
}
