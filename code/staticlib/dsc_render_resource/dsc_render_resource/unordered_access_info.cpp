#include "unordered_access_info.h"
#include <dsc_render/heap_wrapper_item.h>

std::shared_ptr<DscRenderResource::UnorderedAccessInfo> DscRenderResource::UnorderedAccessInfo::Factory(
	const std::shared_ptr<DscRender::HeapWrapperItem>& in_unordered_access_view_handle,
	const D3D12_SHADER_VISIBILITY in_visiblity
	)
{
	return std::make_shared<UnorderedAccessInfo>(
		in_unordered_access_view_handle,
		in_visiblity
		);
}


DscRenderResource::UnorderedAccessInfo::UnorderedAccessInfo(
	const std::shared_ptr < DscRender::HeapWrapperItem >&in_unordered_access_view_handle,
	const D3D12_SHADER_VISIBILITY in_visiblity
	) 
	: _unordered_access_view_handle(in_unordered_access_view_handle)
	, _visiblity(in_visiblity)
{
	return;
}

void DscRenderResource::UnorderedAccessInfo::SetUnorderedAccessViewHandle(const std::shared_ptr < DscRender::HeapWrapperItem >&\
	in_unordered_access_view_handle)
{
	_unordered_access_view_handle = in_unordered_access_view_handle;
	return;
}

void DscRenderResource::UnorderedAccessInfo::Activate(
	ID3D12GraphicsCommandList* const in_command_list,
	const int in_root_param_index,
	const bool in_compute_shader
	)
{
	// we don't know if we have a UAV or a SRV (as _unordered_access_view_handle) but the interface matters to the command list, ie, which type of root signiture is in use
	if (_unordered_access_view_handle)
	{
		auto heap = _unordered_access_view_handle->GetHeap();
		in_command_list->SetDescriptorHeaps(
			1,
			&heap
			);
		if (true == in_compute_shader)
		{
			in_command_list->SetComputeRootDescriptorTable(
				in_root_param_index,
				_unordered_access_view_handle->GetGPUHandle()
				);
		}
		else
		{
			in_command_list->SetGraphicsRootDescriptorTable(
				in_root_param_index,
				_unordered_access_view_handle->GetGPUHandle()
			);
		}
	}
	return;
}

