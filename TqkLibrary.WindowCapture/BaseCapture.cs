using System.Drawing;
using System.Runtime.InteropServices;

namespace TqkLibrary.WindowCapture
{
    public abstract class BaseCapture : BaseNative
    {

        [DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        static extern void BaseCapture_Free(ref IntPtr pointer);


        [DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        protected static extern bool BaseCapture_InitWindowCapture(IntPtr pointer, IntPtr hwnd);


        [DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        protected static extern bool BaseCapture_InitMonitorCapture(IntPtr pointer, IntPtr HMONITOR);


        [DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        protected static extern bool BaseCapture_GetSize(IntPtr pointer, ref UInt32 width, ref UInt32 height);


        [DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        protected static extern bool BaseCapture_IsSupported(IntPtr pointer);


        [DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        protected static extern bool BaseCapture_CaptureImage(IntPtr pointer, IntPtr data, UInt32 width, UInt32 height, UInt32 lineSize);


        [DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        protected static extern bool BaseCapture_Render(IntPtr pointer, IntPtr surface, bool isNewSurface, ref bool isNewtargetView);



        public virtual Size Size
        {
            get
            {
                UInt32 width = 0;
                UInt32 height = 0;
                if (BaseCapture_GetSize(Pointer, ref width, ref height))
                {
                    return new Size((int)width, (int)height);
                }
                return Size.Empty;
            }
        }

        protected virtual string _NotSupportedExceptionText { get; } = string.Empty;
        protected BaseCapture(IntPtr pointer) : base(pointer, BaseCapture_Free)
        {
            if (!BaseCapture_IsSupported(pointer))
            {
                Dispose();
                throw new NotSupportedException(_NotSupportedExceptionText);
            }
        }

        public virtual Task<bool> InitWindowAsync(IntPtr hwnd)
        {
            return Task.FromResult(BaseCapture_InitWindowCapture(Pointer, hwnd));
        }
        public virtual Task<bool> InitMonitorAsync(IntPtr HMONITOR)
        {
            return Task.FromResult(BaseCapture_InitMonitorCapture(Pointer, HMONITOR));
        }

        public abstract Task<Bitmap?> CaptureImageAsync();

        public virtual bool Render(IntPtr surface, bool isNewSurface, ref bool isNewtargetView)
            => BaseCapture_Render(Pointer, surface, isNewSurface, ref isNewtargetView);
    }
}
