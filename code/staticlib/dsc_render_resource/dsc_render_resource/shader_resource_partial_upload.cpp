#include "shader_resource_partial_upload.h"
#include <dsc_render/draw_system.h>
#include <dsc_render/dsc_render.h>
#include <dsc_render/heap_wrapper_item.h>
#include <dsc_render/d3dx12.h>
#include <dsc_render/graphics_memory.h>

DscRenderResource::ShaderResourcePartialUpload::ShaderResourcePartialUpload(
	DscRender::DrawSystem* const in_draw_system,
	const std::shared_ptr<DscRender::HeapWrapperItem>&in_shader_resource,
	const D3D12_RESOURCE_DESC& in_desc,
	const D3D12_SHADER_RESOURCE_VIEW_DESC& in_shader_resource_view_desc,
	const std::vector<uint8_t>& in_data
	) 
	: DscRender::IResource(in_draw_system)
	, _shader_resource(in_shader_resource)
	, _desc(in_desc)
	, _shader_resource_view_desc(in_shader_resource_view_desc)
	, _data(in_data)
	, _current_state(D3D12_RESOURCE_STATE_COPY_DEST)
{
	ID3D12GraphicsCommandList* command_list = in_draw_system->CreateCommandList();
	ID3D12Device2* const device = in_draw_system->GetD3dDevice();

	UploadResource(command_list, device);

	in_draw_system->CommandListFinish(command_list);

	return;
}

void DscRenderResource::ShaderResourcePartialUpload::OnDeviceLost()
{
	_resource.Reset();
}

std::shared_ptr < DscRender::HeapWrapperItem > DscRenderResource::ShaderResourcePartialUpload::GetHeapWrapperItem() const
{
	return _shader_resource;
}

std::vector<uint8_t>& DscRenderResource::ShaderResourcePartialUpload::GetData(
	const bool in_mark_dirty,
	const int32 in_dirty_y_low,
	const int32 in_dirty_y_high
	)
{
	if (true == in_mark_dirty)
	{
		_dirty = true;
		_dirty_height_low = std::min(_dirty_height_low, in_dirty_y_low);
		_dirty_height_high = std::max(_dirty_height_high, in_dirty_y_high);
	}
	return _data;
}

void DscRenderResource::ShaderResourcePartialUpload::UploadDataIfDirty(
	DscRender::DrawSystem* const in_draw_system,
	ID3D12GraphicsCommandList* const in_command_list
	)
{
	if (_dirty)
	{
		_dirty = false;
		//TODO: actually fo a partial upload, rather than a full upload
		_dirty_height_low = _desc.Height;
		_dirty_height_high = 0;

		OnResourceBarrier(in_command_list, D3D12_RESOURCE_STATE_COPY_DEST);

		UploadResource(
			in_draw_system,
			in_command_list,
			_resource,
			_desc,
			_data.size(),
			_data.size() ? _data.data() : nullptr
			);

		OnResourceBarrier(in_command_list, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
}

void DscRenderResource::ShaderResourcePartialUpload::UploadResource(
	DscRender::DrawSystem* const in_draw_system,
	ID3D12GraphicsCommandList* const in_command_list,
	Microsoft::WRL::ComPtr<ID3D12Resource> &in_resource,
	const D3D12_RESOURCE_DESC& in_desc,
	const size_t in_data_size,
	void* in_data
	)
{
	if (nullptr != in_data)
	{
		const UINT64 upload_buffer_size = GetRequiredIntermediateSize(
			in_resource.Get(),
			0,
			1
			);
		auto memory_upload = in_draw_system->AllocateUpload(
			upload_buffer_size,
			nullptr,
			D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT
			);
		const int bytes_per_texel = (int) DirectX::BytesPerPixel(in_desc.Format);
		// //D3D12_ShaderResourcePartialUpload2D_DATA_PITCH_ALIGNMENT
		D3D12_SUBRESOURCE_DATA ShaderResourcePartialUpload2DData = {};
		ShaderResourcePartialUpload2DData.pData = in_data;
		ShaderResourcePartialUpload2DData.RowPitch = in_desc.Width * bytes_per_texel;
		ShaderResourcePartialUpload2DData.SlicePitch = in_data_size;
		if (in_command_list)
		{
			UpdateSubresources(
				in_command_list,
				in_resource.Get(),
				memory_upload.Resource(),
				memory_upload.ResourceOffset(),
				0,
				1,
				&ShaderResourcePartialUpload2DData
				);
		}
	}

	return;
}

void DscRenderResource::ShaderResourcePartialUpload::OnDeviceRestored(
	ID3D12GraphicsCommandList* const in_command_list,
	ID3D12Device2* const in_device
	)
{
	UploadResource(in_command_list, in_device);
	return;
}

void DscRenderResource::ShaderResourcePartialUpload::OnResourceBarrier(
	ID3D12GraphicsCommandList* const in_command_list,
	D3D12_RESOURCE_STATES in_new_state
)
{
	if (in_new_state == _current_state)
	{
		return;
	}

	D3D12_RESOURCE_BARRIER barrierDesc = {};

	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrierDesc.Transition.pResource = _resource.Get();
	barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrierDesc.Transition.StateBefore = _current_state;
	barrierDesc.Transition.StateAfter = in_new_state;

	in_command_list->ResourceBarrier(1, &barrierDesc);
	_current_state = in_new_state;
}

void DscRenderResource::ShaderResourcePartialUpload::UploadResource(
	ID3D12GraphicsCommandList* const in_command_list,
	ID3D12Device2* const in_device
	)
{
	_current_state = D3D12_RESOURCE_STATE_COPY_DEST;

	CD3DX12_HEAP_PROPERTIES heap_default(D3D12_HEAP_TYPE_DEFAULT);
	DirectX::ThrowIfFailed(in_device->CreateCommittedResource(
		&heap_default,
		D3D12_HEAP_FLAG_NONE,
		&_desc,
		_current_state,
		nullptr,
		IID_PPV_ARGS(_resource.ReleaseAndGetAddressOf())
	));
	_resource->SetName(L"Shader Texture2D resource");

	UploadResource(
		_draw_system,
		in_command_list,
		_resource,
		_desc,
		_data.size(),
		_data.size() ? _data.data() : nullptr
	);

	OnResourceBarrier(
		in_command_list,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);

	in_device->CreateShaderResourceView(
		_resource.Get(),
		&_shader_resource_view_desc,
		_shader_resource->GetCPUHandleFrame()
	);
}

