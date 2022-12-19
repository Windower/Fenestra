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
    using System.Collections.Immutable;
    using System.ComponentModel;
    using System.Diagnostics.CodeAnalysis;
    using System.Globalization;
    using System.IO;
    using System.Linq;
    using System.Xml;
    using System.Xml.Linq;

    public class ProfileManager
    {
        private readonly string file;

        private IImmutableSet<Profile> profiles = ImmutableSortedSet<Profile>.Empty
            .WithComparer(Comparer<Profile>.Create((a, b) => string.Compare(a.Name, b.Name,
                StringComparison.CurrentCultureIgnoreCase)));

        public ProfileManager() :
            this(Paths.ExpandPath(Path.Combine(Paths.GlobalSettingsPath, "profiles.xml")))
        {
        }

        public ProfileManager(string file)
        {
            this.file = file;
            try
            {
                var document = XDocument.Load(file);
                Profiles = document.Root.Elements("profile").Select(DeserializeProfile).Aggregate(Profiles, (a, b) => a.Add(b));
            }
            catch (XmlException) { }
            catch (DirectoryNotFoundException) { }
            catch (FileNotFoundException) { }
        }

        public event EventHandler ProfilesChanged;

        public IImmutableSet<Profile> Profiles
        {
            get => profiles;
            private set
            {
                profiles = value;
                ProfilesChanged?.Invoke(this, new EventArgs());
            }
        }

        public Profile this[string name]
        {
            get
            {
                if (name == null)
                {
                    throw new ArgumentNullException(nameof(name));
                }

                if (TryGetValue(name, out Profile result))
                {
                    return result;
                }
                throw new KeyNotFoundException(string.Format(CultureInfo.InvariantCulture,
                    "No profile with name \"{0}\" was found.", name.Trim()));
            }
        }

        public bool Contains(string name)
        {
            if (name == null)
            {
                throw new ArgumentNullException(nameof(name));
            }

            return Profiles.Contains(Profile.Default.With(Name: name));
        }

        public bool TryGetValue(string name, out Profile result)
        {
            if (name == null)
            {
                throw new ArgumentNullException(nameof(name));
            }

            return Profiles.TryGetValue(Profile.Default.With(Name: name), out result);
        }

        public void Add(Profile profile)
        {
            var temp = Profiles.Add(profile);
            if (temp == Profiles)
            {
                throw new ArgumentException(string.Format(CultureInfo.InvariantCulture,
                    "A profile with name \"{0}\" already exists.", profile.Name), nameof(profile));
            }
            Profiles = temp;
        }

        public bool Remove(Profile profile)
        {
            var temp = Profiles.Remove(profile);
            var removed = temp != Profiles;
            Profiles = temp;
            return removed;
        }

        internal void Save()
        {
            var document =
                new XDocument(
                    new XDeclaration("1.0", "utf-8", "yes"),
                    new XElement("profiles",
                        from p in Profiles
                        select new XElement("profile", Serialize(p))));

            Directory.CreateDirectory(Path.GetDirectoryName(file));
            document.Save(file);
        }

        private static IEnumerable<XObject> Serialize(Profile p)
        {
            yield return new XAttribute("name", p.Name);
            yield return new XElement("region", ToLowerString(p.Region) ?? "auto");
            yield return new XElement("use-steam", p.UseSteam);
            if (p.Executable != null)
            {
                yield return new XElement("executable", p.Executable);
            }
            if (p.ExecutableArgs != null)
            {
                yield return new XElement("executable-args", p.ExecutableArgs);
            }
            if (p.RunAsAdmin)
            {
                yield return new XElement("run-as-admin", p.RunAsAdmin);
            }
            yield return new XElement("window-type", ToLowerString(p.WindowType));
            yield return new XElement("display", p.Display);
            yield return new XElement("resolution", Serialize(p.Resolution));
            if (p.WindowType != WindowType.FullScreen)
            {
                yield return new XElement("position", Serialize(p.Position));
            }
            yield return new XElement("samples-per-pixel", p.SamplesPerPixel);
            yield return new XElement("ui-scale", p.UIScale?.ToString() ?? "auto");
            yield return new XElement("hardware-mouse", p.HardwareMouse);
            yield return new XElement("max-sounds", p.MaxSounds);
            yield return new XElement("play-sound-when-unfocused", p.PlaySoundWhenUnfocused);
            yield return new XElement("mipmapping", p.Mipmapping);
            yield return new XElement("bump-mapping", p.BumpMapping);
            yield return new XElement("map-compression", ToLowerString(p.MapCompression));
            yield return new XElement("texture-compression", ToLowerString(p.TextureCompression));
            yield return new XElement("environment-animation", ToLowerString(p.EnvironmentAnimation));
            yield return new XElement("font-type", ToLowerString(p.FontType));
            yield return new XElement("gamma", p.Gamma?.ToString() ?? "auto");
            yield return new XElement("driver-stability", p.DriverStability);
            yield return new XElement("play-intro", p.PlayIntro);
            if (p.Debug)
            {
                yield return new XElement("debug", p.Debug);
            }
            if (p.DeveloperMode)
            {
                yield return new XElement("developer-mode", p.DeveloperMode);
            }
            if (p.SettingsPath != null)
            {
                yield return new XElement("settings-path", p.SettingsPath);
            }
            if (p.UserPath != null)
            {
                yield return new XElement("user-path", p.UserPath);
            }
            if (p.TempPath != null)
            {
                yield return new XElement("temp-path", p.TempPath);
            }
            if (!p.AccessControlPrompt)
            {
                yield return new XElement("access-control-prompt", p.AccessControlPrompt);
            }
        }

        private static Profile DeserializeProfile(XElement e)
        {
            var builder = new ProfileBuilder
            {
                Name = (string)e.Attribute("name"),
                Region = Convert<Region>(e.Element("region")),
                UseSteam = Convert(e.Element("use-steam"), false),
                Executable = ConvertString(e.Element("executable")),
                ExecutableArgs = ConvertString(e.Element("executable-args")),
                RunAsAdmin = Convert(e.Element("run-as-admin"), false),
                WindowType = Convert(e.Element("window-type"), WindowType.Borderless),
                Display = ConvertString(e.Element("display")),
                Resolution = DeserializeDimension(e.Element("resolution")),
                Position = DeserializePoint(e.Element("position")),
                Gamma = Convert<float>(e.Element("gamma")),
                SamplesPerPixel = Convert(e.Element("samples-per-pixel"), 1f),
                UIScale = Convert<float>(e.Element("ui-scale")),
                HardwareMouse = Convert(e.Element("hardware-mouse"), true),
                MaxSounds = Convert(e.Element("max-sounds"), 32),
                PlaySoundWhenUnfocused = Convert(e.Element("play-sound-when-unfocused"), true),
                Mipmapping = Convert(e.Element("mipmapping"), 0),
                BumpMapping = Convert(e.Element("bump-mapping"), false),
                MapCompression = Convert(e.Element("map-compression"), false),
                TextureCompression = Convert(e.Element("texture-compression"), TextureCompression.Uncompressed),
                EnvironmentAnimation = Convert(e.Element("environment-animation"), EnvironmentAnimation.Smooth),
                FontType = Convert(e.Element("font-type"), FontType.Uncompressed),
                DriverStability = Convert(e.Element("driver-stability"), false),
                PlayIntro = Convert(e.Element("play-intro"), false),
                Debug = Convert(e.Element("debug"), false),
                DeveloperMode = Convert(e.Element("developer-mode"), false),
                SettingsPath = ConvertString(e.Element("settings-path")),
                UserPath = ConvertString(e.Element("user-path")),
                TempPath = ConvertString(e.Element("temp-path")),
                AccessControlPrompt = Convert(e.Element("access-control-prompt"), true),
            };
            return builder.Get();
        }

        private static IEnumerable<XObject> Serialize(Dimension? d)
        {
            if (d == null)
            {
                yield return new XText("auto");
            }
            else
            {
                yield return new XElement("width", ((Dimension)d).Width);
                yield return new XElement("height", ((Dimension)d).Height);
            }
        }

        private static Dimension? DeserializeDimension(XElement e)
        {
            if (e == null || (string)e == "auto")
            {
                return null;
            }

            var builder = new DimensionBuilder
            {
                Width = Convert(e.Element("width"), 1),
                Height = Convert(e.Element("height"), 1)
            };
            return builder.Get();
        }

        private static IEnumerable<XObject> Serialize(Point? d)
        {
            if (d == null)
            {
                yield return new XText("auto");
            }
            else
            {
                yield return new XElement("x", ((Point)d).X);
                yield return new XElement("y", ((Point)d).Y);
            }
        }

        private static Point? DeserializePoint(XElement e)
        {
            if (e == null || (string)e == "auto")
            {
                return null;
            }

            var builder = new PointBuilder
            {
                X = Convert(e.Element("x"), 0),
                Y = Convert(e.Element("y"), 0)
            };
            return builder.Get();
        }

        [SuppressMessage("Microsoft.Globalization", "CA1308")]
        private static string ToLowerString<T>(T value) => value?.ToString().ToLowerInvariant();

        private static string ConvertString(XElement e)
        {
            var str = (string)e;
            return string.IsNullOrWhiteSpace(str) ? null : str;
        }

        private static T Convert<T>(XElement e, T defaultValue) where T : struct => Convert<T>(e, null) ?? defaultValue;

        private static T? Convert<T>(XElement e, string auto = "auto") where T : struct
        {
            var str = (string)e;
            if (string.IsNullOrWhiteSpace(str) || str == auto)
            {
                return null;
            }
            else if (typeof(T).IsEnum)
            {
                return Enum.TryParse(str, true, out T result) ? result : (T?)null;
            }
            else
            {
                var converter = TypeDescriptor.GetConverter(typeof(T));
                return converter.IsValid(str) ? (T?)converter.ConvertFromInvariantString(str) : null;
            }
        }
    }
}
