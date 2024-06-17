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
        [DllImport("TqkLibrary.WindowCapture.Native.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr BitbltCapture_Alloc();

        [DllImport("TqkLibrary.WindowCapture.Native.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void BitbltCapture_Free(ref IntPtr pointer);

        [DllImport("TqkLibrary.WindowCapture.Native.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern bool BitbltCapture_InitCapture(IntPtr pointer, IntPtr handle);

        [DllImport("TqkLibrary.WindowCapture.Native.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr BitbltCapture_Shoot(IntPtr pointer);

        [DllImport("TqkLibrary.WindowCapture.Native.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern bool HBITMAP_Release(IntPtr hbitmap);



        public BitbltCapture() : base(BitbltCapture_Alloc(), BitbltCapture_Free)
        {

        }

        public override bool Init(IntPtr captureHandler)
            => BitbltCapture_InitCapture(Pointer, captureHandler);

        public override Bitmap Shoot()
        {
            IntPtr hBitmap = BitbltCapture_Shoot(Pointer);
            try
            {
                return Bitmap.FromHbitmap(hBitmap);
            }
            finally
            {
                HBITMAP_Release(hBitmap);
            }
        }
    }
}
