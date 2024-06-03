// See https://aka.ms/new-console-template for more information
using System.Diagnostics;
using TqkLibrary.WindowCapture;

Console.WriteLine("Hello, World!");
var chromes = Process.GetProcessesByName("chrome");
var chrome = chromes.First(x => x.MainWindowHandle != IntPtr.Zero);
NativeWrapper.Test(chrome.MainWindowHandle);

Console.ReadLine();