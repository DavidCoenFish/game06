#include "heap_wrapper.h"

#include "draw_system.h"
#include "heap_wrapper_page.h"

DscRender::HeapWrapper::HeapWrapper(
	DrawSystem* const in_draw_system,
	const D3D12_DESCRIPTOR_HEAP_TYPE in_type,
	const bool in_shader_visible,
	const UINT in_num_descriptors
	) 
	: IResource(in_draw_system)
	, _desc()
{
	_desc.Type = in_type;
	_desc.NumDescriptors = in_num_descriptors;
	_desc.Flags = (in_shader_visible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	return;
}

D3D12_CPU_DESCRIPTOR_HANDLE DscRender::HeapWrapper::GetCPUHandle(const int in_index)
{
	const int frame_index = _draw_system ? _draw_system->GetBackBufferIndex() : 0;
	return GetCPUHandleFrame(
		in_index
		);
}

D3D12_CPU_DESCRIPTOR_HANDLE DscRender::HeapWrapper::GetCPUHandleFrame(
	const int in_index
	)
{
	const int page_index = in_index / _desc.NumDescriptors;
	if ((0 <= page_index) && (page_index < (int) _array_page.size()))
	{
		const int local_index = in_index - (_desc.NumDescriptors * page_index);
		return _array_page[page_index]->GetCPUHandle(
			local_index
			);
	}
	return D3D12_CPU_DESCRIPTOR_HANDLE();
}

D3D12_GPU_DESCRIPTOR_HANDLE DscRender::HeapWrapper::GetGPUHandle(const int in_index)
{
	assert(_desc.Flags&D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	const int page_index = in_index / _desc.NumDescriptors;
	if ((0 <= page_index) && (page_index < (int) _array_page.size()))
	{
		const int local_index = in_index - (_desc.NumDescriptors* page_index);
		return _array_page[page_index]->GetGPUHandle(
			local_index
			);
	}
	return D3D12_GPU_DESCRIPTOR_HANDLE();
}

ID3D12DescriptorHeap* const DscRender::HeapWrapper::GetHeap(const int in_index)
{
	const int page_index = in_index / _desc.NumDescriptors;
	if ((0 <= page_index) && (page_index < (int) _array_page.size()))
	{
		return _array_page[page_index]->GetHeap();
	}
	return nullptr;
}

const int DscRender::HeapWrapper::GetFreeIndex(
	ID3D12Device2* const in_device,
	const int in_length
	)
{
	int trace = 0;
	for (auto iter : _array_page)
	{
		const int result = iter->GetFreeIndex(in_length);
		if (- 1 != result)
		{
			return result + trace;
		}
		trace += _desc.NumDescriptors;
	}

	{
		auto page = HeapWrapperPage::Factory(
			_desc,
			in_device
			);
		const int result = page->GetFreeIndex(in_length);
		_array_page.push_back(page);
		return result + trace;
	}
}

void DscRender::HeapWrapper::FreeIndex(
	const int in_index,
	const int in_length
	)
{
	const int page_index = in_index / _desc.NumDescriptors;
	if ((0 <= page_index) && (page_index < (int) _array_page.size()))
	{
		const int local_index = in_index - (_desc.NumDescriptors* page_index);
		_array_page[page_index]->FreeIndex(
			local_index,
			in_length
			);
	}
	return;
}

void DscRender::HeapWrapper::OnDeviceLost()
{
	for (auto iter : _array_page)
	{
		iter->OnDeviceLost();
	}
	return;
}

void DscRender::HeapWrapper::OnDeviceRestored(
	ID3D12GraphicsCommandList* const,
	ID3D12Device2* const in_device
	)
{
	for (auto iter : _array_page)
	{
		iter->OnDeviceRestored(
			_desc,
			in_device
			);
	}
	return;
}

