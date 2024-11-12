using TqkLibrary.WpfUi;

namespace WpfTest
{
    internal class WinrtGraphicCaptureVM : BaseViewModel
    {
        public ToggleVM Cursor { get; } = new();
        public ToggleVM Border { get; } = new();

        internal class ToggleVM : BaseViewModel
        {
            bool _IsShow = true;
            public bool IsShow
            {
                get { return _IsShow; }
                set { _IsShow = value; NotifyPropertyChange(); }
            }

            bool _IsSupported = false;
            public bool IsSupported
            {
                get { return _IsSupported; }
                set { _IsSupported = value; NotifyPropertyChange(); }
            }
        }
    }
}
