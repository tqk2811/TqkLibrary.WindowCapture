#include "RenderToSurface.hpp"


RenderToSurface::RenderToSurface()
{

}
RenderToSurface::~RenderToSurface()
{

}

BOOL RenderToSurface::Init()
{
	BOOL result = FALSE;
	result = this->_d3d.Initialize();

	if (result)
		result = this->_pixelShader.Initialize(this->_d3d.GetDevice(), D3D11_FILTER::D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT);
	if (result)
		result = this->_vertexShader.Initialize(this->_d3d.GetDevice());

	return result;
}

ComPtr<ID3D11Device> RenderToSurface::GetDevive()
{
	return _d3d.GetDevice();
}
ComPtr<ID3D11DeviceContext> RenderToSurface::GetDeviceContext()
{
	return _d3d.GetDeviceContext();
}
ComPtr<IDXGIDevice> RenderToSurface::GetDXGIDevice()
{
	return _d3d.GetDXGIDevice();
}


BOOL RenderToSurface::InitializeSurface(IUnknown* surface, bool isNewSurface, bool& isNewtargetView)
{
	if (!surface)
		return FALSE;

	ComPtr<ID3D11Device> device = this->_d3d.GetDevice();
	ComPtr<ID3D11DeviceContext> deviceCtx = this->_d3d.GetDeviceContext();

	BOOL result = this->_renderSurface.Initialize(device.Get(), surface, isNewSurface, isNewtargetView);
	if (!result)
		return FALSE;

	return result;
}
BOOL RenderToSurface::SendTexture(ID3D11Texture2D* texture)
{
	ComPtr<ID3D11Device> device = this->_d3d.GetDevice();
	ComPtr<ID3D11DeviceContext> deviceCtx = this->_d3d.GetDeviceContext();
	if (texture)
	{
		D3D11_TEXTURE2D_DESC desc;
		texture->GetDesc(&desc);

		BOOL result = this->_inputTexture.Initialize(device.Get(), desc.Width, desc.Height);
		if (!result)
			return FALSE;

		result = this->_inputTexture.Copy(deviceCtx.Get(), texture);
		if (!result)
			return FALSE;

		return TRUE;
	}//else re-render with old texture when new surface
	return TRUE;
}
BOOL RenderToSurface::Render()
{
	ComPtr<ID3D11DeviceContext> deviceCtx = this->_d3d.GetDeviceContext();

	deviceCtx->ClearState();
	deviceCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	this->_vertexShader.Set(deviceCtx.Get());
	this->_pixelShader.Set(deviceCtx.Get(), _inputTexture.GetView());//////////////////
	this->_renderSurface.SetRenderTarget(deviceCtx.Get(), nullptr);
	this->_renderSurface.SetViewPort(deviceCtx.Get(), this->_renderSurface.Width(), this->_renderSurface.Height());
	deviceCtx->Draw(this->_vertexShader.GetVertexCount(), 0);

	return TRUE;
}