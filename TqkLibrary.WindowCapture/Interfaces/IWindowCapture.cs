namespace TqkLibrary.WindowCapture.Interfaces
{
    public interface IWindowCapture : ICapture
    {
        bool InitWindow(IntPtr hwnd);
    }
}
