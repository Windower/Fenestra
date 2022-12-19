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

    public class CrashReportDetailsViewModel : ViewModelBase
    {
        private INavigationService navigation;
        private string localReport;
        private Uri remoteReport;
        private bool deleteLocalReport;

        public CrashReportDetailsViewModel(INavigationService navigation, string status, string localReport, Uri remoteReport)
        {
            this.navigation = navigation ?? throw new ArgumentNullException(nameof(navigation));

            Close = new DelegateCommand(ExecuteClose);
            ShowLocalReport = new DelegateCommand(ExecuteShowLocalReport, CanExecuteShowLocalReport);
            ShowRemoteReport = new DelegateCommand(ExecuteShowRemoteReport, CanExecuteShowRemoteReport);
            Status = status;

            switch (Status)
            {
                default:
                case "ReportNotSubmitted":
                case "ReportingDisabled":
                    DeleteLocalReport = false;
                    break;

                case "ReportSubmitted":
                case "ReportExists":
                    DeleteLocalReport = true;
                    break;
            }

            this.localReport = localReport;
            this.remoteReport = remoteReport;
        }

        public ICommand ShowLocalReport { get; }

        public ICommand ShowRemoteReport { get; }

        public ICommand Close { get; }

        public string Status { get; }

        public bool DeleteLocalReport
        {
            get => deleteLocalReport;
            set => Set(ref deleteLocalReport, value);
        }

        private void ExecuteShowLocalReport(object obj)
        {
            if (File.Exists(localReport))
            {
                Process.Start(localReport);
            }
        }

        private bool CanExecuteShowLocalReport(object obj) => File.Exists(localReport);

        private void ExecuteShowRemoteReport(object obj)
        {
            if (remoteReport.IsAbsoluteUri)
            {
                Process.Start(remoteReport.AbsoluteUri);
            }
        }

        private bool CanExecuteShowRemoteReport(object obj) => remoteReport != null && remoteReport.IsAbsoluteUri;

        private void ExecuteClose(object arg) => navigation.Close(DeleteLocalReport);
    }
}
