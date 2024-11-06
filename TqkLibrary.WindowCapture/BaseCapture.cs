using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace TqkLibrary.WindowCapture
{
    public abstract class BaseCapture : BaseNative
    {

        [DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void BaseCapture_Free(ref IntPtr pointer);


        [DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern bool BaseCapture_InitCapture(IntPtr pointer, IntPtr hwnd);


        [DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern bool BaseCapture_GetSize(IntPtr pointer, ref UInt32 width, ref UInt32 height);


        [DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern bool BaseCapture_CaptureImage(IntPtr pointer, IntPtr data, UInt32 width, UInt32 height, UInt32 lineSize);


        [DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern bool BaseCapture_Render(IntPtr pointer, IntPtr surface, bool isNewSurface, ref bool isNewtargetView);



        public virtual Size Size
        {
            get
            {
                UInt32 width = 0;
                UInt32 height = 0;
                if (BaseCapture_GetSize(this.Pointer, ref width, ref height))
                {
                    return new Size((int)width, (int)height);
                }
                return Size.Empty;
            }
        }
        protected BaseCapture(IntPtr pointer) : base(pointer, BaseCapture_Free)
        {
        }

        public virtual Task<bool> InitAsync(IntPtr hwnd)
        {
            return Task.FromResult(BaseCapture_InitCapture(Pointer, hwnd));
        }

        public abstract Task<Bitmap?> CaptureImageAsync();

        public bool Render(IntPtr surface, bool isNewSurface, ref bool isNewtargetView)
            => BaseCapture_Render(Pointer, surface, isNewSurface, ref isNewtargetView);
    }
}
