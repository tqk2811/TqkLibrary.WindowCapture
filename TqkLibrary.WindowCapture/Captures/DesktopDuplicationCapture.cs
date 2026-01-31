using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using TqkLibrary.WindowCapture.Interfaces;

namespace TqkLibrary.WindowCapture.Captures
{
    public class DesktopDuplicationCapture : BaseCapture, IMonitorCapture
    {

        [DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr DesktopDuplication_Alloc();


        public DesktopDuplicationCapture() : base(DesktopDuplication_Alloc())
        {
        }

        public bool InitMonitor(IntPtr HMONITOR) => BaseCapture_InitMonitorCapture(Pointer, HMONITOR);
    }
}
