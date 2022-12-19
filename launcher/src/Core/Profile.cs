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

namespace Windower.Core
{
    using PlayOnline;
    using System;
    using System.Collections.Generic;
    using System.Diagnostics.CodeAnalysis;
    using System.Text;

    [SuppressMessage("Microsoft.Naming", "CA1724")]
    [Serializable]
    public struct Profile : IEquatable<Profile>
    {
        private readonly string name;
        private readonly float? samplesPerPixel;
        private readonly bool? hardwareMouse;
        private readonly int? maxSounds;
        private readonly bool? playSoundWhenUnfocused;
        private readonly TextureCompression? textureCompression;
        private readonly EnvironmentAnimation? environmentAnimation;
        private readonly FontType? fontType;
        private readonly bool? accessControlPrompt;

        public static Profile Default { get; } = default(Profile);

        public Profile(string name, Region? region, bool useSteam, string executable, string executableArgs, bool runAsAdmin,
            WindowType windowType, string display, Dimension? resolution, Point? position, float samplesPerPixel, float? uiScale,
            bool hardwareMouse, int maxSounds, bool playSoundWhenUnfocused, int mipmapping, bool bumpMapping, bool mapCompression,
            TextureCompression textureCompression, EnvironmentAnimation environmentAnimation, FontType fontType, float? gamma,
            bool driverStability, bool playIntro, bool debug, bool developerMode, string settingsPath, string userPath,
            string tempPath, bool accessControlPrompt)
        {
            this.name = name?.Trim() ?? throw new ArgumentNullException(nameof(name));
            this.samplesPerPixel = samplesPerPixel;
            this.hardwareMouse = hardwareMouse;
            this.maxSounds = maxSounds;
            this.playSoundWhenUnfocused = playSoundWhenUnfocused;
            this.textureCompression = textureCompression;
            this.environmentAnimation = environmentAnimation;
            this.fontType = fontType;
            this.accessControlPrompt = accessControlPrompt;

            Region = region;
            UseSteam = useSteam;
            Executable = executable;
            ExecutableArgs = executableArgs;
            RunAsAdmin = runAsAdmin;
            WindowType = windowType;
            Display = display;
            Resolution = resolution;
            Position = position;
            UIScale = uiScale;
            Mipmapping = mipmapping;
            BumpMapping = bumpMapping;
            MapCompression = mapCompression;
            Gamma = gamma;
            DriverStability = driverStability;
            PlayIntro = playIntro;
            Debug = debug;
            DeveloperMode = developerMode;
            SettingsPath = Paths.CollapsePath(settingsPath);
            UserPath = Paths.CollapsePath(userPath);
            TempPath = Paths.CollapsePath(tempPath);
        }

        public string Name => name ?? string.Empty;

        public Region? Region { get; }

        public bool UseSteam { get; }

        public string Executable { get; }

        public string ExecutableArgs { get; }

        public bool RunAsAdmin { get; }

        public WindowType WindowType { get; }

        public string Display { get; }

        public Dimension? Resolution { get; }

        public Point? Position { get; }

        public float SamplesPerPixel => samplesPerPixel ?? 1f;

        public float? UIScale { get; }

        public bool HardwareMouse => hardwareMouse ?? true;

        public int MaxSounds => maxSounds ?? 32;

        public bool PlaySoundWhenUnfocused => playSoundWhenUnfocused ?? true;

        public int Mipmapping { get; }

        public bool BumpMapping { get; }

        public bool MapCompression { get; }

        public TextureCompression TextureCompression => textureCompression ?? TextureCompression.Uncompressed;

        public EnvironmentAnimation EnvironmentAnimation => environmentAnimation ?? EnvironmentAnimation.Smooth;

        public FontType FontType => fontType ?? FontType.Uncompressed;

        public float? Gamma { get; }

        public bool DriverStability { get; }

        public bool PlayIntro { get; }

        public bool Debug { get; }

        public bool DeveloperMode { get; }

        public string SettingsPath { get; }

        public string UserPath { get; }

        public string TempPath { get; }

        public bool AccessControlPrompt => accessControlPrompt ?? true;

