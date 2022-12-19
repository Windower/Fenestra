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
    using Boiler;
    using Core;
    using PlayOnline;
    using System;
    using System.Collections.Generic;
    using System.Collections.Immutable;
    using System.ComponentModel;
    using System.Linq;
    using System.Windows.Forms;
    using System.Windows.Input;

    public class EditProfileViewModel : ViewModelBase, IDataErrorInfo
    {
        private readonly INavigationService navigation;
        private ProfileManager manager;
        private Profile profile;
        private string originalName;
        private string profileName;

        public EditProfileViewModel(INavigationService navigation, ProfileManager manager, Profile profile) :
            this(navigation, manager, profile, true)
        { }

        public EditProfileViewModel(INavigationService navigation, ProfileManager manager, Profile profile, bool editCancellable)
        {
            this.navigation = navigation ?? throw new ArgumentNullException(nameof(navigation));
            this.manager = manager ?? throw new ArgumentNullException(nameof(manager));
            this.profile = profile;

            originalName = profile.Name;

            Save = new DelegateCommand(ExecuteSave);
            Cancel = new DelegateCommand(ExecuteCancel, o => EditCancellable);
            BrowseSettingsPath = new DelegateCommand(ExecuteBrowseSettingsPath);
            BrowseUserPath = new DelegateCommand(ExecuteBrowseUserPath);
            BrowseTempPath = new DelegateCommand(ExecuteBrowseTempPath);
            FixAccessControl = new DelegateCommand(ExecuteFixAccessControl, o => !profile.RunAsAdmin && IsElevationRequired);
            EditCancellable = editCancellable;
        }

        public ICommand Save { get; }

        public ICommand Cancel { get; }

        public ICommand BrowseSettingsPath { get; }

        public ICommand BrowseUserPath { get; }

        public ICommand BrowseTempPath { get; }

        public ICommand FixAccessControl { get; }

        public bool EditCancellable { get; }

        public bool IsAdministrator { get; } = Launcher.IsAdministrator();

        public bool IsElevationRequired => Launcher.Resolve(profile).Region?.IsInstalled() == true
            && Launcher.IsElevationRequired(profile) && !profile.RunAsAdmin;

        public string Name
        {
            get => profileName ?? profile.Name;
            set
            {
                profileName = value;
                profile = profile.With(Name: value);
                OnPropertyChanged();
            }
        }

        public Region? Region
        {
            get => profile.Region;
            set
            {
                Set(ref profile, profile.With(Region: value));
                OnPropertyChanged(nameof(IsSteamAvailable));
                OnPropertyChanged(nameof(IsElevationRequired));
                if (!IsSteamAvailable)
                {
                    UseSteam = false;
                }
                ((DelegateCommand)FixAccessControl).RaiseCanExecuteChanged();
            }
        }

        public bool IsSteamAvailable => Steam.IsInstalled && IsInstalled(Region);

        public bool UseSteam
        {
            get => profile.UseSteam || (Steam.IsInstalled && !IsInstalled(Region) && IsOwned(Region));
            set
            {
                Set(ref profile, profile.With(UseSteam: value));
                OnPropertyChanged(nameof(IsElevationRequired));
                ((DelegateCommand)FixAccessControl).RaiseCanExecuteChanged();
            }
        }

        public WindowType WindowType
        {
            get => profile.WindowType;
            set
            {
                Set(ref profile, profile.With(WindowType: value));
                OnPropertyChanged(nameof(UIScale));
                OnPropertyChanged(nameof(GammaMin));
                OnPropertyChanged(nameof(GammaMax));
                if (Gamma < GammaMin)
                {
                    Gamma = GammaMin;
                }
                FixBounds();
            }
        }

        public static IImmutableList<DisplayDevice> Displays { get; } = DisplayInfo.Devices
            .Insert(0, DisplayInfo.PrimaryDevice.With(DeviceName: null));

        public static int DisplayCount { get; } = DisplayInfo.Devices.Count;

        public DisplayDevice SelectedDisplay
        {
            get => Displays.Where(d => d.DeviceName == profile.Display)
                .DefaultIfEmpty(new DisplayDevice(profile.Display, profile.Display, false,
                    profile.Resolution ?? new Dimension(1024, 768), profile.Position ?? new Point(0, 0), 96, null)).First();

            set
            {
                Set(ref profile, profile.With(Display: value.DeviceName));
                OnPropertyChanged(nameof(ResolutionMin));
                OnPropertyChanged(nameof(ResolutionMax));
                OnPropertyChanged(nameof(CanUseCustomResolution));
                if (!CanUseCustomResolution)
                {
                    UseCustomResolution = false;
                }
                FixBounds();
            }
        }

        public IEnumerable<Dimension> AvailableResolutions =>
            SelectedDisplay.AvailableResolutions?.Where(d => d.Width >= ResolutionMin.Width && d.Height >= ResolutionMin.Height);

        public Dimension ResolutionMin { get; } = new Dimension(640, 480);

        public Dimension ResolutionMax => SelectedDisplay.CurrentResolution;

        public bool CanUseCustomResolution =>
            ResolutionMax.Width > ResolutionMin.Width && ResolutionMax.Height > ResolutionMin.Height;

        public bool UseCustomResolution
        {
            get => profile.Resolution != null;
            set
            {
                Set(ref profile, profile.With(Resolution: value ? GetDefaultResoluton() : (Dimension?)null));
                OnPropertyChanged(nameof(CanUseCustomUIScale));
                OnPropertyChanged(nameof(CustomResolutionWidth));
                OnPropertyChanged(nameof(CustomResolutionHeight));
                OnPropertyChanged(nameof(UIScale));
                OnPropertyChanged(nameof(UIScaleMin));
                OnPropertyChanged(nameof(UIScaleMax));
                if (!CanUseCustomUIScale)
                {
                    UseCustomUIScale = false;
                }
            }
        }

        public int CustomResolutionWidth
        {
            get => profile.Resolution?.Width ?? GetDefaultResoluton().Width;
            set
            {
                Set(ref profile, profile.With(Resolution: profile.Resolution?.With(Width: value)));
                OnPropertyChanged(nameof(CanUseCustomUIScale));
                OnPropertyChanged(nameof(UIScaleMin));
                OnPropertyChanged(nameof(UIScaleMax));
                if (UIScale > UIScaleMax)
                {
                    UIScale = UIScaleMax;
                }

                if (!CanUseCustomUIScale)
                {
                    UseCustomUIScale = false;
                }
            }
        }

        public int CustomResolutionHeight
        {
            get => profile.Resolution?.Height ?? GetDefaultResoluton().Height;
            set
            {
                Set(ref profile, profile.With(Resolution: profile.Resolution?.With(Height: value)));
                OnPropertyChanged(nameof(CanUseCustomUIScale));
                OnPropertyChanged(nameof(UIScaleMin));
                OnPropertyChanged(nameof(UIScaleMax));
                if (UIScale > UIScaleMax)
                {
                    UIScale = UIScaleMax;
                }

                if (!CanUseCustomUIScale)
                {
                    UseCustomUIScale = false;
                }
            }
        }

        public bool UseCustomPosition
        {
            get => profile.Position != null;
            set
            {
                Set(ref profile, profile.With(Position: value ? GetDefaultPosition() : (Point?)null));
                OnPropertyChanged(nameof(CustomPositionX));
                OnPropertyChanged(nameof(CustomPositionY));
            }
        }

        public int CustomPositionX
        {
            get => profile.Position?.X ?? GetDefaultPosition().X;
            set => Set(ref profile, profile.With(Position: profile.Position?.With(X: value)));
        }

        public int CustomPositionY
        {
            get => profile.Position?.Y ?? GetDefaultPosition().Y;
            set => Set(ref profile, profile.With(Position: profile.Position?.With(Y: value)));
        }

        public float SamplesPerPixel
        {
            get => profile.SamplesPerPixel;
            set => Set(ref profile, profile.With(SamplesPerPixel: value));
        }

        public float SamplesPerPixelMin { get; } = 0.25f;

        public float SamplesPerPixelMax { get; } = 4f;

        public bool CanUseCustomUIScale => UIScaleMax > UIScaleMin;

        public bool UseCustomUIScale
        {
            get => profile.UIScale != null;
            set
            {
                Set(ref profile, profile.With(UIScale: value ? GetDefaultUIScale() : (float?)null));
                OnPropertyChanged(nameof(UIScale));
            }
        }

        public float UIScale
        {
            get => profile.UIScale ?? GetDefaultUIScale();
            set => Set(ref profile, profile.With(UIScale: value));
        }

        public float UIScaleMin { get; } = 1f;

        public float UIScaleMax =>
            Math.Max(UIScaleMin,
                (float)Math.Floor(Math.Min(CustomResolutionWidth / 640f, CustomResolutionHeight / 480f) * 20) / 20f);

        public bool HardwareMouse
        {
            get => profile.HardwareMouse;
            set => Set(ref profile, profile.With(HardwareMouse: value));
        }

        public int MaxSounds
        {
            get => profile.MaxSounds;
            set => Set(ref profile, profile.With(MaxSounds: value));
        }

        public int MaxSoundsMin { get; } = 0;

        public int MaxSoundsMax { get; } = 32;

        public bool PlaySoundWhenUnfocused
        {
            get => profile.PlaySoundWhenUnfocused;
            set => Set(ref profile, profile.With(PlaySoundWhenUnfocused: value));
        }

        public int Mipmapping
        {
            get => profile.Mipmapping;
            set => Set(ref profile, profile.With(Mipmapping: value));
        }

        public int MipmappingMin { get; } = 0;

        public int MipmappingMax { get; } = 3;

        public bool BumpMapping
        {
            get => profile.BumpMapping;
            set => Set(ref profile, profile.With(BumpMapping: value));
        }

        public bool MapCompression
        {
            get => profile.MapCompression;
            set => Set(ref profile, profile.With(MapCompression: value));
        }

        public TextureCompression TextureCompression
        {
            get => profile.TextureCompression;
            set => Set(ref profile, profile.With(TextureCompression: value));
        }

        public EnvironmentAnimation EnvironmentAnimation
        {
            get => profile.EnvironmentAnimation;
            set => Set(ref profile, profile.With(EnvironmentAnimation: value));
        }

        public FontType FontType
        {
            get => profile.FontType;
            set => Set(ref profile, profile.With(FontType: value));
        }

        public bool UseCustomGamma
        {
            get => profile.Gamma != null;
            set
            {
                Set(ref profile, profile.With(Gamma: value ? 2.2f : (float?)null));
                OnPropertyChanged(nameof(Gamma));
            }
        }

        public float Gamma
        {
            get => profile.Gamma ?? 2.2f;
            set => Set(ref profile, profile.With(Gamma: value));
        }

        public float GammaMin => WindowType == WindowType.FullScreen ? 0.1f : 0.8f;

        public float GammaMax { get; } = 4f;

        public bool DriverStability
        {
            get => profile.DriverStability;
            set => Set(ref profile, profile.With(DriverStability: value));
        }

        public bool PlayIntro
        {
            get => profile.PlayIntro;
            set => Set(ref profile, profile.With(PlayIntro: value));
        }

        public bool Debug
        {
            get => profile.Debug;
            set => Set(ref profile, profile.With(Debug: value));
        }

        public bool CustomSettingsPath
        {
            get => profile.SettingsPath != null;
            set => Set(ref profile, profile.With(SettingsPath: value ? Paths.GlobalSettingsPath : null));
        }

        public string SettingsPath
        {
            get => profile.SettingsPath ?? Paths.GlobalSettingsPath;
            set => Set(ref profile, profile.With(SettingsPath: value));
        }

        public bool CustomUserPath
        {
            get => profile.UserPath != null;
            set => Set(ref profile, profile.With(UserPath: value ? Paths.GlobalUserPath : null));
        }

        public string UserPath
        {
            get => profile.UserPath ?? Paths.GlobalUserPath;
            set => Set(ref profile, profile.With(UserPath: value));
        }

        public bool CustomTempPath
        {
            get => profile.TempPath != null;
            set => Set(ref profile, profile.With(TempPath: value ? Paths.GlobalTempPath : null));
        }

        public string TempPath
        {
            get => profile.TempPath ?? Paths.GlobalTempPath;
            set => Set(ref profile, profile.With(TempPath: value));
        }

        public string Error { get; } = null;

        public string this[string columnName]
        {
            get
            {
                switch (columnName)
                {
                    case "Name":
                        if (string.IsNullOrWhiteSpace(Name))
                        {
                            return GetString("Strings.ProfileNameEmptyError");
                        }
                        if (Name != originalName && manager.Contains(Name))
                        {
                            return GetString("Strings.ProfileNameConflictError");
                        }
                        break;
                }

                return string.Empty;
            }
        }

        private static bool IsInstalled(Region? region) =>
            region?.IsInstalled() ?? ClientInfo.InstalledRegions.Any(r => r.GetInstalledSteamAppIds().Any());

        private static bool IsOwned(Region? region) =>
            region?.IsOwned() ?? ClientInfo.OwnedRegions.Any(r => r.GetOwnedSteamAppIds().Any());

        private Dimension GetDefaultResoluton() =>
            WindowType == WindowType.Window ? new Dimension(1280, 720) : SelectedDisplay.CurrentResolution;

        private Point GetDefaultPosition()
        {
            var display = SelectedDisplay;
            if (WindowType == WindowType.FullScreen)
            {
                return display.CurrentPosition;
            }
            else
            {
                var x = (display.CurrentResolution.Width - CustomResolutionWidth) / 2;
                var y = (display.CurrentResolution.Height - CustomResolutionHeight) / 2;
                return new Point(display.CurrentPosition.X + Math.Max(0, x), display.CurrentPosition.Y + Math.Max(0, y));
            }
        }

        private float GetDefaultUIScale() =>
            WindowType == WindowType.FullScreen ? 1f :
            Math.Min(UIScaleMax, (float)Math.Round(SelectedDisplay.CurrentDpi / 96f * 100f) / 100f);

        private void FixBounds()
        {
            FixResolution();
            FixPosition();
        }

        private void FixResolution()
        {
            if (profile.Resolution != null)
            {
                if (WindowType == WindowType.FullScreen)
                {
                    if (SelectedDisplay.AvailableResolutions.All(res => res != profile.Resolution))
                    {
                        profile = profile.With(Resolution: SelectedDisplay.CurrentResolution);
                    }
                }
                else
                {
                    var min = ResolutionMin;
                    var max = ResolutionMax;
                    var selected = (Dimension)profile.Resolution;
                    if (selected.Width < min.Width || selected.Height < min.Height ||
                        selected.Width > max.Width || selected.Height > max.Height)
                    {
                        profile = profile.With(Resolution: GetDefaultResoluton());
                    }
                }
            }

            OnPropertyChanged(nameof(CustomResolutionWidth));
            OnPropertyChanged(nameof(CustomResolutionHeight));
        }

        private void FixPosition()
        {
            if (profile.Position != null)
            {
                if (WindowType != WindowType.FullScreen)
                {
                    var displayPosition = SelectedDisplay.CurrentPosition;
                    var displayResolution = SelectedDisplay.CurrentResolution;
                    var selectedPosition = (Point)profile.Position;
                    var selectedResolution = (Dimension)profile.Resolution;

                    if (selectedPosition.X < displayPosition.X || selectedPosition.Y < displayPosition.Y ||
                        selectedPosition.X + selectedResolution.Width > displayPosition.X + displayResolution.Width ||
                        selectedPosition.Y + selectedResolution.Height > displayPosition.Y + displayResolution.Height)
                    {
                        profile = profile.With(Position: GetDefaultPosition());
                    }
                }
            }

            OnPropertyChanged(nameof(CustomPositionX));
            OnPropertyChanged(nameof(CustomPositionY));
        }

        private void ExecuteSave(object obj) => navigation.Close(profile);

        private void ExecuteCancel(object obj) => navigation.Close();

        private void ExecuteBrowseSettingsPath(object obj)
        {
            var dialog = new FolderBrowserDialog();
            dialog.Description = obj as string;
            dialog.SelectedPath = Paths.ExpandPath(SettingsPath);
            if (dialog.ShowDialog() == DialogResult.OK)
            {
                SettingsPath = dialog.SelectedPath;
            }
        }

        private void ExecuteBrowseUserPath(object obj)
        {
            var dialog = new FolderBrowserDialog();
            dialog.Description = obj as string;
            dialog.SelectedPath = Paths.ExpandPath(UserPath);
            if (dialog.ShowDialog() == DialogResult.OK)
            {
                UserPath = dialog.SelectedPath;
            }
        }

        private void ExecuteBrowseTempPath(object obj)
        {
            var dialog = new FolderBrowserDialog();
            dialog.Description = obj as string;
            dialog.SelectedPath = Paths.ExpandPath(TempPath);
            if (dialog.ShowDialog() == DialogResult.OK)
            {
                TempPath = dialog.SelectedPath;
            }
        }

        private void ExecuteFixAccessControl(object obj) => Launcher.FixAccessControl(profile);

        private static string GetString(string name) => System.Windows.Application.Current.TryFindResource(name) as string;
    }
}
