using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;
using TqkLibrary.WindowCapture.Interfaces;

namespace TqkLibrary.WindowCapture.Captures
{
    public sealed class HdcCapture : BaseCapture, IMonitorCapture, IWindowCapture
    {
        public enum HdcCaptureMode : byte
        {
            BitBlt,
            PrintWindow
        }


        [DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        static extern IntPtr HdcCapture_Alloc();

        [DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        static extern HdcCaptureMode HdcCapture_GetMode(IntPtr pointer);

        [DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        static extern void HdcCapture_SetMode(IntPtr pointer, HdcCaptureMode mode);


        public HdcCapture() : base(HdcCapture_Alloc())
        {

        }

        public HdcCaptureMode Mode
        {
            get { return HdcCapture_GetMode(Pointer); }
            set { HdcCapture_SetMode(Pointer, value); }
        }
        public bool InitWindow(IntPtr hwnd) => BaseCapture_InitWindowCapture(Pointer, hwnd);
        public bool InitMonitor(IntPtr HMONITOR)
        {
            if (Mode == HdcCaptureMode.PrintWindow) throw new InvalidOperationException($"Mode {nameof(HdcCaptureMode.PrintWindow)} not support capture Monitor");
            return BaseCapture_InitMonitorCapture(Pointer, HMONITOR);
        }

    }
}
