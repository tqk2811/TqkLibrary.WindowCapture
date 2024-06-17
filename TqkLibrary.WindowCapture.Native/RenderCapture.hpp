#ifndef _WindowCapture_H_RenderCapture_H_
#define _WindowCapture_H_RenderCapture_H_

#include "BaseCapture.hpp"
#include "D3DClass.hpp"
#include "PixelShaderClass.hpp"
#include "VertexShaderClass.hpp"
#include "RenderTextureSurfaceClass.hpp"
#include "InputTextureClass.hpp"
#include "Exports.hpp"

class RenderCapture
{
public:
	RenderCapture();
	~RenderCapture();

	BOOL Init();
	BOOL Render(BaseCapture* baseCapture, IUnknown* surface, bool isNewSurface, bool& isNewtargetView);

private:

	D3DClass _d3d;
	PixelShaderClass _pixelShader;
	VertexShaderClass _vertexShader;
	RenderTextureSurfaceClass _renderSurface;
	InputTextureClass _inputTexture;
};


TqkLibrary_WindowCapture_Export RenderCapture* RenderCapture_Alloc();
TqkLibrary_WindowCapture_Export VOID RenderCapture_Free(RenderCapture** ppRenderCapture);
TqkLibrary_WindowCapture_Export BOOL RenderCapture_Render(RenderCapture* pRenderCapture, BaseCapture* baseCapture, IUnknown* surface, bool isNewSurface, bool& isNewtargetView);




#endif // !_WindowCapture_H_RenderCapture_H_


