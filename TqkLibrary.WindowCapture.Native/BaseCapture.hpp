#ifndef _WindowCapture_H_BaseCapture_H_
#define _WindowCapture_H_BaseCapture_H_

#include "Directx.hpp"
#include "D3DClass.hpp"
#include "Exports.hpp"

class BaseCapture
{
public:
	virtual BOOL Draw(ID3D11Device* device, ID3D11DeviceContext* deviceCtx, ComPtr<ID3D11Texture2D>& texture) = 0;
};


#endif


