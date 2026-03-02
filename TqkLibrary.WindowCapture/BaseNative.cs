namespace TqkLibrary.WindowCapture
{
    public delegate void ReleaseNative(ref IntPtr pointer);
    public abstract class BaseNative : IDisposable
    {
        protected const string _dllName = "TqkLibrary.WindowCapture.Native.dll";
#if DEBUG && NETFRAMEWORK
        static BaseNative()
        {
            var entryAssembly = Assembly.GetEntryAssembly();
            if (entryAssembly == null) return;
            string? dir = Path.GetDirectoryName(entryAssembly.Location);
            if (dir == null) return;

            string path = Path.Combine(
                dir,
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

        private bool _disposed = false;
        protected IntPtr Pointer
        {
            get
            {
                if (_disposed) throw new ObjectDisposedException(GetType().Name);
                return _pointer;
            }
        }
        private IntPtr _pointer;
        readonly ReleaseNative _releaseNative;
        protected BaseNative(
            IntPtr pointer,
            ReleaseNative releaseNative
            )
        {
            if (pointer == IntPtr.Zero) throw new ApplicationException($"{GetType().Name} alloc failed");
            _pointer = pointer;
            _releaseNative = releaseNative ?? throw new ArgumentNullException(nameof(releaseNative));
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

        protected virtual void Dispose(bool disposing)
        {
            if (!_disposed)
            {
                _releaseNative.Invoke(ref _pointer);
                _disposed = true;
            }
        }
    }
}

