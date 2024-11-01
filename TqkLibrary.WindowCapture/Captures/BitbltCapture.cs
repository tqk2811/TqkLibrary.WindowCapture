using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace TqkLibrary.WindowCapture.Captures
{
    public class BitbltCapture : BaseCapture
    {

        [DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr BitbltCapture_Alloc();

        public BitbltCapture() : base(BitbltCapture_Alloc())
        {

        }

    }
}
