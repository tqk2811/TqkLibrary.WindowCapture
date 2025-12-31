using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;

namespace TqkLibrary.WindowCapture.Captures
{
    public sealed class WinrtGraphicCapture : BaseCapture
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
        static extern bool WinrtGraphicCapture_SetCursorState(IntPtr pointer, bool isVisible);

        [DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        static extern bool WinrtGraphicCapture_GetCursorState(IntPtr pointer, ref bool isVisible);



        [DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        static extern bool WinrtGraphicCapture_IsBorderToggleSupported();

        [DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        static extern bool WinrtGraphicCapture_SetBorderState(IntPtr pointer, bool isVisible);

        [DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        static extern bool WinrtGraphicCapture_GetBorderState(IntPtr pointer, ref bool isVisible);




        //[DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        //static extern bool WinrtGraphicCapture_IsMinUpdateIntervalSupported();

        //[DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        //static extern bool WinrtGraphicCapture_SetMinUpdateInterval(IntPtr pointer, TimeSpan timeSpan);

        //[DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        //static extern bool WinrtGraphicCapture_GetMinUpdateInterval(IntPtr pointer, ref TimeSpan timeSpan);


        public static bool IsCaptureCursorToggleSupported { get; } = WinrtGraphicCapture_IsCaptureCursorToggleSupported();
        public static bool IsBorderToggleSupported { get; } = WinrtGraphicCapture_IsBorderToggleSupported();
        //public static bool IsMinUpdateIntervalSupported { get; } = WinrtGraphicCapture_IsMinUpdateIntervalSupported();


        /*
https://jrsoftware.org/ishelp/index.php?topic=winvernotes
10.0.18362	Windows 10 Version 1903 (May 2019 Update)
10.0.18363	Windows 10 Version 1909 (November 2019 Update)
10.0.19041	Windows 10 Version 2004 (May 2020 Update)
10.0.19042	Windows 10 Version 20H2 (October 2020 Update)
10.0.19043	Windows 10 Version 21H1 (May 2021 Update)
10.0.19044	Windows 10 Version 21H2 (November 2021 Update)
10.0.19045	Windows 10 Version 22H2 (2022 Update)
10.0.20348	Windows Server 2022 Version 21H2
10.0.22000	Windows 11 Version 21H2 (original release)
10.0.22621	Windows 11 Version 22H2 (2022 Update)
10.0.22631	Windows 11 Version 23H2 (2023 Update)
10.0.26100	Windows 11 Version 24H2 (2024 Update)
         */
        const string _Text_IsShowCursor_Error = "Not support to change Cursor Visibly. Required: Windows 10, version 2004 (introduced in 10.0.19041.0)";
        const string _Text_IsShowBorder_Error = "Not support to change Border Visibly. Required: Windows 10, version 2104 (introduced in 10.0.20348.0)";
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

        //public TimeSpan MinUpdateInterval
        //{
        //    get
        //    {
        //        TimeSpan timeSpan = TimeSpan.Zero;
        //        Task.Factory.StartNew(() => WinrtGraphicCapture_GetMinUpdateInterval(Pointer, ref timeSpan), TaskCreationOptions.LongRunning).Wait();
        //        return timeSpan;
        //    }
        //    set
        //    {
        //        Task.Factory.StartNew(() => WinrtGraphicCapture_SetMinUpdateInterval(Pointer, value), TaskCreationOptions.LongRunning).Wait();
        //    }
        //}

        public bool IsShowCursor
        {
            get
            {
                if (!IsCaptureCursorToggleSupported)
                    throw new NotSupportedException(_Text_IsShowCursor_Error);
                bool isVisible = false;
                Task.Factory.StartNew(() => WinrtGraphicCapture_GetCursorState(Pointer, ref isVisible), TaskCreationOptions.LongRunning).Wait();
                return isVisible;
            }
            set
            {
                if (!IsCaptureCursorToggleSupported)
                    throw new NotSupportedException(_Text_IsShowCursor_Error);
                Task.Factory.StartNew(() => WinrtGraphicCapture_SetCursorState(Pointer, value), TaskCreationOptions.LongRunning).Wait();
            }
        }

        public bool IsShowBorder
        {
            get
            {
                if (!IsBorderToggleSupported)
                    throw new NotSupportedException(_Text_IsShowBorder_Error);
                bool isVisible = false;
                Task.Factory.StartNew(() => WinrtGraphicCapture_GetBorderState(Pointer, ref isVisible), TaskCreationOptions.LongRunning).Wait();
                return isVisible;
            }
            set
            {
                if (!IsBorderToggleSupported)
                    throw new NotSupportedException(_Text_IsShowBorder_Error);
                Task.Factory.StartNew(() => WinrtGraphicCapture_SetBorderState(Pointer, value), TaskCreationOptions.LongRunning).Wait();
            }
        }


        public override Task<bool> InitWindowAsync(IntPtr hwnd)
        {
            return Task.Factory.StartNew(() => BaseCapture_InitWindowCapture(Pointer, hwnd), TaskCreationOptions.LongRunning);
        }
        public override Task<bool> InitMonitorAsync(IntPtr HMONITOR)
        {
            return Task.Factory.StartNew(() => BaseCapture_InitMonitorCapture(Pointer, HMONITOR), TaskCreationOptions.LongRunning);
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
