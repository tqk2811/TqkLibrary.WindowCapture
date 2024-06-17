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
            "maplestory"
        };
        Process? GetProcess()
        {
            Process? process = null;
            for (int i = 0; i < names.Count() && process is null; i++)
            {
                var processs = Process.GetProcessesByName(names.Skip(i).First());
                process = processs.FirstOrDefault(x => x.MainWindowHandle != IntPtr.Zero);
            }
            return process;
        }



        bool lastVisible;
        TimeSpan lastRender;

        readonly IntPtr windowHandle;
        readonly BaseCapture baseCapture;
        readonly RenderCapture renderCapture;
        public MainWindow()
        {
            Process? process = GetProcess();
            if (process is null) throw new Exception();

            windowHandle = process.MainWindowHandle;

            //using Bitmap? bitmap = windowHandle.Capture(TqkLibrary.WinApi.Enums.CaptureType.PrintWindow);
            //bitmap?.Save("D:\\test.png");



            baseCapture = new BitbltCapture();


            if (!baseCapture.Init(windowHandle))
                throw new Exception();

            renderCapture = new RenderCapture(baseCapture);

            //using Bitmap bitmap = baseCapture.Shoot();
            //bitmap?.Save("D:\\test.png");






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

            if (lastVisible != isVisible)
            {
                lastVisible = isVisible;
                if (lastVisible)
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
            if (this.lastRender != args.RenderingTime)
            {
                InteropImage.RequestRender();
                this.lastRender = args.RenderingTime;
            }
        }

        void OnRender(IntPtr surface, bool isNewSurface)
        {
            bool isNewtargetView = false;
            renderCapture.Draw(surface, isNewSurface, ref isNewtargetView);
            if (isNewtargetView)
            {
                _SizeChanged();
            }
        }
    }
}