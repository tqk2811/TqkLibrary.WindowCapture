using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace TqkLibrary.WindowCapture
{
    public class RenderCapture : BaseNative
    {
        [DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr RenderCapture_Alloc();

        [DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern void RenderCapture_Free(ref IntPtr pointer);

        [DllImport(_dllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern bool RenderCapture_Render(IntPtr pointer, IntPtr capture, IntPtr surface, bool isNewSurface, ref bool isNewtargetView);






        readonly BaseCapture _baseCapture;
        public RenderCapture(BaseCapture baseCapture) : base(RenderCapture_Alloc(), RenderCapture_Free)
        {
            this._baseCapture = baseCapture ?? throw new ArgumentNullException(nameof(baseCapture));
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="surface"></param>
        /// <param name="isNewSurface"></param>
        /// <param name="isNewtargetView">true if source image change size</param>
        /// <returns></returns>
        public virtual bool Draw(IntPtr surface, bool isNewSurface, ref bool isNewtargetView)
            => RenderCapture_Render(Pointer, _baseCapture.Pointer, surface, isNewSurface, ref isNewtargetView);

    }
}
