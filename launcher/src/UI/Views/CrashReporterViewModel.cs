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
    using Core;
    using System;
    using System.IO;
    using System.Net;
    using System.Threading.Tasks;

    public class CrashReporterViewModel : ViewModelBase
    {
        private INavigationService navigation;
        private string signature;
        private string crashDumpPath;
        private string stackTrace;
        private long progress;
        private long total;
        private object status;

        public CrashReporterViewModel(INavigationService navigation, string signature, string crashDumpPath, string stackTrace)
        {
            this.navigation = navigation ?? throw new ArgumentNullException(nameof(navigation));
            this.signature = signature;
            this.crashDumpPath = crashDumpPath;
            this.stackTrace = stackTrace;
        }

        public long Total
        {
            get => total;
            set
            {
                total = value;
                OnPropertyChanged();
            }
        }

        public long Progress
        {
            get => progress;
            set
            {
                progress = value;
                OnPropertyChanged();
            }
        }

        public object Status
        {
            get => status;
            set
            {
                status = value;
                OnPropertyChanged();
            }
        }

        public override async void Opened()
        {
            var progress = new Progress<ProgressDetail<string>>(d =>
            {
                Total = d.Total;
                Progress = d.Progress;
                Status = d.Status;
            });

            bool delete;
            FileStream stream = null;
            string file = null;
            try
            {
                (stream, file) = await navigation.Uninterruptible(
                    CrashReporter.PrepareCrashDumpAsync(crashDumpPath, stackTrace, progress.Wrap("Preparing")));
                delete = (bool)await navigation.Open(this, "CrashReportDetails", "ReportingDisabled", file, null);
            }
            catch (WebException)
            {
                delete = (bool)await navigation.Open(this, "CrashReportDetails", "ReportingFailed", file, null);
            }
            finally
            {
                stream?.Dispose();
            }

            if (delete)
            {
                try
                {
                    await Task.Run(() => File.Delete(file));
                }
                catch (IOException) { }
            }

            navigation.Close();
        }

        private async Task<NetworkCredential> GetCredential(string message, IProgress<ProgressDetail<string>> progressReporter)
        {
            var task = navigation.Open(this, "GitHubSignIn", message);
            await Task.Delay(500);
            progressReporter.Report("SigningIn");
            return (NetworkCredential)await task;
        }

        private async Task<string> GetOnetimePassword(string type, IProgress<ProgressDetail<string>> progressReporter)
        {
            var task = navigation.Open(this, "GitHubOnetimePassword", type);
            await Task.Delay(500);
            progressReporter.Report("Verifying");
            return (string)await task;
        }
    }
}
