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
    using System.Diagnostics.CodeAnalysis;

    [Serializable]
    public struct Dimension : IEquatable<Dimension>
    {
        public Dimension(int width, int height)
        {
            Width = width;
            Height = height;
        }

        public int Width { get; }

        public int Height { get; }

        [SuppressMessage("Microsoft.Design", "CA1026")]
        [SuppressMessage("Microsoft.Naming", "CA1709")]
        public Dimension With(
            Maybe<int> Width = new Maybe<int>(),
            Maybe<int> Height = new Maybe<int>())
        {
            if (Width != this.Width || Height != this.Height)
            {
                return new Dimension(
                    Width.Default(this.Width),
                    Height.Default(this.Height));
            }

            return this;
        }

        public static bool operator ==(Dimension left, Dimension right) =>
            left.Width == right.Width && left.Height == right.Height;

        public static bool operator !=(Dimension left, Dimension right) => !(left == right);

        public bool Equals(Dimension other) => this == other;

        public override bool Equals(object obj) => obj is Dimension other && Equals(other);

        public override int GetHashCode() => Width.GetHashCode() * 17 + Height.GetHashCode();
    }
}
