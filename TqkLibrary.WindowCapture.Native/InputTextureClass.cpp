﻿#include "InputTextureClass.hpp"
extern "C" {
	void planar_to_interleave
	(
		uint32_t uv_size,
		uint8_t* u_et_v,
		const uint8_t* u,
		const uint8_t* v
	)
	{
		assert(uv_size % 2 == 0);
		int u_size = uv_size / 2;
		int i;

#ifdef __AVX2__
		if (HW_AVX2 && uv_size % 32 == 0)
		{
			for (i = 0; i < u_size; i += 16)//16 byte step
			{
				//https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html
				__m128i s0 = _mm_loadu_epi8(&u[i]);//load 16 byte vào thanh ghi 128 bit
				__m128i s1 = _mm_loadu_epi8(&v[i]);
				__m128i s2 = _mm_unpacklo_epi8(s0, s1);//xen kẽ lo
				__m128i s3 = _mm_unpackhi_epi8(s0, s1);//xen kẽ hi
				*(__m128i*)(&u_et_v[2 * i]) = s2;//byte 0-15, 32-47,....
				*(__m128i*)(&u_et_v[2 * i + 16]) = s3;//byte 16-31, 48-63,.....
			}
			return;
		}
#endif // __AVX2__

		for (i = 0; i < u_size; i++)
		{
			uint8_t u_data = u[i];  // fetch u data
			uint8_t v_data = v[i];  // fetch v data

			u_et_v[2 * i] = u_data;   // write u data
			u_et_v[2 * i + 1] = v_data;   // write v data
		}
	}
}

InputTextureClass::InputTextureClass() {

}

InputTextureClass::~InputTextureClass() {
	this->Shutdown();
}

bool InputTextureClass::Initialize(ID3D11Device* device, int width, int height) {
	assert(device != nullptr);

	if (width != this->m_width || height != this->m_height) {
		this->Shutdown();
	}
	else
	{
		return true;
	}

	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;// DXGI_FORMAT_NV12;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.ArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.Usage = D3D11_USAGE_DEFAULT;// D3D11_USAGE_DYNAMIC;
	texDesc.CPUAccessFlags = 0;// D3D11_CPU_ACCESS_WRITE;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.MiscFlags = 0;
	HRESULT hr = device->CreateTexture2D(&texDesc, nullptr, this->m_texture.GetAddressOf());
	if (FAILED(hr))
		return false;

	//texDesc.Usage = D3D11_USAGE_DYNAMIC;
	//texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	//hr = device->CreateTexture2D(&texDesc, nullptr, this->m_texture_cache.GetAddressOf());
	//if (FAILED(hr))
	//	return false;


	D3D11_SHADER_RESOURCE_VIEW_DESC const viewDesc
		= CD3D11_SHADER_RESOURCE_VIEW_DESC(this->m_texture.Get(), D3D11_SRV_DIMENSION_TEXTURE2D, texDesc.Format);
	hr = device->CreateShaderResourceView(this->m_texture.Get(), &viewDesc, this->m_View.GetAddressOf());
	if (FAILED(hr))
		return false;

	this->m_width = width;
	this->m_height = height;

	return true;
}

BOOL InputTextureClass::Copy(ID3D11DeviceContext* deviceCtx, ID3D11Texture2D* texture)
{
	deviceCtx->CopyResource(this->m_texture.Get(), texture);
	return TRUE;
}


void InputTextureClass::Shutdown() {
	m_texture.Reset();
	m_View.Reset();
}

ID3D11ShaderResourceView* InputTextureClass::GetView() {
	return this->m_View.Get();
}

int InputTextureClass::Width() {
	return this->m_width;
}

