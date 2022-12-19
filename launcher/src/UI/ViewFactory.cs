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
    using System.Globalization;
    using System.Reflection;
    using System.Runtime.ExceptionServices;
    using System.Windows;

    internal static class ViewFactory
    {
        public static string ViewFormat { get; set; } = typeof(ViewFactory).Namespace + ".Views.{0}View";
        public static string ViewModelFormat { get; set; } = typeof(ViewFactory).Namespace + ".Views.{0}ViewModel";

        public static FrameworkElement Create(string name, params object[] args)
        {
            try
            {
                var viewType = Type.GetType(string.Format(CultureInfo.InvariantCulture, ViewFormat, name));
                if (viewType == null)
                {
                    throw new ArgumentException("Invalid view name: " + name);
                }

                var viewModelType = Type.GetType(string.Format(CultureInfo.InvariantCulture, ViewModelFormat, name));
                if (viewModelType == null)
                {
                    throw new ArgumentException("Invalid view model name: " + name);
                }

                var view = (FrameworkElement)Activator.CreateInstance(viewType);
                view.DataContext = Activator.CreateInstance(viewModelType, args);
                return view;
            }
            catch (TargetInvocationException e)
            {
                if (e.InnerException != null)
                {
                    ExceptionDispatchInfo.Capture(e.InnerException).Throw();
                }
                throw;
            }
        }
    }
}
