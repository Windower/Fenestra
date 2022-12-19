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

using System;

namespace Boiler
{
    public struct SteamGame : IGame, IEquatable<SteamGame>
    {
        public SteamGame(long appId, string name, string installDirectory)
        {
            AppId = appId;
            Name = name;
            InstallDirectory = installDirectory;
        }

        public bool IsSteamGame => true;

        public long AppId { get; }

        public string Name { get; }

        public string InstallDirectory { get; }

        public static bool operator ==(SteamGame left, SteamGame right) => left.AppId == right.AppId;

        public static bool operator !=(SteamGame left, SteamGame right) => !(left == right);

        public static bool operator ==(SteamGame left, IGame right) => left.AppId == right?.AppId;

        public static bool operator !=(SteamGame left, IGame right) => !(left == right);

        public static bool operator ==(IGame left, SteamGame right) => left?.AppId == right.AppId;

        public static bool operator !=(IGame left, SteamGame right) => !(left == right);

        public bool Equals(SteamGame other) => AppId == other.AppId;

        public bool Equals(IGame other) => AppId == other?.AppId;

        public override bool Equals(object obj)
        {
            if (obj is SteamGame temp1)
            {
                return Equals(temp1);
            }
            else if (obj is IGame temp2)
            {
                return Equals(temp2);
            }

            return false;
        }

        public override int GetHashCode() => AppId.GetHashCode();
    }
}
