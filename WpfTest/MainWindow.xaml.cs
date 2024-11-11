using System.Diagnostics;
using System.Drawing;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using TqkLibrary.WinApi;
using TqkLibrary.WinApi.FindWindowHelper;
using TqkLibrary.WindowCapture;
using TqkLibrary.WindowCapture.Captures;
using TqkLibrary.Wpf.Interop.DirectX;
using TqkLibrary.WpfUi;

namespace WpfTest
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {

        readonly MainWVM _mainWVM;
        public MainWindow()
        {
            InitializeComponent();
            this._mainWVM = this.DataContext as MainWVM ?? throw new InvalidOperationException();
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            InteropImage.WindowOwner = new WindowInteropHelper(this).Handle;
            InteropImage.OnRender = OnRender;
            InteropImage.RequestRender();


            this._mainWVM.RenderSelected = CaptureType.WinrtGraphicCapture;
            this._mainWVM.CaptureImageSelected = CaptureType.WinrtGraphicCapture;
        }

        //start first then Window_Loaded start second
        private void Window_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            _SizeChanged();
        }


        void _SizeChanged()
        {
            double dpiScale = 1.0;

            double surfWidth = this.ActualWidth < 0 ? 0 : Math.Ceiling(this.ActualWidth * dpiScale);
            double surfHeight = this.ActualHeight < 0 ? 0 : Math.Ceiling(this.ActualHeight * dpiScale);

            if (_baseCapture_Render is not null)
            {
                System.Drawing.Size videoSize = _baseCapture_Render.Size;
                if (!videoSize.IsEmpty && surfWidth != 0 && surfHeight != 0)
                {
                    //recalc surfWidth, surfHeight

                    rateVideoAndDraw = Math.Min(surfWidth / videoSize.Width, surfHeight / videoSize.Height);
                    surfWidth = videoSize.Width * rateVideoAndDraw;
                    surfHeight = videoSize.Height * rateVideoAndDraw;
                }
                Debug.WriteLine($"Raw: {videoSize}, rate: {rateVideoAndDraw}; Surface: {surfWidth}x{surfHeight}");
            }

            InteropImage.SetPixelSize((int)surfWidth, (int)surfHeight);

            bool isVisible = (surfWidth != 0 && surfHeight != 0);

            if (_lastVisible != isVisible)
            {
                _lastVisible = isVisible;
                if (_lastVisible)
                {
                    CompositionTarget.Rendering += CompositionTarget_Rendering;
                }
                else
                {
                    CompositionTarget.Rendering -= CompositionTarget_Rendering;
                }
            }
        }

        private void CompositionTarget_Rendering(object? sender, EventArgs e)
        {
            RenderingEventArgs args = (RenderingEventArgs)e;
            if (this._lastRender != args.RenderingTime)
            {
                InteropImage.RequestRender();
                this._lastRender = args.RenderingTime;
            }
        }

        readonly Stopwatch _stopwatch = new Stopwatch();
        readonly IntPtr _windowHandle;
        bool _lastVisible;
        TimeSpan _lastRender;
        DateTime _dateTime = DateTime.Now;
        int fpsCount = 0;
        double rateVideoAndDraw;
        void OnRender(IntPtr surface, bool isNewSurface)
        {
            if (DateTime.Now - _dateTime >= TimeSpan.FromSeconds(1))
            {
                tb_fps.Text = $"{fpsCount:000} fps";
                _dateTime = DateTime.Now;
                fpsCount = 0;
            }

            _stopwatch.Restart();
            bool isNewtargetView = false;
            if (_baseCapture_Render?.Render(surface, isNewSurface, ref isNewtargetView) == true)
            {
                _stopwatch.Stop();
                fpsCount++;
                tb_drawTime.Text = $"{_stopwatch.ElapsedMilliseconds:000} ms";
            }

            if (isNewtargetView)
            {
                _SizeChanged();
            }
        }





        private async void cbb_windows_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (_mainWVM.WindowHelperSelected is not null)
            {
                IntPtr intPtr = _mainWVM.WindowHelperSelected.WindowHandle;
                this.Cursor = Cursors.Wait;

                if (_baseCapture_Render is not null) await _baseCapture_Render.InitAsync(intPtr);
                if (_baseCapture_Shoot is not null) await _baseCapture_Shoot.InitAsync(intPtr);

                this.Cursor = null;
            }
        }
        BaseCapture? _baseCapture_Render;
        private async void cbb_renderMode_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            _baseCapture_Render?.Dispose();
            _baseCapture_Render = null;

            _baseCapture_Render = CreateCapture(_mainWVM.RenderSelected);
            if (_mainWVM.WindowHelperSelected is not null)
            {
                IntPtr intPtr = _mainWVM.WindowHelperSelected.WindowHandle;
                this.Cursor = Cursors.Wait;

                if (_baseCapture_Render is not null) await _baseCapture_Render.InitAsync(intPtr);

                this.Cursor = null;
            }
        }

        BaseCapture? _baseCapture_Shoot;
        private async void cbb_captureMode_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            _baseCapture_Shoot?.Dispose();
            _baseCapture_Shoot = null;

            _baseCapture_Shoot = CreateCapture(_mainWVM.CaptureImageSelected);
            if (_mainWVM.WindowHelperSelected is not null)
            {
                IntPtr intPtr = _mainWVM.WindowHelperSelected.WindowHandle;
                this.Cursor = Cursors.Wait;

                if (_baseCapture_Shoot is not null) await _baseCapture_Shoot.InitAsync(intPtr);

                this.Cursor = null;
            }
        }

        private async void btn_sceenshoot_Click(object sender, RoutedEventArgs e)
        {
            using Bitmap? bitmap = _baseCapture_Shoot is not null ? await _baseCapture_Shoot.CaptureImageAsync() : null;
            if (bitmap is not null)
            {
                _mainWVM.CapturedImage = bitmap.ToBitmapImage();
            }
            else
            {
                _mainWVM.CapturedImage = null;
            }
        }



        BaseCapture? CreateCapture(CaptureType? captureType)
        {
            try
            {
            switch (captureType)
            {
                case CaptureType.HdcCapture_BitBlt:
                    return new HdcCapture() { Mode = HdcCapture.HdcCaptureMode.BitBlt };

                case CaptureType.HdcCapture_PrintWindow:
                    return new HdcCapture() { Mode = HdcCapture.HdcCaptureMode.PrintWindow };

                case CaptureType.WinrtGraphicCapture:
                    return new WinrtGraphicCapture() { MaxFps = 0 };

                    default: return null;
                }
            }
            catch
            {
                return null;
            }
        }
    }
}