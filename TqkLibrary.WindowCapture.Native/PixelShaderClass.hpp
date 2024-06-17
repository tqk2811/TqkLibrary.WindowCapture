#ifndef _WindowCapture_H_PixelShaderClass_H_
#define _WindowCapture_H_PixelShaderClass_H_

class PixelShaderClass
{
public:
	PixelShaderClass();
	~PixelShaderClass();

	bool Initialize(ID3D11Device* d3d11_device, D3D11_FILTER filter);
	void Set(ID3D11DeviceContext* d3d11_deviceCtx, ID3D11ShaderResourceView* colorResourceView);
	void Shutdown();

private:
	ComPtr<ID3D11PixelShader> m_d3d11_pixelShader = nullptr;
	ComPtr<ID3D11SamplerState> m_d3d11_samplerState = nullptr;
};

#endif // !_WindowCapture_H_PixelShaderClass_H_



