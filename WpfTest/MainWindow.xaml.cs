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

namespace WpfTest
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        readonly IEnumerable<string> names = new List<string>()
        {
            "notepad",
            "chrome",//hardware-accelerated graphics context (OpenGL- or DirectX-based) not work for Bitblt
        };
        IntPtr GetHandler()
        {
            for (int i = 0; i < names.Count(); i++)
            {
                var processHelpers = Process.GetProcessesByName(names.Skip(i).First()).Select(x => new ProcessHelper((uint)x.Id));
                foreach (ProcessHelper processHelper in processHelpers)
                {
                    foreach(WindowHelper windowHelper in processHelper.AllWindows)
                    {
                        if(windowHelper.IsWindow && !string.IsNullOrWhiteSpace(windowHelper.Title))
                        {
                            return windowHelper.WindowHandle;
                        }
                    }
                }
            }
            return IntPtr.Zero;
        }



        bool _lastVisible;
        TimeSpan _lastRender;
        readonly Stopwatch _stopwatch = new Stopwatch();
        readonly IntPtr _windowHandle;
        readonly BaseCapture _baseCapture;
        readonly RenderCapture _renderCapture;
        DateTime _dateTime = DateTime.Now;
        int fpsCount = 0;
        public MainWindow()
        {
            _windowHandle = GetHandler();

            _baseCapture = new BitbltCapture();

            if (!_baseCapture.Init(_windowHandle))
                throw new Exception();

            _renderCapture = new RenderCapture(_baseCapture);

            InitializeComponent();
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            InteropImage.WindowOwner = new WindowInteropHelper(this).Handle;
            InteropImage.OnRender = OnRender;
            InteropImage.RequestRender();
        }

        //start first then Window_Loaded start second
        private void Window_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            _SizeChanged();
        }


        void _SizeChanged()
        {
            double dpiScale = 1.0;

            int surfWidth = (int)(this.ActualWidth < 0 ? 0 : Math.Ceiling(this.ActualWidth * dpiScale));
            int surfHeight = (int)(this.ActualHeight < 0 ? 0 : Math.Ceiling(this.ActualHeight * dpiScale));

            InteropImage.SetPixelSize(surfWidth, surfHeight);

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
            if (_renderCapture.Draw(surface, isNewSurface, ref isNewtargetView))
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
    }
}