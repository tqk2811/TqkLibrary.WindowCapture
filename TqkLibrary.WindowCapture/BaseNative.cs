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
    public delegate void ReleaseNative(ref IntPtr pointer);
    public abstract class BaseNative
    {
#if DEBUG && NETFRAMEWORK
        static BaseNative()
        {
            string path = Path.Combine(
                Path.GetDirectoryName(Assembly.GetEntryAssembly()!.Location)!,
                "runtimes",
                Environment.Is64BitProcess ? "win-x64" : "win-x86",
                "native"
                );

            bool r = SetDllDirectory(path);
            if (!r)
                throw new InvalidOperationException("Can't set Kernel32.SetDllDirectory");
        }

        [DllImport("Kernel32.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Winapi)]
        internal static extern bool SetDllDirectory(string PathName);
#endif

        public IntPtr Pointer { get { return _pointer; } }
        private IntPtr _pointer;
        readonly ReleaseNative _releaseNative;
        protected BaseNative(
            IntPtr pointer,
            ReleaseNative releaseNative
            )
        {
            if (pointer == IntPtr.Zero) throw new ApplicationException($"{this.GetType().Name} alloc failed");
            this._pointer = pointer;
            this._releaseNative = releaseNative ?? throw new ArgumentNullException(nameof(releaseNative));
        }
        ~BaseNative()
        {
            Dispose(false);
        }
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        private void Dispose(bool disposing)
        {
            _releaseNative.Invoke(ref _pointer);
        }
    }
}
