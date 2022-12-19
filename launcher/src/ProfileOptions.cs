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

namespace Windower
{
    using CommandLine;
    using System.Collections.Generic;
    using System.Linq;
    using Core;
    using PlayOnline;
    using System.CodeDom.Compiler;

    public abstract class ProfileOptions
    {
        private ProfileBuilder builder = new ProfileBuilder();

        [Option("region")]
        public RegionInternal Region
        {
            get => builder.Region == null ? RegionInternal.auto : (RegionInternal)builder.Region;
            set => builder.Region = value == RegionInternal.auto ? null : (Region?)value;
        }

        [Option("steam")]
        public bool UseSteam
        {
            get => builder.UseSteam;
            set => builder.UseSteam = value;
        }

        [Option("mode")]
        public WindowTypeInternal WindowType
        {
            get => (WindowTypeInternal)builder.WindowType;
            set => builder.WindowType = (WindowType)value;
        }

        [Option("executable")]
        public string Executable
        {
            get => builder.Executable;
            set => builder.Executable = value;
        }

        [Option("executable-args")]
        public string ExecutableArgs
        {
            get => builder.ExecutableArgs;
            set => builder.ExecutableArgs = value;
        }

        [Option("admin")]
        public bool RunAsAdmin
        {
            get => builder.RunAsAdmin;
            set => builder.RunAsAdmin = value;
        }

        [Option("display")]
        public string Display
        {
            get => builder.Display;
            set => builder.Display = value;
        }

        [Option("res", Min = 2, Max = 2, Separator = 'x')]
        public IEnumerable<int> Resolution
        {
            get
            {
                var resolution = builder.Resolution;
                if (resolution != null)
                {
                    yield return ((Dimension)resolution).Width;
                    yield return ((Dimension)resolution).Height;
                }
            }

            set
            {
                var array = value?.ToArray();
                if (array != null && array.Length != 0)
                {
                    builder.Resolution = new Dimension(array[0], array[1]);
                }
            }
        }

        [Option("pos", Min = 2, Max = 2, Separator = ',')]
        public IEnumerable<int> Position
        {
            get
            {
                var resolution = builder.Resolution;
                if (resolution != null)
                {
                    yield return ((Dimension)resolution).Width;
                    yield return ((Dimension)resolution).Height;
                }
            }

            set
            {
                var array = value?.ToArray();
                if (array != null && array.Length != 0)
                {
                    builder.Resolution = new Dimension(array[0], array[1]);
                }
            }
        }

        [Option("ssaa")]
        public float SamplesPerPixel
        {
            get => builder.SamplesPerPixel;
            set => builder.SamplesPerPixel = value;
        }

        [Option("ui")]
        public float? UIScale
        {
            get => builder.UIScale;
            set => builder.UIScale = value;
        }

        [Option("hw-mouse")]
        public bool HardwareMouse
        {
            get => builder.HardwareMouse;
            set => builder.HardwareMouse = value;
        }

        [Option("sounds")]
        public int MaxSounds
        {
            get => builder.MaxSounds;
            set => builder.MaxSounds = value;
        }

        [Option("play-if-focus-lost")]
        public bool PlaySoundWhenUnfocused
        {
            get => builder.PlaySoundWhenUnfocused;
            set => builder.PlaySoundWhenUnfocused = value;
        }

        [Option("mipmapping")]
        public int Mipmapping
        {
            get => builder.Mipmapping;
            set => builder.Mipmapping = value;
        }

        [Option("bump-mapping")]
        public bool BumpMapping
        {
            get => builder.BumpMapping;
            set => builder.BumpMapping = value;
        }

        [Option("map-comp")]
        public bool MapCompression
        {
            get => builder.MapCompression;
            set => builder.MapCompression = value;
        }

        [Option("texture-comp")]
        public TextureCompression TextureCompression
        {
            get => builder.TextureCompression;
            set => builder.TextureCompression = value;
        }

        [Option("env-anim")]
        public EnvironmentAnimation EnvironmentAnimation
        {
            get => builder.EnvironmentAnimation;
            set => builder.EnvironmentAnimation = value;
        }

        [Option("font")]
        public FontType FontType
        {
            get => builder.FontType;
            set => builder.FontType = value;
        }

        [Option("gamma")]
        public float? Gamma
        {
            get => builder.Gamma;
            set => builder.Gamma = value;
        }

        [Option("driver-stability")]
        public bool DriverStability
        {
            get => builder.DriverStability;
            set => builder.DriverStability = value;
        }

        [Option("intro")]
        public bool PlayIntro
        {
            get => builder.PlayIntro;
            set => builder.PlayIntro = value;
        }

        [Option("debug")]
        public bool Debug
        {
            get => builder.Debug;
            set => builder.Debug = value;
        }

        [Option("dev")]
        public bool DeveloperMode
        {
            get => builder.DeveloperMode;
            set => builder.DeveloperMode = value;
        }

        [Option("settings-path")]
        public string SettingsPath
        {
            get => builder.SettingsPath;
            set => builder.SettingsPath = value;
        }

        [Option("user-path")]
        public string UserPath
        {
            get => builder.UserPath;
            set => builder.UserPath = value;
        }

        [Option("temp-path")]
        public string TempPath
        {
            get => builder.TempPath;
            set => builder.TempPath = value;
        }

        protected Profile GetProfile(Profile baseProfile) => builder.Get(baseProfile);

        // HACK: Marked as generated code to shut up Code Analysis.
        [GeneratedCode("", "")]
        public enum RegionInternal
        {
            auto = -1,
            na = PlayOnline.Region.NA,
            jp = PlayOnline.Region.JP,
            eu = PlayOnline.Region.EU,
        }

        // HACK: Marked as generated code to shut up Code Analysis.
        [GeneratedCode("", "")]
        public enum WindowTypeInternal
        {
            borderless = Core.WindowType.Borderless,
            window = Core.WindowType.Window,
            fullscreen = Core.WindowType.FullScreen,
        }
    }
}
