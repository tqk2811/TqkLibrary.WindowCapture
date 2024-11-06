using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
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
