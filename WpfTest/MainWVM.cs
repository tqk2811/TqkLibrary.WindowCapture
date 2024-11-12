using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;
using System.Windows.Media;
using TqkLibrary.WinApi.FindWindowHelper;
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




        public ObservableCollection<WindowHelper> Windows { get; } = new ObservableCollection<WindowHelper>();
        WindowHelper? _WindowHelperSelected = null;
        public WindowHelper? WindowHelperSelected
        {
            get { return _WindowHelperSelected; }
            set { _WindowHelperSelected = value; NotifyPropertyChange(); }
        }
        public BaseCommand RefreshWindowCommand { get; }
        void _RefreshWindowCommand()
        {
            Windows.Clear();
            foreach (var item in WindowHelper.AllAltTabWindows)
            {
                Windows.Add(item);
            }
        }
    }
}
