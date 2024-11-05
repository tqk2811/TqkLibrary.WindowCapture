#ifndef _WindowCapture_H_VertexShaderClass_H_
#define _WindowCapture_H_VertexShaderClass_H_

class VertexShaderClass
{
public:
	VertexShaderClass();
	~VertexShaderClass();

	bool Initialize(ComPtr<ID3D11Device> d3d11_device);
	void Set(ComPtr<ID3D11DeviceContext> d3d11_deviceCtx);
	UINT GetVertexCount();
	void Shutdown();

private:
	ComPtr<ID3D11VertexShader> m_d3d11_vertexShader = nullptr;
	ComPtr<ID3D11InputLayout> m_d3d11_inputLayout = nullptr;
	ComPtr<ID3D11Buffer> m_d3d11_vertexBuffer = nullptr;
};
#endif // !VertexShaderClass_H

