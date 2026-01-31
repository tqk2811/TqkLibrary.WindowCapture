using TqkLibrary.WinApi.Helpers;
using TqkLibrary.WpfUi;

namespace WpfTest
{
    class WindowOrMonitorItem : BaseViewModel
    {
        public WindowOrMonitorItem(WindowHelper windowHelper)
        {
            this.WindowHelper = windowHelper ?? throw new ArgumentNullException(nameof(windowHelper));
        }
        public WindowOrMonitorItem(IntPtr hmonitor)
        {
            if (hmonitor == IntPtr.Zero) throw new ArgumentNullException(nameof(hmonitor));
            this.HMonitor = hmonitor;
        }
        public WindowHelper? WindowHelper { get; init; }
        public IntPtr? HMonitor { get; init; }


        public override string ToString()
        {
            if (WindowHelper is not null)
            {
                return WindowHelper.Title;
            }
            else if (HMonitor is not null)
            {
                return HMonitor.Value.ToString("X" + (IntPtr.Size * 2));
            }
            return $"Empty";
        }
    }
}
