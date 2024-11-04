using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace TqkLibrary.WindowCapture.Captures
{
    public class WindowGraphicCapture : BaseCapture
    {
        [DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr WindowGraphicCapture_Alloc();

        public WindowGraphicCapture() : base(WindowGraphicCapture_Alloc())
        {

        }
    }
}
