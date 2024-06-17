#ifndef _WindowCapture_H_InputTextureClass_H_
#define _WindowCapture_H_InputTextureClass_H_

#include "Directx.hpp"

class InputTextureClass
{
public:
	InputTextureClass();
	~InputTextureClass();


	bool Initialize(ID3D11Device* device, int width, int height);
	void Shutdown();

	ID3D11ShaderResourceView* GetView();
	BOOL Copy(ID3D11DeviceContext* deviceCtx, ID3D11Texture2D* texture);

	int Width();
	int Height();

private:
	int m_width{ 0 };
	int m_height{ 0 };

	ComPtr<ID3D11Texture2D> m_texture = nullptr;

	ComPtr<ID3D11ShaderResourceView> m_View = nullptr;
};
#endif // !_WindowCapture_H_InputTextureClass_H_


