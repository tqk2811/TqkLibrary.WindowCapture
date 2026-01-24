using System.Drawing;

namespace TqkLibrary.WindowCapture.Interfaces
{
    public interface ICapture : IDisposable
    {
        Size Size { get; }
        Bitmap? Capture();
        bool Render(IntPtr surface, bool isNewSurface, ref bool isNewtargetView);
    }
}
