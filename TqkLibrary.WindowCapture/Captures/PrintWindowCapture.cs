using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace TqkLibrary.WindowCapture.Captures
{
    public class PrintWindowCapture : BaseCapture
    {

        [DllImport("TqkLibrary.WindowCapture.Native.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr PrintWindow_Alloc();

        public PrintWindowCapture() : base(PrintWindow_Alloc())
        {

        }

    }
}
