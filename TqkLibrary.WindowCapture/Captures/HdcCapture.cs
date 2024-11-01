using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;

namespace TqkLibrary.WindowCapture.Captures
{
    public class HdcCapture : BaseCapture
    {
        public enum HdcCaptureMode : byte
        {
            BitBlt,
            PrintWindow
        }


        [DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr HdcCapture_Alloc();

        [DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern HdcCaptureMode HdcCapture_GetMode(IntPtr pointer);

        [DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void HdcCapture_SetMode(IntPtr pointer, HdcCaptureMode mode);



        public HdcCapture() : base(HdcCapture_Alloc())
        {

        }

        public HdcCaptureMode Mode
        {
            get { return HdcCapture_GetMode(this.Pointer); }
            set { HdcCapture_SetMode(this.Pointer, value); }
        }
    }
}
