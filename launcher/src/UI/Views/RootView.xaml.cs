namespace Windower.UI.Views
{
    using System;
    using System.ComponentModel;
    using System.Runtime.InteropServices;
    using System.Windows;
    using System.Windows.Interop;
    using System.Windows.Media;

    /// <summary>
    /// Interaction logic for RootView.xaml
    /// </summary>
    public partial class RootView : Window
    {
        public RootView() => InitializeComponent();

        protected override void OnSourceInitialized(EventArgs e)
        {
            Background = Brushes.Transparent;
            var source = HwndSource.FromHwnd(new WindowInteropHelper(this).Handle);
            source.CompositionTarget.BackgroundColor = Colors.Transparent;
            source.AddHook(WndProc);
            var margins = default(NativeMethods.MARGINS);
            margins.cxLeftWidth = 1;
            margins.cxRightWidth = 1;
            margins.cyTopHeight = 1;
            margins.cyBottomHeight = 1;
            try
            {
                Marshal.ThrowExceptionForHR(NativeMethods.DwmExtendFrameIntoClientArea(source.Handle, ref margins));
            }
            catch (DllNotFoundException) { }
            catch (EntryPointNotFoundException) { }
            base.OnSourceInitialized(e);
        }

        private IntPtr WndProc(IntPtr hwnd, int msg, IntPtr wParam, IntPtr lParam, ref bool handled)
        {
            if (msg == 0x0083) // WM_NCCALCSIZE
            {
                handled = true;
            }
            return IntPtr.Zero;
        }

        private void HandleClosing(object sender, CancelEventArgs e)
        {
            if (((dynamic)DataContext)?.Interruptible == false)
            {
                e.Cancel = true;
            }
        }

        private void HandleCloseButtonClick(object sender, RoutedEventArgs e) => Close();

        private void HandleMinimizeButtonClick(object sender, RoutedEventArgs e) => WindowState = WindowState.Minimized;

        private void HandleWindowDrag(object sender, System.Windows.Input.MouseButtonEventArgs e) => DragMove();
    }
}
