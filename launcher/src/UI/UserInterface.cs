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

namespace Windower.UI
{
    using Core;
    using System;
    using System.Globalization;
    using System.IO;
    using System.Threading;
    using System.Windows;

    internal static class UserInterface
    {
        public static void Run() => Run(null);

        public static void Run(Profile? profile) => RunInternal("Main", profile);

        public static void RunCrashReporter(string signature, string crashDumpPath, string stackTrace) =>
            RunInternal("CrashReporter", signature, crashDumpPath, stackTrace);

        private static void RunInternal(string view, params object[] args)
        {
            if (Thread.CurrentThread.GetApartmentState() == ApartmentState.STA)
            {
                var application = new WindowerApplication();
                LoadStringResources(application);

                application.ShutdownMode = ShutdownMode.OnLastWindowClose;
                application.Startup += (s, e) =>
                {
                    var window = (Window)ViewFactory.Create("Root", view, args);
                    application.MainWindow = window;
                    window.Show();
                };
                application.Run();
            }
            else
            {
                var thread = new Thread(() => RunInternal(view, args)) { Name = "UI Thread" };
                thread.SetApartmentState(ApartmentState.STA);
                thread.Start();
                thread.Join();
            }
        }

        private static void LoadStringResources(Application application)
        {
            var culture = CultureInfo.CurrentUICulture;

            var languageTag = culture.TwoLetterISOLanguageName;
            var cultureTag = culture.Name;

            try
            {
                application.Resources.MergedDictionaries.Add(new ResourceDictionary()
                {
                    Source = new Uri("/windower;component/res/Strings." + languageTag + ".xaml", UriKind.Relative)
                });
            }
            catch (IOException) { }

            if (cultureTag != languageTag)
            {
                try
                {
                    application.Resources.MergedDictionaries.Add(new ResourceDictionary()
                    {
                        Source = new Uri("/windower;component/res/Strings." + cultureTag + ".xaml", UriKind.Relative)
                    });
                }
                catch (IOException) { }
            }
        }
    }
}
