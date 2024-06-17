#ifndef _WindowCapture_H_D3DClass_H_
#define _WindowCapture_H_D3DClass_H_

#include "Directx.hpp"
class D3DClass
{
public:
	D3DClass();
	~D3DClass();

	bool Initialize();
	void Shutdown();

	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetDeviceContext();
private:
	ComPtr<ID3D11Device> m_device = nullptr;
	ComPtr<ID3D11DeviceContext> m_deviceContext = nullptr;
};
#endif // !_WindowCapture_H_D3DClass_H_