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
    using System.Collections.Generic;

    public struct ProgressDetail
    {
        public static ProgressDetail Create(long progress, long total) => new ProgressDetail(progress, total);

        public static ProgressDetail Create<T>(ProgressDetail<T> detail) => new ProgressDetail(detail.Progress, detail.Total);

        public static ProgressDetail<T> Create<T>(T status) => new ProgressDetail<T>(0, 0, status);

        public static ProgressDetail<T> Create<T>(long progress, long total, T status) =>
            new ProgressDetail<T>(progress, total, status);

        public static ProgressDetail<T> Create<T>(ProgressDetail detail, T status) =>
            new ProgressDetail<T>(detail.Progress, detail.Total, status);

        public static ProgressDetail<T> Create<T, TOther>(ProgressDetail<TOther> detail, T status) =>
            new ProgressDetail<T>(detail.Progress, detail.Total, status);

        public ProgressDetail(long progress, long total)
        {
            Progress = progress;
            Total = total;
        }

        public long Progress { get; }

        public long Total { get; }

        public bool Equals(ProgressDetail other) => this == other;

        public override bool Equals(object obj) => obj is ProgressDetail other && Equals(other);

        public override int GetHashCode() => Progress.GetHashCode() * 17 + Total.GetHashCode();

        public static bool operator ==(ProgressDetail left, ProgressDetail right) =>
            left.Progress == right.Progress && left.Total == right.Total;

        public static bool operator !=(ProgressDetail left, ProgressDetail right) => !(left == right);
    }

    public struct ProgressDetail<T>
    {
        public ProgressDetail(long progress, long total, T status)
        {
            Progress = progress;
            Total = total;
            Status = status;
        }

        public long Progress { get; }

        public long Total { get; }

        public T Status { get; }

        public bool Equals(ProgressDetail<T> other) => this == other;

        public override bool Equals(object obj) => obj is ProgressDetail<T> other && Equals(other);

        public override int GetHashCode()
        {
            var result = Progress.GetHashCode();
            result = result * 17 + Total.GetHashCode();
            result = result * 17 + Status.GetHashCode();
            return result;
        }

        public static bool operator ==(ProgressDetail<T> left, ProgressDetail<T> right) =>
            left.Progress == right.Progress && left.Total == right.Total &&
            EqualityComparer<T>.Default.Equals(left.Status, right.Status);

        public static bool operator !=(ProgressDetail<T> left, ProgressDetail<T> right) => !(left == right);
    }
}
