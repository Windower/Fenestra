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
    using System;
    using System.Runtime.InteropServices;
    using System.Windows;
    using System.Windows.Interop;
    using System.Windows.Media.Imaging;

    public static class StockIcons
    {
        private static Lazy<BitmapSource> shield = new Lazy<BitmapSource>(() => GetStockIcon(NativeMethods.SIID_SHIELD));

        public static BitmapSource Shield => shield.Value;

        private static BitmapSource GetStockIcon(int id)
        {
            var info = default(NativeMethods.SHSTOCKICONINFO);
            info.cbSize = (uint)Marshal.SizeOf(typeof(NativeMethods.SHSTOCKICONINFO));
            try
            {
                Marshal.ThrowExceptionForHR(NativeMethods.SHGetStockIconInfo(id,
                    NativeMethods.SHGSI_ICON | NativeMethods.SHGSI_SMALLICON, ref info));
                using (new SafeIconHandle(info.hIcon))
                {
                    return Imaging.CreateBitmapSourceFromHIcon(info.hIcon, Int32Rect.Empty, BitmapSizeOptions.FromEmptyOptions());
                }
            }
            catch (DllNotFoundException) { }
            catch (EntryPointNotFoundException) { }
            return null;
        }
    }
}
