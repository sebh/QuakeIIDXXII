#pragma once

#include "Dx12Device.h"




class RenderBufferGenericDynamic
{
public:
	RenderBufferGenericDynamic(
		uint64 TotalSizeInBytes, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
	virtual ~RenderBufferGenericDynamic();

	void* Map();
	void Unmap();

protected:

private:
	RenderBufferGenericDynamic();
	RenderBufferGenericDynamic(RenderBufferGenericDynamic&);

	RenderBufferGeneric* mFrameUploadBuffers[frameBufferCount];
	RenderBufferGeneric mRenderBuffer;
};




////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////




class RenderTextureDynamic
{
public:
	RenderTextureDynamic(
		unsigned int width, unsigned int height,
		unsigned int depth, DXGI_FORMAT format,
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
		unsigned int initDataCopySizeByte = 0, unsigned int RowPitchByte = 0, unsigned int SlicePitchByte = 0);
	virtual ~RenderTextureDynamic();

	void* Map();
	void Unmap();

	void Upload(void* DataPtr, unsigned int RowPitchByte, unsigned int SlicePitchByte);

protected:

private:
	RenderTextureDynamic();
	RenderTextureDynamic(RenderTextureDynamic&);

	unsigned int mInitDataCopySizeByte = 0;
	unsigned int mRowPitchByte = 0;
	unsigned int mSlicePitchByte = 0;

	ID3D12Resource* mFrameUploadTextures[frameBufferCount];
	RenderTexture mRenderTexture;
};




