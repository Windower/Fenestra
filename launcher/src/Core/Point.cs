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

    public struct Point : IEquatable<Point>
    {
        [SuppressMessage("Microsoft.Naming", "CA1704")]
        public Point(int x, int y)
        {
            X = x;
            Y = y;
        }

        [SuppressMessage("Microsoft.Naming", "CA1704")]
        public int X { get; }

        [SuppressMessage("Microsoft.Naming", "CA1704")]
        public int Y { get; }

        [SuppressMessage("Microsoft.Design", "CA1026")]
        [SuppressMessage("Microsoft.Naming", "CA1704")]
        [SuppressMessage("Microsoft.Naming", "CA1709")]
        public Point With(
            Maybe<int> X = new Maybe<int>(),
            Maybe<int> Y = new Maybe<int>())
        {
            if (X != this.X || Y != this.Y)
            {
                return new Point(
                    X.Default(this.X),
                    Y.Default(this.Y));
            }

            return this;
        }

        public static bool operator ==(Point left, Point right) => left.X == right.X && left.Y == right.Y;

        public static bool operator !=(Point left, Point right) => !(left == right);

        public bool Equals(Point other) => this == other;

        public override bool Equals(object obj) => obj is Point other && Equals(other);

        public override int GetHashCode() => X.GetHashCode() * 17 + Y.GetHashCode();
    }
}
