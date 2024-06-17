#include "RenderCapture.hpp"

RenderCapture* RenderCapture_Alloc()
{
	RenderCapture* renderCapture = new RenderCapture();

	if (!renderCapture->Init())
	{
		delete renderCapture;
		renderCapture = nullptr;
	}
	return renderCapture;
}
VOID RenderCapture_Free(RenderCapture** ppRenderCapture)
{
	if (ppRenderCapture)
	{
		RenderCapture* pRenderCapture = *ppRenderCapture;
		if (pRenderCapture)
		{
			delete pRenderCapture;
			*ppRenderCapture = nullptr;
		}
	}
}
BOOL RenderCapture_Render(RenderCapture* pRenderCapture, BaseCapture* baseCapture, IUnknown* surface, bool isNewSurface, bool& isNewtargetView)
{
	assert(pRenderCapture);
	assert(baseCapture);
	assert(surface);
	return pRenderCapture->Render(baseCapture, surface, isNewSurface, isNewtargetView);
}






RenderCapture::RenderCapture()
{

}
RenderCapture::~RenderCapture()
{

}

BOOL RenderCapture::Init()
{
	BOOL result = FALSE;
	result = this->_d3d.Initialize();

	if (result)
		result = this->_pixelShader.Initialize(this->_d3d.GetDevice(), D3D11_FILTER::D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT);
	if (result)
		result = this->_vertexShader.Initialize(this->_d3d.GetDevice());

	return result;
}

BOOL RenderCapture::Render(BaseCapture* baseCapture, IUnknown* surface, bool isNewSurface, bool& isNewtargetView)
{
	if (!baseCapture || !surface)
		return FALSE;

	ComPtr<ID3D11Device> device = this->_d3d.GetDevice();
	ComPtr<ID3D11DeviceContext> deviceCtx = this->_d3d.GetDeviceContext();

	BOOL result = this->_renderSurface.Initialize(device.Get(), surface, true, isNewtargetView);
	if (!result)
		return FALSE;

	ComPtr<ID3D11Texture2D> texture;
	result = baseCapture->Draw(device.Get(), deviceCtx.Get(), texture);
	if (!result)
		return FALSE;
	D3D11_TEXTURE2D_DESC desc;
	texture->GetDesc(&desc);

	result = this->_inputTexture.Initialize(device.Get(), desc.Width, desc.Height);
	if (!result)
		return FALSE;
	result = this->_inputTexture.Copy(deviceCtx.Get(), texture.Get());
	if (!result)
		return FALSE;

	deviceCtx->ClearState();
	deviceCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	this->_vertexShader.Set(deviceCtx.Get());
	this->_pixelShader.Set(deviceCtx.Get(), _inputTexture.GetView());//////////////////
	this->_renderSurface.SetRenderTarget(deviceCtx.Get(), nullptr);
	this->_renderSurface.SetViewPort(deviceCtx.Get(), this->_renderSurface.Width(), this->_renderSurface.Height());
	deviceCtx->Draw(this->_vertexShader.GetVertexCount(), 0);

	return result;
}