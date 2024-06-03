// pch.cpp: source file corresponding to the pre-compiled header

#include "pch.h"

// When you are using pre-compiled headers, this source file is necessary for compilation to succeed.

void Test(HWND windowHandler) {
	HRESULT hr{ 0 };
	HMONITOR hmonitor = MonitorFromWindow(windowHandler, MONITOR_DEFAULTTONEAREST);
	RECT rect;
	if (!GetWindowRect(windowHandler, &rect))
	{
		return;
	}

	ComPtr<IDirect3D9> pD3D9 = Direct3DCreate9(D3D_SDK_VERSION);
	UINT a_count = pD3D9->GetAdapterCount();
	INT monitorIndex = -1;
	for (UINT i = 0; i < a_count; i++)
	{
		HMONITOR hMonitorTmp = pD3D9->GetAdapterMonitor(i);
		if (hMonitorTmp == hmonitor)
		{
			monitorIndex = i;
			break;
		}
	}

	if (monitorIndex == -1)
		return;

	D3DDISPLAYMODE dMode;
	hr = pD3D9->GetAdapterDisplayMode(monitorIndex, &dMode);
	assert(SUCCEEDED(hr));


	/*
	* https://spazzarama.com/2009/02/07/screencapture-with-direct3d/
	*/
	D3DPRESENT_PARAMETERS para;
	ZeroMemory(&para, sizeof(D3DPRESENT_PARAMETERS));
	para.BackBufferWidth = rect.right - rect.left;//
	para.BackBufferHeight = rect.bottom - rect.top;//
	para.BackBufferFormat = dMode.Format;//
	para.BackBufferCount = 0;

	para.MultiSampleType = D3DMULTISAMPLE_TYPE::D3DMULTISAMPLE_NONE;//
	para.MultiSampleQuality = 0;

	para.SwapEffect = D3DSWAPEFFECT::D3DSWAPEFFECT_DISCARD;//
	para.hDeviceWindow = windowHandler;//
	para.Windowed = TRUE;
	para.EnableAutoDepthStencil = FALSE;
	para.AutoDepthStencilFormat = D3DFORMAT::D3DFMT_UNKNOWN;// dMode.Format;
	para.Flags = 0;

	para.FullScreen_RefreshRateInHz = 0;//
	para.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;//

	ComPtr<IDirect3DDevice9> d3d9Device;
	hr = pD3D9->CreateDevice(
		monitorIndex,
		D3DDEVTYPE::D3DDEVTYPE_HAL,
		windowHandler,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&para,
		d3d9Device.GetAddressOf()
	);
	assert(SUCCEEDED(hr));

	ComPtr<IDirect3DSurface9> surface;
	hr = d3d9Device->CreateOffscreenPlainSurface(
		dMode.Width,
		dMode.Height,
		D3DFORMAT::D3DFMT_A8R8G8B8,
		D3DPOOL::D3DPOOL_SYSTEMMEM,
		surface.GetAddressOf(),
		NULL
	);
	assert(SUCCEEDED(hr));

	hr = d3d9Device->GetFrontBufferData(0, surface.Get());
	assert(SUCCEEDED(hr));

	HDC hdc;
	hr = surface->GetDC(&hdc);
	assert(SUCCEEDED(hr));


	hr = surface->ReleaseDC(hdc);
	assert(SUCCEEDED(hr));

	return;
}
