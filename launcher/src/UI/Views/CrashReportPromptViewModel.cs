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
    using System.Diagnostics;
    using System.IO;
    using System.Windows.Input;

    public class CrashReportPromptViewModel : ViewModelBase
    {
        private INavigationService navigation;
        private string location;

        public CrashReportPromptViewModel(INavigationService navigation, string location)
        {
            this.navigation = navigation ?? throw new ArgumentNullException(nameof(navigation));

            Close = new DelegateCommand(ExecuteClose);
            Submit = new DelegateCommand(ExecuteSubmit);
            ShowReport = new DelegateCommand(ExecuteShowReport, CanExecuteShowReport);

            this.location = location;
        }

        public static bool EncryptionEnabled { get; } = CrashReporter.EncryptionEnabled;

        public ICommand ShowReport { get; }

        public ICommand Close { get; }

        public ICommand Submit { get; }

        private void ExecuteShowReport(object arg)
        {
            if (location != null && File.Exists(location))
            {
                _ = Process.Start(location);
            }
        }

        private bool CanExecuteShowReport(object arg) => location != null && File.Exists(location);

        private void ExecuteClose(object arg) => navigation.Close(false);

        private void ExecuteSubmit(object arg) => navigation.Close(true);
    }
}