int InputTextureClass::Height() {
	return this->m_height;
}
//
//bool InputTextureClass::Copy(ID3D11DeviceContext* device_ctx, const AVFrame* sourceFrame) {
//	if (sourceFrame->format == AV_PIX_FMT_D3D11 && sourceFrame->hw_frames_ctx != nullptr)
//	{
//		ComPtr<ID3D11Texture2D> texture = (ID3D11Texture2D*)sourceFrame->data[0];
//		const UINT64 texture_index = (UINT64)sourceFrame->data[1];
//
//		D3D11_TEXTURE2D_DESC desc{ 0 };
//		texture->GetDesc(&desc);
//
//		D3D11_BOX box{ 0 };
//		box.left = 0;
//		box.right = sourceFrame->width;
//		box.top = 0;
//		box.bottom = sourceFrame->height;
//		box.front = 0;
//		box.back = 1;//https://docs.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-copysubresourceregion
//
//		device_ctx->CopySubresourceRegion(
//			this->m_texture.Get(), 0, 0, 0, 0,
//			texture.Get(), (UINT32)texture_index, &box
//		);
//		return true;
//	}
//	else if (sourceFrame->format == AV_PIX_FMT_YUV420P)
//	{
//		/*this->m_texture_cache->SetEvictionPriority(DXGI_RESOURCE_PRIORITY_MAXIMUM);
//		this->m_texture->SetEvictionPriority(DXGI_RESOURCE_PRIORITY_MAXIMUM);*/
//		device_ctx->ClearState();
//
//		D3D11_MAPPED_SUBRESOURCE map;
//		device_ctx->Map(this->m_texture_cache.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &map);
//
//		INT64 y_size = sourceFrame->linesize[0] * sourceFrame->height;
//		INT64 uv_size = sourceFrame->linesize[1] * sourceFrame->height;//linesize = 1/2 width, height / 2, * 2 u and v
//		INT64 totalSize = y_size + uv_size;
//
//		//assert(map.DepthPitch == 0 || map.DepthPitch == totalSize);
//		assert(sourceFrame->linesize[1] == sourceFrame->linesize[2]);
//
//		bool result = false;
//		if ((UINT)sourceFrame->width <= map.RowPitch &&
//			(map.DepthPitch == 0 || map.DepthPitch == map.RowPitch * 3 * sourceFrame->height / 2))
//		{
//			if (sourceFrame->linesize[0] == map.RowPitch)
//			{
//				memcpy(map.pData, sourceFrame->data[0], y_size);
//				planar_to_interleave(
//					(UINT32)uv_size, 
//					(uint8_t*)((UINT64)map.pData + y_size), 
//					sourceFrame->data[1], 
//					sourceFrame->data[2]
//				);
//			}
//			else
//			{
//				for (int row = 0; row < sourceFrame->height; row++)
//				{
//					memcpy(
//						(uint8_t*)((UINT64)map.pData + (map.RowPitch * row)),
//						sourceFrame->data[0] + (sourceFrame->linesize[0] * row),
//						sourceFrame->width);
//				}
//				uint8_t* start_uv = (uint8_t*)map.pData + map.RowPitch * sourceFrame->height;
//				int uv_rowSizeCopy = sourceFrame->width;//sourceFrame->linesize[1] + sourceFrame->linesize[2];
//				int uv_height = sourceFrame->height / 2;
//
//				for (int row = 0; row < uv_height; row++)
//				{
//					planar_to_interleave(
//						uv_rowSizeCopy,
//						start_uv + map.RowPitch * row,
//						sourceFrame->data[1] + (sourceFrame->linesize[1] * row),
//						sourceFrame->data[2] + (sourceFrame->linesize[2] * row)
//					);
//				}
//			}
//			result = true;
//		}
//		else
//		{
//			std::wstring f(L"sourceFrame linesize: ");
//			f.append(std::to_wstring(sourceFrame->linesize[0]));
//			f.append(L", TotalSize:");
//			f.append(std::to_wstring(totalSize));
//			f.append(L", W:");
//			f.append(std::to_wstring(sourceFrame->width));
//			f.append(L", H:");
//			f.append(std::to_wstring(sourceFrame->height));
//			std::wstring s(L"D3D11_MAPPED_SUBRESOURCE.RowPitch");
//			s.append(std::to_wstring(map.RowPitch));
//			s.append(L", DepthPitch:");
//			s.append(std::to_wstring(map.DepthPitch));
//			MessageBox(NULL, f.c_str(), s.c_str(), 0);
//		}
//
//		device_ctx->Unmap(this->m_texture_cache.Get(), 0);
//
//		device_ctx->Flush();//force upload resoure from cpu -> gpu in m_texture_cache
//
//		device_ctx->CopyResource(this->m_texture.Get(), this->m_texture_cache.Get());
//		return result;
//	}
//	return false;
//}