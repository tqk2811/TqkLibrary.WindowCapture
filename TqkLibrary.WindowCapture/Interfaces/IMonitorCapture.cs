using System;
using System.Collections.Generic;
using System.Text;

namespace TqkLibrary.WindowCapture.Interfaces
{
    public interface IMonitorCapture : ICapture
    {
        bool InitMonitor(IntPtr HMONITOR);
    }
}
