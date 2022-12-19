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
    using System.Collections.Immutable;
    using System.Diagnostics;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Threading.Tasks;
    using System.Windows;
    using System.Windows.Input;
    using System.Windows.Shell;

    public class SelectProfileViewModel : ViewModelBase
    {
        private INavigationService navigation;
        private ProfileManager manager;
        private Profile? selectedProfile;

        private static readonly string executablePath = new Uri(typeof(SelectProfileViewModel).Assembly.EscapedCodeBase).LocalPath;

        public SelectProfileViewModel(INavigationService navigation, ProfileManager manager)
        {
            this.navigation = navigation ?? throw new ArgumentNullException(nameof(navigation));
            this.manager = manager ?? throw new ArgumentNullException(nameof(manager));

            Launch = new DelegateCommand(ExecuteLaunch, o => SelectedProfile != null)
                .ObservesProperty(this, nameof(SelectedProfile));
            CreateProfile = new DelegateCommand(ExecuteCreateProfileAsync);
            DeleteProfile = new DelegateCommand(ExecuteDeleteProfile, o => SelectedProfile != null && Profiles.Count > 1)
                .ObservesProperty(this, nameof(SelectedProfile))
                .ObservesProperty(this, nameof(Profiles));
            EditProfile = new DelegateCommand(ExecuteEditProfileAsync, o => SelectedProfile != null)
                .ObservesProperty(this, nameof(SelectedProfile));
            CreateProfileLink = new DelegateCommand(ExecuteCreateProfileLink, o => SelectedProfile != null)
                .ObservesProperty(this, nameof(SelectedProfile));
            OpenAboutPage = new DelegateCommand(ExecuteOpenAboutPage);
            OpenWebpage = new DelegateCommand(ExecuteOpenWebpage);

            this.manager.ProfilesChanged += ProfilesChanged;
        }

        public override void Opened()
        {
            if (manager.Profiles.Count <= 0)
            {
                ExecuteCreateProfileAsync(null);
            }
            else
            {
                SelectDefaultProfile();
            }
        }

        public override void Closed() => manager.ProfilesChanged -= ProfilesChanged;

        public ICommand Launch { get; }

        public ICommand CreateProfile { get; }

        public ICommand DeleteProfile { get; }

        public ICommand EditProfile { get; }

        public ICommand CreateProfileLink { get; }

        public ICommand OpenAboutPage { get; }

        public ICommand OpenWebpage { get; }

        public bool IsAdministrator { get; } = Launcher.IsAdministrator();

        public bool IsElevationRequired =>
            SelectedProfile != null && Launcher.IsElevationRequired((Profile)SelectedProfile);

        public IImmutableSet<Profile> Profiles => manager.Profiles;

        public Profile? SelectedProfile
        {
            get => selectedProfile;
            set
            {
                Set(ref selectedProfile, value);
                OnPropertyChanged(nameof(IsElevationRequired));
            }
        }

        private void ProfilesChanged(object sender, EventArgs e)
        {
            SelectDefaultProfile();
            OnPropertyChanged(nameof(Profiles));

            var list = new JumpList
            {
                ShowFrequentCategory = false,
                ShowRecentCategory = false,
            };
            list.JumpItems.AddRange(Profiles.Select(profile => new JumpTask
            {
                ApplicationPath = executablePath,
                Arguments = GetLaunchArgs(profile),
                Title = profile.Name
            }));
            JumpList.SetJumpList(System.Windows.Application.Current, list);
        }

        private void SelectDefaultProfile()
        {
            if (SelectedProfile == null || !Profiles.Contains((Profile)SelectedProfile))
            {
                SelectedProfile = Profiles.FirstOrDefault();
            }
        }

        private void ExecuteLaunch(object arg) => navigation.Close((Profile)SelectedProfile);

        private async void ExecuteCreateProfileAsync(object arg)
        {
            var profile = arg as Profile?;
            if (profile == null)
            {
                var format = Application.Current.TryFindResource("Strings.NewProfileNameFormat") as string ?? "{0}";
                var name = Enumerable.Range(1, int.MaxValue)
                    .Select(i => string.Format(CultureInfo.CurrentUICulture, format, i))
                    .First(str => !manager.Contains(str));
                profile = Profile.Default.With(Name: name);
            }
            else
            {
                var originalName = ((Profile)profile).Name;
                var format = Application.Current.TryFindResource("Strings.ProfileCopyNameFormat") as string ?? "{0} {1}";
                var name = Enumerable.Range(1, int.MaxValue)
                    .Select(i => string.Format(CultureInfo.CurrentUICulture, format, originalName, i))
                    .First(str => !manager.Contains(str));
                profile = ((Profile)profile).With(Name: name);
            }

            try
            {
                var result = (Profile?)await navigation.Open(this, "EditProfile", manager, profile, manager.Profiles.Count > 0);
                if (result != null)
                {
                    manager.Add((Profile)result);
                    SelectedProfile = (Profile)result;
                    manager.Save();
                }
            }
            catch (TaskCanceledException) { }
        }

        private void ExecuteDeleteProfile(object arg)
        {
            manager.Remove((Profile)SelectedProfile);
            manager.Save();
        }

        private async void ExecuteEditProfileAsync(object arg)
        {
            var profile = (Profile)SelectedProfile;
            var result = (Profile?)await navigation.Open(this, "EditProfile", manager, profile);
            if (result != null)
            {
                manager.Remove(profile);
                manager.Add((Profile)result);
                SelectedProfile = (Profile)result;
                manager.Save();
            }
        }

        private void ExecuteCreateProfileLink(object arg)
        {
            var profile = (Profile)SelectedProfile;
            var format = System.Windows.Application.Current.TryFindResource("Strings.ProfileLinkFormat") as string;
            var name = string.Format(CultureInfo.InvariantCulture, format, profile.Name);
            name = Path.GetInvalidPathChars().Aggregate(name, (s, c) => s.Replace(c.ToString(), string.Empty)) + ".lnk";
            var path = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.Desktop), name);
            var args = GetLaunchArgs(profile);
            Shell.CreateLink(path, executablePath, args);
        }

        private void ExecuteOpenAboutPage(object arg) => navigation.Open(this, "About");

        private void ExecuteOpenWebpage(object arg)
        {
            if (arg is string url)
            {
                Process.Start(url);
            }
        }

        private static string GetLaunchArgs(Profile profile) =>
            FormattableString.Invariant($"launch \"{profile.Name.Replace("\"", "\\\"")}\"");
    }
}
