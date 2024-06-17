#ifndef _WindowCapture_H_RenderTextureSurfaceClass_H_
#define _WindowCapture_H_RenderTextureSurfaceClass_H_

#include "Directx.hpp"
class RenderTextureSurfaceClass
{
public:
	RenderTextureSurfaceClass();
	~RenderTextureSurfaceClass();

	bool Initialize(ID3D11Device* device, IUnknown* surface, bool isNewSurface, bool& isNewtargetView);
	void Shutdown();

	void SetRenderTarget(ID3D11DeviceContext*, ID3D11DepthStencilView*);
	void ClearRenderTarget(ID3D11DeviceContext*, ID3D11DepthStencilView*, float, float, float, float);
	void SetViewPort(ID3D11DeviceContext* device_ctx);
	void SetViewPort(ID3D11DeviceContext* device_ctx, int width, int height);

	int Width() { return m_Width; }
	int Height() { return m_Height; }
private:
	int m_Width{ 0 };
	int m_Height{ 0 };
	ComPtr<ID3D11RenderTargetView> m_pRenderTargetView = nullptr;

};



#endif // ! _WindowCapture_H_RenderTextureSurfaceClass_H_