        [SuppressMessage("Microsoft.Design", "CA1006")]
        [SuppressMessage("Microsoft.Design", "CA1026")]
        [SuppressMessage("Microsoft.Maintainability", "CA1502")]
        [SuppressMessage("Microsoft.Naming", "CA1709")]
        public Profile With(
            Maybe<string> Name = new Maybe<string>(),
            Maybe<Region?> Region = new Maybe<Region?>(),
            Maybe<bool> UseSteam = new Maybe<bool>(),
            Maybe<string> Executable = new Maybe<string>(),
            Maybe<string> ExecutableArgs = new Maybe<string>(),
            Maybe<bool> RunAsAdmin = new Maybe<bool>(),
            Maybe<WindowType> WindowType = new Maybe<WindowType>(),
            Maybe<string> Display = new Maybe<string>(),
            Maybe<Dimension?> Resolution = new Maybe<Dimension?>(),
            Maybe<Point?> Position = new Maybe<Point?>(),
            Maybe<float> SamplesPerPixel = new Maybe<float>(),
            Maybe<float?> UIScale = new Maybe<float?>(),
            Maybe<bool> HardwareMouse = new Maybe<bool>(),
            Maybe<int> MaxSounds = new Maybe<int>(),
            Maybe<bool> PlaySoundWhenUnfocused = new Maybe<bool>(),
            Maybe<int> Mipmapping = new Maybe<int>(),
            Maybe<bool> BumpMapping = new Maybe<bool>(),
            Maybe<bool> MapCompression = new Maybe<bool>(),
            Maybe<TextureCompression> TextureCompression = new Maybe<TextureCompression>(),
            Maybe<EnvironmentAnimation> EnvironmentAnimation = new Maybe<EnvironmentAnimation>(),
            Maybe<FontType> FontType = new Maybe<FontType>(),
            Maybe<float?> Gamma = new Maybe<float?>(),
            Maybe<bool> DriverStability = new Maybe<bool>(),
            Maybe<bool> PlayIntro = new Maybe<bool>(),
            Maybe<bool> Debug = new Maybe<bool>(),
            Maybe<bool> DeveloperMode = new Maybe<bool>(),
            Maybe<string> SettingsPath = new Maybe<string>(),
            Maybe<string> UserPath = new Maybe<string>(),
            Maybe<string> TempPath = new Maybe<string>(),
            Maybe<bool> AccessControlPrompt = new Maybe<bool>())
        {
            if (Name != this.Name || Region != this.Region || UseSteam != this.UseSteam || Executable != this.Executable ||
                ExecutableArgs != this.ExecutableArgs || RunAsAdmin != this.RunAsAdmin || WindowType != this.WindowType ||
                Display != this.Display || Resolution != this.Resolution || Position != this.Position ||
                SamplesPerPixel != this.SamplesPerPixel || UIScale != this.UIScale || HardwareMouse != this.HardwareMouse ||
                MaxSounds != this.MaxSounds || PlaySoundWhenUnfocused != this.PlaySoundWhenUnfocused ||
                Mipmapping != this.Mipmapping || BumpMapping != this.BumpMapping || MapCompression != this.MapCompression ||
                TextureCompression != this.TextureCompression || EnvironmentAnimation != this.EnvironmentAnimation ||
                FontType != this.FontType || Gamma != this.Gamma || DriverStability != this.DriverStability ||
                PlayIntro != this.PlayIntro || Debug != this.Debug || DeveloperMode != this.DeveloperMode ||
                SettingsPath != this.SettingsPath || UserPath != this.UserPath || TempPath != this.TempPath ||
                AccessControlPrompt != this.AccessControlPrompt)
            {
                return new Profile(
                    Name.Default(this.Name),
                    Region.Default(this.Region),
                    UseSteam.Default(this.UseSteam),
                    Executable.Default(this.Executable),
                    ExecutableArgs.Default(this.ExecutableArgs),
                    RunAsAdmin.Default(this.RunAsAdmin),
                    WindowType.Default(this.WindowType),
                    Display.Default(this.Display),
                    Resolution.Default(this.Resolution),
                    Position.Default(this.Position),
                    SamplesPerPixel.Default(this.SamplesPerPixel),
                    UIScale.Default(this.UIScale),
                    HardwareMouse.Default(this.HardwareMouse),
                    MaxSounds.Default(this.MaxSounds),
                    PlaySoundWhenUnfocused.Default(this.PlaySoundWhenUnfocused),
                    Mipmapping.Default(this.Mipmapping),
                    BumpMapping.Default(this.BumpMapping),
                    MapCompression.Default(this.MapCompression),
                    TextureCompression.Default(this.TextureCompression),
                    EnvironmentAnimation.Default(this.EnvironmentAnimation),
                    FontType.Default(this.FontType),
                    Gamma.Default(this.Gamma),
                    DriverStability.Default(this.DriverStability),
                    PlayIntro.Default(this.PlayIntro),
                    Debug.Default(this.Debug),
                    DeveloperMode.Default(this.Debug),
                    SettingsPath.Default(this.SettingsPath),
                    UserPath.Default(this.UserPath),
                    TempPath.Default(this.TempPath),
                    AccessControlPrompt.Default(this.AccessControlPrompt));
            }

            return this;
        }

