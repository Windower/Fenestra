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
    using System.Collections.Immutable;
    using System.Diagnostics.CodeAnalysis;

    public struct DisplayDevice : IEquatable<DisplayDevice>
    {
        public DisplayDevice(string deviceName, string friendlyName, bool isPrimary, Dimension currentResolution,
            Point currentPosition, int currentDpi, IImmutableList<Dimension> availableResolutions)
        {
            DeviceName = deviceName;
            FriendlyName = friendlyName;
            IsPrimary = isPrimary;
            CurrentResolution = currentResolution;
            CurrentPosition = currentPosition;
            CurrentDpi = currentDpi;
            AvailableResolutions = availableResolutions ?? ImmutableArray<Dimension>.Empty.Add(CurrentResolution);
        }

        public string DeviceName { get; }
        public string FriendlyName { get; }
        public bool IsPrimary { get; }
        public Dimension CurrentResolution { get; }
        public Point CurrentPosition { get; }
        public IImmutableList<Dimension> AvailableResolutions { get; }
        public int CurrentDpi { get; }

        [SuppressMessage("Microsoft.Design", "CA1006")]
        [SuppressMessage("Microsoft.Design", "CA1026")]
        [SuppressMessage("Microsoft.Naming", "CA1709")]
        public DisplayDevice With(
            Maybe<string> DeviceName = new Maybe<string>(),
            Maybe<string> FriendlyName = new Maybe<string>(),
            Maybe<bool> IsPrimary = new Maybe<bool>(),
            Maybe<Dimension> CurrentResolution = new Maybe<Dimension>(),
            Maybe<Point> CurrentPosition = new Maybe<Point>(),
            Maybe<int> CurrentDpi = new Maybe<int>(),
            Maybe<IImmutableList<Dimension>> AvailableResolutions = new Maybe<IImmutableList<Dimension>>())
        {
            if (DeviceName != this.DeviceName || FriendlyName != this.FriendlyName || IsPrimary != this.IsPrimary ||
                CurrentResolution != this.CurrentResolution || CurrentPosition != this.CurrentPosition ||
                CurrentDpi != this.CurrentDpi || AvailableResolutions != this.AvailableResolutions)
            {
                return new DisplayDevice(
                    DeviceName.Default(this.DeviceName),
                    FriendlyName.Default(this.FriendlyName),
                    IsPrimary.Default(this.IsPrimary),
                    CurrentResolution.Default(this.CurrentResolution),
                    CurrentPosition.Default(this.CurrentPosition),
                    CurrentDpi.Default(this.CurrentDpi),
                    AvailableResolutions.Default(this.AvailableResolutions));
            }

            return this;
        }

        public static bool operator ==(DisplayDevice left, DisplayDevice right) => left.DeviceName == right.DeviceName;

        public static bool operator !=(DisplayDevice left, DisplayDevice right) => !(left == right);

        public bool Equals(DisplayDevice other) => DeviceName == other.DeviceName;

        public override bool Equals(object obj) => obj is DisplayDevice other && Equals(other);

        public override int GetHashCode() => (DeviceName ?? string.Empty).GetHashCode();
    }
}
