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
    using System.Collections.Generic;
    using System.ComponentModel;
    using System.Diagnostics.CodeAnalysis;
    using System.Windows.Input;

    public class DelegateCommand : ICommand
    {
        private Func<object, bool> canExecute;
        private Action<object> execute;
        private Dictionary<INotifyPropertyChanged, HashSet<string>> observedProperties =
            new Dictionary<INotifyPropertyChanged, HashSet<string>>();

        public DelegateCommand(Action<object> execute) :
            this(execute, null)
        { }

        public DelegateCommand(Action<object> execute, Func<object, bool> canExecute)
        {
            this.execute = execute ?? throw new ArgumentNullException(nameof(execute));
            this.canExecute = canExecute;
        }

        public event EventHandler CanExecuteChanged;

        public bool CanExecute(object parameter)
        {
            var result = canExecute?.Invoke(parameter) ?? true;
            return result;
        }

        public void Execute(object parameter) => execute(parameter);

        public DelegateCommand ObservesProperty(INotifyPropertyChanged obj, string name)
        {
            if (obj == null)
            {
                throw new ArgumentNullException(nameof(obj));
            }

            if (name == null)
            {
                throw new ArgumentNullException(nameof(name));
            }

            if (!observedProperties.TryGetValue(obj, out var names))
            {
                names = new HashSet<string>();
                observedProperties.Add(obj, names);
                obj.PropertyChanged += PropertyChanged;
            }
            names.Add(name);
            return this;
        }

        [SuppressMessage("Microsoft.Design", "CA1030")]
        public void RaiseCanExecuteChanged() => CanExecuteChanged?.Invoke(this, new EventArgs());

        private void PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            if (sender is INotifyPropertyChanged obj && observedProperties.TryGetValue(obj, out var names))
            {
                if (names.Contains(e.PropertyName))
                {
                    RaiseCanExecuteChanged();
                }
            }
        }
    }
}
