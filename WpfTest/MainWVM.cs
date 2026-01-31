using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;
using System.Windows.Media;
using TqkLibrary.WinApi.Helpers;
using TqkLibrary.WindowCapture;
using TqkLibrary.WpfUi;

namespace WpfTest
{
    internal class MainWVM : BaseViewModel
    {
        public MainWVM()
        {
            CaptureTypes = Enum.GetValues<CaptureType>().ToArray();
            RefreshWindowCommand = new(_RefreshWindowCommand);
            _RefreshWindowCommand();
        }
        public WinrtGraphicCaptureVM WinrtGraphicCaptureVM { get; } = new();




        public IEnumerable<CaptureType> CaptureTypes { get; }

        CaptureType? _RenderSelected = null;
        public CaptureType? RenderSelected
        {
            get { return _RenderSelected; }
            set { _RenderSelected = value; NotifyPropertyChange(); }
        }

        CaptureType? _CaptureImageSelected = null;
        public CaptureType? CaptureImageSelected
        {
            get { return _CaptureImageSelected; }
            set { _CaptureImageSelected = value; NotifyPropertyChange(); }
        }




        ImageSource? _CapturedImage = null;
        public ImageSource? CapturedImage
        {
            get { return _CapturedImage; }
            set { _CapturedImage = value; NotifyPropertyChange(); }
        }




        public ObservableCollection<WindowOrMonitorItem> WindowOrMonitors { get; } = new ObservableCollection<WindowOrMonitorItem>();
        WindowOrMonitorItem? _WindowOrMonitorSelected = null;
        public WindowOrMonitorItem? WindowOrMonitorSelected
        {
            get { return _WindowOrMonitorSelected; }
            set { _WindowOrMonitorSelected = value; NotifyPropertyChange(); }
        }
        public BaseCommand RefreshWindowCommand { get; }
        void _RefreshWindowCommand()
        {
            WindowOrMonitors.Clear();
            foreach (var item in BaseCapture.Monitors)
            {
                WindowOrMonitors.Add(new(item));
            }
            foreach (var item in WindowHelper.AllAltTabWindows)
            {
                WindowOrMonitors.Add(new(item));
            }
        }
    }
}
