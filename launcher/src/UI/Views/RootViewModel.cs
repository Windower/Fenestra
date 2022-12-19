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

namespace Windower.UI.Views
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Threading.Tasks;
    using System.Windows;

    public class RootViewModel : ViewModelBase, INavigationService
    {
        private bool interruptible;
        private readonly Stack<Tuple<object, FrameworkElement, TaskCompletionSource<object>>> views =
            new Stack<Tuple<object, FrameworkElement, TaskCompletionSource<object>>>();

        public RootViewModel(string name, params object[] args)
        {
            Interruptible = true;

            Open(null, name, args);
        }

        public bool Interruptible
        {
            get => interruptible;
            private set => Set(ref interruptible, value);
        }

        public object CurrentView => views.FirstOrDefault()?.Item2;

        public Task<object> Open(object caller, string name, params object[] args)
        {
            if (args == null)
            {
                throw new ArgumentNullException(nameof(args));
            }

            if (!Interruptible)
            {
                throw new InvalidOperationException();
            }

            if (views.Any(t => t.Item1 == caller))
            {
                return Task.FromResult<object>(null);
            }

            var temp = new object[args.Length + 1];
            temp[0] = this;
            Array.Copy(args, 0, temp, 1, args.Length);

            var view = ViewFactory.Create(name, temp);
            views.Push(Tuple.Create(caller, view, new TaskCompletionSource<object>()));
            var task = views.Peek().Item3.Task;
            (view.DataContext as ViewModelBase)?.Opened();

            OnPropertyChanged(nameof(CurrentView));
            return task;
        }

        async public Task<T> Open<T>(object caller, string name, params object[] args) => (T)await Open(caller, name, args);

        public void Close() => Close(null);

        public void Close(object result)
        {
            if (!Interruptible)
            {
                throw new InvalidOperationException();
            }

            if (views.Count > 0)
            {
                var pair = views.Pop();

                OnPropertyChanged(nameof(CurrentView));

                (pair.Item2.DataContext as ViewModelBase)?.Closed();
                pair.Item3.SetResult(result);
            }

            if (views.Count == 0)
            {
                foreach (var window in Application.Current.Windows.Cast<Window>())
                {
                    if (window.DataContext == this)
                    {
                        window.Close();
                    }
                }
            }
        }

        public async Task Uninterruptible(Task task)
        {
            try
            {
                Interruptible = false;
                await task;
            }
            finally
            {
                Interruptible = true;
            }
        }

        public async Task<T> Uninterruptible<T>(Task<T> task)
        {
            try
            {
                Interruptible = false;
                return await task;
            }
            finally
            {
                Interruptible = true;
            }
        }
    }
}
