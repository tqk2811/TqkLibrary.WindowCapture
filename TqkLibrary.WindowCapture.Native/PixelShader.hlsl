//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos         : SV_POSITION;
	float2 texCoord    : TEXCOORD;
};

Texture2D<float4>  colorChannel		 : t0;
SamplerState      defaultSampler     : s0;

[numthreads(32, 32, 1)]
float4 PS(PixelShaderInput input) : SV_TARGET
{
	return colorChannel.Sample(defaultSampler, input.texCoord);
}