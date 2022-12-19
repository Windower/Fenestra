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

    public class ProfileBuilder
    {
        private string name = string.Empty;
        private Maybe<Region?> region;
        private Maybe<bool> useSteam;
        private Maybe<string> executable;
        private Maybe<string> executableArgs;
        private Maybe<bool> runAsAdmin;
        private Maybe<WindowType> windowType;
        private Maybe<string> display;
        private Maybe<Dimension?> resolution;
        private Maybe<Point?> position;
        private Maybe<float> samplesPerPixel;
        private Maybe<float?> uiScale;
        private Maybe<bool> hardwareMouse;
        private Maybe<int> maxSounds;
        private Maybe<bool> playSoundWhenUnfocused;
        private Maybe<int> mipmapping;
        private Maybe<bool> bumpMapping;
        private Maybe<bool> mapCompression;
        private Maybe<TextureCompression> textureCompression;
        private Maybe<EnvironmentAnimation> environmentAnimation;
        private Maybe<FontType> fontType;
        private Maybe<float?> gamma;
        private Maybe<bool> driverStability;
        private Maybe<bool> playIntro;
        private Maybe<bool> debug;
        private Maybe<bool> developerMode;
        private Maybe<string> settingsPath;
        private Maybe<string> userPath;
        private Maybe<string> tempPath;
        private Maybe<bool> accessControlPrompt;

        public string Name
        {
            get => name;
            set
            {
                if (value == null)
                {
                    throw new ArgumentNullException(nameof(value));
                }

                name = value.Trim();
            }
        }

        public Region? Region
        {
            get => region.Default(null);
            set => region = value;
        }

        public bool UseSteam
        {
            get => useSteam.Default(false);
            set => useSteam = value;
        }

        public string Executable
        {
            get => executable.Default(null);
            set => executable = value;
        }

        public string ExecutableArgs
        {
            get => executableArgs.Default(null);
            set => executableArgs = value;
        }

        public bool RunAsAdmin
        {
            get => runAsAdmin.Default(false);
            set => runAsAdmin = value;
        }

        public WindowType WindowType
        {
            get => windowType.Default(WindowType.Borderless);
            set => windowType = value;
        }

        public string Display
        {
            get => display.Default(null);
            set => display = value;
        }

        public Dimension? Resolution
        {
            get => resolution.Default(null);
            set => resolution = value;
        }

        public Point? Position
        {
            get => position.Default(null);
            set => position = value;
        }

        public float SamplesPerPixel
        {
            get => samplesPerPixel.Default(1);
            set => samplesPerPixel = value;
        }

        public float? UIScale
        {
            get => uiScale.Default(null);
            set => uiScale = value;
        }

        public bool HardwareMouse
        {
            get => hardwareMouse.Default(true);
            set => hardwareMouse = value;
        }

        public int MaxSounds
        {
            get => maxSounds.Default(32);
            set => maxSounds = value;
        }

        public bool PlaySoundWhenUnfocused
        {
            get => playSoundWhenUnfocused.Default(true);
            set => playSoundWhenUnfocused = value;
        }

        public int Mipmapping
        {
            get => mipmapping.Default(0);
            set => mipmapping = value;
        }

        public bool BumpMapping
        {
            get => bumpMapping.Default(false);
            set => bumpMapping = value;
        }

        public bool MapCompression
        {
            get => mapCompression.Default(false);
            set => mapCompression = value;
        }

        public TextureCompression TextureCompression
        {
            get => textureCompression.Default(TextureCompression.Uncompressed);
            set => textureCompression = value;
        }

        public EnvironmentAnimation EnvironmentAnimation
        {
            get => environmentAnimation.Default(EnvironmentAnimation.Smooth);
            set => environmentAnimation = value;
        }

        public FontType FontType
        {
            get => fontType.Default(FontType.Uncompressed);
            set => fontType = value;
        }

        public float? Gamma
        {
            get => gamma.Default(0f);
            set => gamma = value;
        }

        public bool DriverStability
        {
            get => driverStability.Default(false);
            set => driverStability = value;
        }

        public bool PlayIntro
        {
            get => playIntro.Default(false);
            set => playIntro = value;
        }

        public bool Debug
        {
            get => debug.Default(false);
            set => debug = value;
        }

        public bool DeveloperMode
        {
            get => developerMode.Default(false);
            set => developerMode = value;
        }

        public string SettingsPath
        {
            get => settingsPath.Default(null);
            set => settingsPath = value;
        }

        public string UserPath
        {
            get => userPath.Default(null);
            set => userPath = value;
        }

        public string TempPath
        {
            get => tempPath.Default(null);
            set => tempPath = value;
        }

        public bool AccessControlPrompt
        {
            get => accessControlPrompt.Default(true);
            set => accessControlPrompt = value;
        }

        public Profile Get() => Get(default(Profile));

        public Profile Get(Profile baseValue)
        {
            if (baseValue == null)
            {
                throw new ArgumentNullException(nameof(baseValue));
            }

            return baseValue.With(name, region, useSteam, executable, executableArgs, runAsAdmin, windowType, display, resolution, position,
                samplesPerPixel, uiScale, hardwareMouse, maxSounds, playSoundWhenUnfocused, mipmapping, bumpMapping, mapCompression,
                textureCompression, environmentAnimation, fontType, gamma, driverStability, playIntro, debug, developerMode, userPath, userPath,
                tempPath, accessControlPrompt);
        }
    }
}