        [SuppressMessage("Microsoft.Design", "CA1006")]
        public IEnumerable<KeyValuePair<string, object>> Settings
        {
            get
            {
                yield return Pair("window_type", WindowType);
                yield return Pair("display_device_name", Display);
                yield return Pair("width", Resolution?.Width);
                yield return Pair("height", Resolution?.Height);
                yield return Pair("x_position", Position?.X);
                yield return Pair("y_position", Position?.Y);
                yield return Pair("samples_per_pixel", SamplesPerPixel);
                yield return Pair("ui_scale", UIScale);
                yield return Pair("hardware_mouse", HardwareMouse);
                yield return Pair("max_sounds", MaxSounds);
                yield return Pair("play_sound_when_unfocused", PlaySoundWhenUnfocused);
                yield return Pair("mipmapping", Mipmapping);
                yield return Pair("bump_mapping", BumpMapping);
                yield return Pair("map_compression", MapCompression);
                yield return Pair("texture_compression", TextureCompression);
                yield return Pair("environment_animation", EnvironmentAnimation);
                yield return Pair("font_type", FontType);
                yield return Pair("gamma", Gamma);
                yield return Pair("driver_stability", DriverStability);
                yield return Pair("play_intro", PlayIntro);
                yield return Pair("debug", Debug);
                yield return Pair("developer_mode", DeveloperMode);
                yield return Pair("settings_path", Paths.ExpandPath(SettingsPath ?? Paths.GlobalSettingsPath));
                yield return Pair("user_path", Paths.ExpandPath(UserPath ?? Paths.GlobalUserPath));
                yield return Pair("temp_path", Paths.ExpandPath(TempPath ?? Paths.GlobalTempPath));
                yield return Pair("command_line_args", With(UseSteam: false).ArgString);
                yield return Pair("verbose_logging", true);
            }
        }

        [SuppressMessage("Microsoft.Maintainability", "CA1502")]
        public string ArgString
        {
            get
            {
                var builder = new StringBuilder("launch");
                AddOption(builder, p => p.Region, "region");
                AddOption(builder, p => p.UseSteam, "steam");
                AddOption(builder, p => p.Executable, "executable");
                AddOption(builder, p => p.ExecutableArgs, "executable-args");
                AddOption(builder, p => p.RunAsAdmin, "admin");
                AddOption(builder, p => p.WindowType, "mode");
                AddOption(builder, p => p.Display, "display");
                AddOption(builder, p => p.Resolution, "res", v => v?.Width + "x" + v?.Height);
                AddOption(builder, p => p.Position, "pos", v => v?.X + "," + v?.Y);
                AddOption(builder, p => p.SamplesPerPixel, "ssaa");
                AddOption(builder, p => p.UIScale, "ui");
                AddOption(builder, p => p.HardwareMouse, "hw-mouse");
                AddOption(builder, p => p.MaxSounds, "sounds");
                AddOption(builder, p => p.PlaySoundWhenUnfocused, "play-if-focus-lost");
                AddOption(builder, p => p.Mipmapping, "mipmapping");
                AddOption(builder, p => p.BumpMapping, "bump-mapping");
                AddOption(builder, p => p.MapCompression, "map-comp");
                AddOption(builder, p => p.TextureCompression, "texture-comp");
                AddOption(builder, p => p.EnvironmentAnimation, "env-anim");
                AddOption(builder, p => p.Gamma, "gamma");
                AddOption(builder, p => p.DriverStability, "driver-stability");
                AddOption(builder, p => p.PlayIntro, "intro");
                AddOption(builder, p => p.Debug, "debug");
                AddOption(builder, p => p.DeveloperMode, "dev");
                AddOption(builder, p => p.SettingsPath, "settings-path");
                AddOption(builder, p => p.UserPath, "user-path");
                AddOption(builder, p => p.TempPath, "temp-path");
                return builder.ToString();
            }
        }

        public static bool operator ==(Profile left, Profile right) =>
            string.Equals(left.Name, right.Name, StringComparison.CurrentCultureIgnoreCase);

        public static bool operator !=(Profile left, Profile right) => !(left == right);

        public bool Equals(Profile other) => this == other;

        public override bool Equals(object obj) => obj is Profile other && Equals(other);

        public override int GetHashCode() => Name.GetHashCode();

        private static KeyValuePair<string, object> Pair(string key, object value) => new KeyValuePair<string, object>(key, value);

        [SuppressMessage("Microsoft.Globalization", "CA1308")]
        private void AddOption<T>(StringBuilder builder, Func<Profile, T> getter, string arg) =>
            AddOption(builder, getter, arg, v => v?.ToString().ToLowerInvariant() ?? "auto");

        private void AddOption<T>(StringBuilder builder, Func<Profile, T> getter, string arg,
            Func<T, string> formatter)
        {
            var value = getter(this);
            if (!Equals(value, getter(Default)))
            {
                builder.Append(" --").Append(arg);
                if (typeof(T) != typeof(bool))
                {
                    builder.Append(" ").Append(formatter(value));
                }
            }
        }
    }
}
