namespace TqkLibrary.WindowCapture
{
    public class Capture : IDisposable
    {
        public IntPtr WindowHandler { get; }
        public Capture(IntPtr handler)
        {
            this.WindowHandler = handler;
        }
        ~Capture()
        {

        }
        public void Dispose()
        {

        }
        void Dispose(bool isDisposing)
        {

        }

    }
}
