#include "resource_list.h"
#include "i_resource.h"
#include "dsc_render.h"

#if defined(DRAW_SYSTEM_RESOURCE_LIST_DEBUG)
int s_id = 0;
#endif


std::shared_ptr<DscRender::ResourceList> DscRender::ResourceList::Factory(
	ID3D12Device& in_device
	)
{
	Microsoft::WRL::ComPtr<ID3D12Fence> fence;

	// Create a fence for tracking GPU execution progress.
	DirectX::ThrowIfFailed(in_device.CreateFence(
		0,
		D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(fence.ReleaseAndGetAddressOf())
		));
	DSC_ASSERT(nullptr != fence, "failed to create fence");
	fence->SetName(L"Fence");

	return std::make_shared<DscRender::ResourceList>(fence);
}

DscRender::ResourceList::ResourceList(
	Microsoft::WRL::ComPtr<ID3D12Fence>& in_fence
	)
	: _fence(in_fence)
	, _command_list_marked_finished(false)
#if defined(DRAW_SYSTEM_RESOURCE_LIST_DEBUG)
	, _id(s_id++)
#endif
{
#if defined(DRAW_SYSTEM_RESOURCE_LIST_DEBUG)
	LOG_MESSAGE_RENDER("DscRender::ResourceList_[%d] ctor", _id);
#endif

	// Nop
}

DscRender::ResourceList::~ResourceList()
{
#if defined(DRAW_SYSTEM_RESOURCE_LIST_DEBUG)
	LOG_MESSAGE_RENDER("DscRender::ResourceList_[%d] dtor", _id);
#endif

	// Nop
}

void DscRender::ResourceList::AddResource(
	const std::shared_ptr<IResource>& in_resource
	)
{
	DSC_ASSERT(false == _command_list_marked_finished, "once a command list has been marked finished, do not add more resources");
	if (nullptr != in_resource)
	{
		_resource_array.push_back(in_resource);
	}
	return;
}

void DscRender::ResourceList::MarkFinished(
	const Microsoft::WRL::ComPtr<ID3D12CommandQueue>& in_command_queue
	)
{
	in_command_queue->Signal(
		_fence.Get(),
		1
		);

	_command_list_marked_finished = true;

	return;
}

const bool DscRender::ResourceList::GetFinished() const
{
	auto completed_value = _fence->GetCompletedValue();

#if defined(DRAW_SYSTEM_RESOURCE_LIST_DEBUG)
	LOG_MESSAGE_RENDER("DscRender::ResourceList_[%d] completed_value %d, size %d", _id, completed_value, _resource_array.size() );
#endif

	return (0 < completed_value);
}

