#include "constant_buffer.h"
#include <dsc_render/draw_system.h>
#include <dsc_render/dsc_render.h>
#include <dsc_render/heap_wrapper_item.h>
#include <dsc_render/d3dx12.h>

DscRenderResource::ConstantBuffer::ConstantBuffer(
	const std::shared_ptr < DscRender::HeapWrapperItem >&in_heap_wrapper_item,
	const std::vector<uint8_t>& in_data,
	const D3D12_SHADER_VISIBILITY in_visiblity
	) 
	: _heap_wrapper_item(in_heap_wrapper_item)
	, _gpu_address(nullptr)
	, _data(in_data)
	, _visiblity(in_visiblity)
{
	// Nop
}

void DscRenderResource::ConstantBuffer::DeviceLost()
{
	_constant_buffer_upload_heap.Reset();
	_gpu_address = 0;
	return;
}

void DscRenderResource::ConstantBuffer::DeviceRestored(ID3D12Device* const in_device)
{
	const auto heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	const auto resource_desc = CD3DX12_RESOURCE_DESC::Buffer(1024* 64);
	DirectX::ThrowIfFailed(in_device->CreateCommittedResource(
		&heap_properties,
		// This heap will be used to upload the constant buffer data
		D3D12_HEAP_FLAG_NONE,
		// No flags
		&resource_desc,
		// Size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
		D3D12_RESOURCE_STATE_GENERIC_READ,
		// Will be data that is read from so we keep it in the generic read state
		nullptr,
		// We do not have use an optimized clear value for constant buffers
		IID_PPV_ARGS(&_constant_buffer_upload_heap)
		));
	_constant_buffer_upload_heap->SetName(L"Constant Buffer Upload Resource Heap");
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc = {};
	cbv_desc.BufferLocation = _constant_buffer_upload_heap->GetGPUVirtualAddress();
	cbv_desc.SizeInBytes = (_data.size() + 255) & ~255;
	// CB size is required to be 256-byte aligned.
	in_device->CreateConstantBufferView(
		&cbv_desc,
		_heap_wrapper_item->GetCPUHandleFrame()
		);
	// MainDescriptorHeap[i]->GetCPUDescriptorHandleForHeapStart());
	// ZeroMemory(&cbColorMultiplierData, sizeof(cbColorMultiplierData));
	CD3DX12_RANGE read_range(
		0,
		0
		);
	// We do not intend to read from this resource on the CPU. (End is less than or equal to begin)
	DirectX::ThrowIfFailed(_constant_buffer_upload_heap->Map(
		0,
		&read_range,
		reinterpret_cast < void** > (&_gpu_address)
		));
	// Memcpy(m_pGPUAddress[i], &cbColorMultiplierData, sizeof(cbColorMultiplierData));
}

void DscRenderResource::ConstantBuffer::Activate(
	ID3D12GraphicsCommandList* const in_command_list,
	// Void* const pData,
	const int in_root_param_index
	)
{
	ID3D12DescriptorHeap* descriptor_heaps[] =
	{
		_heap_wrapper_item->GetHeap()
	};
	in_command_list->SetDescriptorHeaps(
		_countof(descriptor_heaps),
		descriptor_heaps
		);
	in_command_list->SetGraphicsRootDescriptorTable(
		in_root_param_index,
		_heap_wrapper_item->GetGPUHandle()
		);
	if (0 == _gpu_address)
	{
		return;
	}
	memcpy(
		_gpu_address,
		_data.data(),
		_data.size()
		);
	return;
}

const int DscRenderResource::ConstantBuffer::GetNum32BitValues() const
{
	return (int)(_data.size() / 4);
}

void DscRenderResource::ConstantBuffer::UpdateData(
	const void* const in_data
#if defined(_DEBUG)
	, const size_t in_data_size
#endif
)
{
	DSC_ASSERT(in_data_size == _data.size(), "param size doesn't match data size");
	memcpy(
		_data.data(),
		in_data,
		_data.size()
		);
	return;
}
