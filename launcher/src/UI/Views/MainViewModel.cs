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
    using System.Threading;
    using System.Threading.Tasks;
    using System.Windows.Input;
    using PlayOnline;

    public class MainViewModel : ViewModelBase
    {
        private INavigationService navigation;
        private CancellationTokenSource cancellationTokenSource;
        private Profile? profile;
        private long progress;
        private long total;
        private object status;

        public MainViewModel(INavigationService navigation, Profile? profile)
        {
            this.navigation = navigation ?? throw new ArgumentNullException(nameof(navigation));
            this.profile = profile;

            Cancel = new DelegateCommand(ExecuteCancel, o => CancellationTokenSource != null);
        }

        public ICommand Cancel { get; }

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

        public static string BuildTag { get; } = Program.BuildTag;

        public bool IsCancellationRequested => CancellationTokenSource?.IsCancellationRequested ?? false;

        private CancellationTokenSource CancellationTokenSource
        {
            get => cancellationTokenSource;
            set
            {
                cancellationTokenSource = value;
                ((DelegateCommand)Cancel).RaiseCanExecuteChanged();
                OnPropertyChanged(nameof(IsCancellationRequested));
            }
        }

        public override void Opened() => OpenedAsync();

        private async void OpenedAsync()
        {
            await Updater.Update(new Progress<ProgressDetail<UpdateStatus>>(d =>
            {
                Total = d.Total;
                Progress = d.Progress;
                Status = d.Status;
            }));

            while (true)
            {
                var selected = Launcher.Resolve(await SelectProfileAsync());
                if (selected.Region == null)
                {
                    await navigation.Open(this, "ClientNotFound");
                    navigation.Close();
                    return;
                }

                if (selected.Region?.IsInstalled() == true && selected.AccessControlPrompt &&
                    Launcher.IsElevationRequired(selected) && !selected.RunAsAdmin)
                {
                    await Task.Delay(250);
                    var (ok, dontAskAgain) = await navigation.Open<(bool ok, bool dontAskAgain)>(this, "FixAccessControlPrompt");
                    if (ok)
                    {
                        await Launcher.FixAccessControlAsync(selected);
                    }

                    if (!dontAskAgain)
                    {
                        if (Launcher.ProfileManager.Remove(selected))
                        {
                            Launcher.ProfileManager.Add(selected.With(AccessControlPrompt: false));
                            Launcher.ProfileManager.Save();
                        }
                    }
                }

                Status = LaunchStatus.CheckingDirectPlay;
                var directPlayInstalled = await Launcher.CheckDirectPlayAsync();
                while (!directPlayInstalled)
                {
                    if ((bool)await navigation.Open(this, "DirectPlayPrompt"))
                    {
                        Status = LaunchStatus.InstallingDirectPlay;
                        directPlayInstalled = await Launcher.InstallDirectPlayAsync();
                    }
                    else
                    {
                        navigation.Close();
                        return;
                    }
                }

                Status = null;
                try
                {
                    using (var source = new CancellationTokenSource())
                    {
                        CancellationTokenSource = source;
                        var progress = new Progress<ProgressDetail<LaunchStatus>>(d =>
                        {
                            Total = d.Total;
                            Progress = d.Progress;
                            Status = d.Status;
                        });
                        await navigation.Uninterruptible(Launcher.LaunchAsync(selected, progress, source.Token));
                        CancellationTokenSource = null;
                    }

                    navigation.Close();
                    return;
                }
                catch (OperationCanceledException) { }

                Status = null;
                CancellationTokenSource = null;
            }
        }

        private async Task<Profile> SelectProfileAsync()
        {
            var selected = profile ?? (Profile)await navigation.Open(this, "SelectProfile", Launcher.ProfileManager);
            profile = null;
            return selected;
        }

        private void ExecuteCancel(object obj)
        {
            CancellationTokenSource?.Cancel();
            OnPropertyChanged(nameof(IsCancellationRequested));
        }
    }
}
