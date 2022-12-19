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

    public struct Maybe<T>
    {
        public Maybe(T value)
        {
            Value = value;
            HasValue = true;
        }

        public T Value { get; }

        public bool HasValue { get; }

        public bool Equals(Maybe<T> other) => this == other;

        public bool Equals(T other) => this == other;

        public override bool Equals(object obj)
        {
            if (obj is Maybe<T> other)
            {
                return this == other;
            }
            else if (obj is T)
            {
                return this == (T)obj;
            }

            return false;
        }

        public override int GetHashCode() => HasValue.GetHashCode() * 17 + (HasValue ? Value?.GetHashCode() ?? -1 : 0);

        public static bool operator ==(Maybe<T> left, T right) => left.HasValue && Equals(left.Value, right);

        public static bool operator !=(Maybe<T> left, T right) => !(left == right);

        public static bool operator ==(T left, Maybe<T> right) => right.HasValue && Equals(left, right.Value);

        public static bool operator !=(T left, Maybe<T> right) => !(left == right);

        public static bool operator ==(Maybe<T> left, Maybe<T> right) =>
            !left.HasValue && !right.HasValue || left.HasValue && right.HasValue && Equals(left.Value, right.Value);

        public static bool operator !=(Maybe<T> left, Maybe<T> right) => !(left == right);

        [SuppressMessage("Microsoft.Usage", "CA2225:OperatorOverloadsHaveNamedAlternates")]
        public static implicit operator Maybe<T>(T value) => new Maybe<T>(value);

        public T Default() => HasValue ? Value : default(T);

        public T Default(T defaultValue) => HasValue ? Value : defaultValue;

        [SuppressMessage("Microsoft.Design", "CA1006:DoNotNestGenericTypesInMemberSignatures")]
        public Maybe<TResult> Bind<TResult>(Func<T, Maybe<TResult>> apply)
        {
            if (apply == null)
            {
                throw new ArgumentNullException(nameof(apply));
            }

            return HasValue ? apply(Value) : default(Maybe<TResult>);
        }

        public Maybe<TResult> Bind<TResult>(Func<T, TResult> apply)
        {
            if (apply == null)
            {
                throw new ArgumentNullException(nameof(apply));
            }

            return HasValue ? new Maybe<TResult>(apply(Value)) : default(Maybe<TResult>);
        }
    }
}
