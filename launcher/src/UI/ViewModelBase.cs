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

namespace Windower.UI
{
    using System;
    using System.ComponentModel;
    using System.Diagnostics;
    using System.Diagnostics.CodeAnalysis;
    using System.Runtime.CompilerServices;
    using System.Threading;
    using System.Windows;

    public abstract class ViewModelBase : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        public virtual void Opened() { }

        public virtual void Closed() { }

        protected static Func<TResult> Dispatch<TResult>(Func<TResult> function) =>
            () =>
            {
                if (Thread.CurrentThread == Application.Current.Dispatcher.Thread)
                {
                    return function();
                }
                return Application.Current.Dispatcher.Invoke(function);
            };

        protected static Func<T1, TResult> Dispatch<T1, TResult>(Func<T1, TResult> function) =>
            a1 =>
            {
                if (Thread.CurrentThread == Application.Current.Dispatcher.Thread)
                {
                    return function(a1);
                }
                return (TResult)Application.Current.Dispatcher.Invoke(function, a1);
            };

        protected static Func<T1, T2, TResult> Dispatch<T1, T2, TResult>(Func<T1, T2, TResult> function) =>
            (a1, a2) =>
            {
                if (Thread.CurrentThread == Application.Current.Dispatcher.Thread)
                {
                    return function(a1, a2);
                }
                return (TResult)Application.Current.Dispatcher.Invoke(function, a1, a2);
            };

        protected static Func<T1, T2, T3, TResult> Dispatch<T1, T2, T3, TResult>(Func<T1, T2, T3, TResult> function) =>
            (a1, a2, a3) =>
            {
                if (Thread.CurrentThread == Application.Current.Dispatcher.Thread)
                {
                    return function(a1, a2, a3);
                }
                return (TResult)Application.Current.Dispatcher.Invoke(function, a1, a2, a3);
            };

        protected static Func<T1, T2, T3, T4, TResult> Dispatch<T1, T2, T3, T4, TResult>(Func<T1, T2, T3, T4, TResult> function) =>
            (a1, a2, a3, a4) =>
            {
                if (Thread.CurrentThread == Application.Current.Dispatcher.Thread)
                {
                    return function(a1, a2, a3, a4);
                }
                return (TResult)Application.Current.Dispatcher.Invoke(function, a1, a2, a3, a4);
            };

        [SuppressMessage("Microsoft.Design", "CA1026")]
        protected void OnPropertyChanged([CallerMemberName]string propertyName = null)
        {
            ValidatePropertyName(propertyName);
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        [SuppressMessage("Microsoft.Design", "CA1045")]
        [SuppressMessage("Microsoft.Design", "CA1026")]
        protected void Set<T>(ref T backingField, T value, [CallerMemberName]string propertyName = null)
        {
            backingField = value;
            OnPropertyChanged(propertyName);
        }

        [Conditional("DEBUG")]
        [DebuggerStepThrough]
        private void ValidatePropertyName(string propertyName)
        {
            if (propertyName == null)
            {
                throw new ArgumentNullException(nameof(propertyName));
            }

            if (TypeDescriptor.GetProperties(this)[propertyName] == null)
            {
                throw new ArgumentException("Invalid property name: " + propertyName, nameof(propertyName));
            }
        }
    }
}
