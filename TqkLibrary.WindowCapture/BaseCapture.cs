using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace TqkLibrary.WindowCapture
{
    public abstract class BaseCapture : BaseNative
    {

        [DllImport("TqkLibrary.WindowCapture.Native.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void BaseCapture_Free(ref IntPtr pointer);

        [DllImport("TqkLibrary.WindowCapture.Native.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern bool BaseCapture_InitCapture(IntPtr pointer, IntPtr handle);

        [DllImport("TqkLibrary.WindowCapture.Native.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr BaseCapture_Shoot(IntPtr pointer);

        [DllImport("TqkLibrary.WindowCapture.Native.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern bool BaseCapture_GetSize(IntPtr pointer, ref UInt32 width, ref UInt32 height);

        [DllImport("TqkLibrary.WindowCapture.Native.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern bool HBITMAP_Release(IntPtr hbitmap);


        public virtual Size Size
        {
            get
            {
                UInt32 width = 0;
                UInt32 height = 0;
                if(BaseCapture_GetSize(this.Pointer, ref width, ref height))
                {
                    return new Size((int)width, (int)height);
                }
                return Size.Empty;
            }
        }
        protected BaseCapture(IntPtr pointer) : base(pointer, BaseCapture_Free)
        {
        }

        public virtual bool Init(IntPtr captureHandler)
            => BaseCapture_InitCapture(Pointer, captureHandler);

        public virtual Bitmap Shoot()
        {
            IntPtr hBitmap = BaseCapture_Shoot(Pointer);
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
