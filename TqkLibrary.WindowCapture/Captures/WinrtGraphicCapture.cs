using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace TqkLibrary.WindowCapture.Captures
{
    public class WinrtGraphicCapture : BaseCapture
    {
        [DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr WinrtGraphicCapture_Alloc();

        public WinrtGraphicCapture() : base(WinrtGraphicCapture_Alloc())
        {

        }

        public override Task<bool> InitAsync(IntPtr hwnd)
        {
            return Task.Factory.StartNew(() => BaseCapture_InitCapture(Pointer, hwnd), TaskCreationOptions.LongRunning);
        }

        public override Task<Bitmap?> CaptureImageAsync()
        {
            Size size = Size;
            if (size.Width <= 0 || size.Height <= 0) return Task.FromResult<Bitmap?>(null);

            Bitmap bitmap = new Bitmap(size.Width, size.Height);
            BitmapData bitmapData = bitmap.LockBits(new Rectangle(0, 0, size.Width, size.Height), ImageLockMode.WriteOnly, PixelFormat.Format32bppRgb);

            //ARGB only
            bool result = BaseCapture_CaptureImage(
                base.Pointer,
                bitmapData.Scan0,
                (UInt32)bitmapData.Width,
                (UInt32)bitmapData.Height,
                (UInt32)bitmapData.Stride
                );
            bitmap.UnlockBits(bitmapData);

            if (result)
            {
                return Task.FromResult<Bitmap?>(bitmap);
            }
            else
            {
                bitmap.Dispose();
                return Task.FromResult<Bitmap?>(null);
            }
        }
    }
}
