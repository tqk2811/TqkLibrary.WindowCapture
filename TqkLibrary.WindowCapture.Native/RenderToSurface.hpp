#ifndef _WindowCapture_H_RenderToSurface_H_
#define _WindowCapture_H_RenderToSurface_H_

#include "Directx.hpp"
#include "D3DClass.hpp"
#include "PixelShaderClass.hpp"
#include "VertexShaderClass.hpp"
#include "RenderTextureSurfaceClass.hpp"
#include "InputTextureClass.hpp"
#include "Exports.hpp"

class RenderToSurface
{
public:
	RenderToSurface();
	~RenderToSurface();

	BOOL Init();

	BOOL InitializeSurface(IUnknown* surface, bool isNewSurface, bool& isNewtargetView);
	BOOL RenderTexture(ID3D11Texture2D* texture);


	ComPtr<ID3D11Device> GetDevive();
	ComPtr<ID3D11DeviceContext> GetDeviceContext();
	ComPtr<IDXGIDevice> GetDXGIDevice();

protected:
	D3DClass _d3d;
	PixelShaderClass _pixelShader;
	VertexShaderClass _vertexShader;
	RenderTextureSurfaceClass _renderSurface;
	InputTextureClass _inputTexture;

	ComPtr<ID3D11Texture2D> _texture{ NULL };

};

#endif