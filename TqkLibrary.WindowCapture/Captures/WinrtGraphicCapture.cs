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
        static extern IntPtr WinrtGraphicCapture_Alloc();

        [DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        static extern Int32 WinrtGraphicCapture_GetDelay(IntPtr pointer);

        [DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        static extern void WinrtGraphicCapture_SetDelay(IntPtr pointer, Int32 delay);

        [DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        static extern bool WinrtGraphicCapture_IsCaptureCursorToggleSupported();

        [DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        static extern bool WinrtGraphicCapture_ShowCursor(IntPtr pointer, bool isVisible);

        [DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        static extern bool WinrtGraphicCapture_GetShowCursorState(IntPtr pointer, ref bool state);


        public static bool IsCaptureCursorToggleSupported { get; } = WinrtGraphicCapture_IsCaptureCursorToggleSupported();
        protected override string _NotSupportedExceptionText => "required Win10 1903 or higher";
        public WinrtGraphicCapture() : base(WinrtGraphicCapture_Alloc())
        {

        }
        protected override void Dispose(bool disposing)
        {
            Task.Factory.StartNew(() => base.Dispose(disposing), TaskCreationOptions.LongRunning).Wait();
        }

        /// <summary>
        /// 0 is no limit
        /// </summary>
        public int MaxFps
        {
            get { return (int)(1000.0 / WinrtGraphicCapture_GetDelay(Pointer)); }
            set
            {
                if (value <= 0) WinrtGraphicCapture_SetDelay(Pointer, 0);
                else
                {
                    WinrtGraphicCapture_SetDelay(Pointer, (int)(1000.0 / value));
                }
            }
        }
        public bool IsShowCursor
        {
            get
            {
                if (!IsCaptureCursorToggleSupported)
                    throw new NotSupportedException($"Not support to change Cursor Visibly");
                bool state = false;
                Task.Factory.StartNew(() => WinrtGraphicCapture_GetShowCursorState(Pointer, ref state), TaskCreationOptions.LongRunning).Wait();
                return state;
            }
            set
            {
                if (!IsCaptureCursorToggleSupported)
                    throw new NotSupportedException($"Not support to change Cursor Visibly");
                Task.Factory.StartNew(() => WinrtGraphicCapture_ShowCursor(Pointer, value), TaskCreationOptions.LongRunning).Wait();
            }
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
