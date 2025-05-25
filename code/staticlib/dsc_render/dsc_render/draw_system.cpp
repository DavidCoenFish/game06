#include "draw_system.h"
#include "device_resources.h"
#include "i_resource.h"
#include "graphics_memory.h"
#include "heap_wrapper.h"
#include "heap_wrapper_item.h"
#include "dsc_render.h"
#include <dsc_common/log_system.h>

std::unique_ptr<DscRender::DrawSystem> DscRender::DrawSystem::Factory(
	const HWND in_hwnd
	)
{
	return std::make_unique<DrawSystem>(
		in_hwnd
		);
}

DscRender::DrawSystem::DrawSystem(
	const HWND in_hwnd,
	const unsigned int in_back_buffer_count,
	const D3D_FEATURE_LEVEL in_d3d_feature_level,
	const unsigned int in_options,
	const RenderTargetFormatData& in_target_format_data,
	const RenderTargetDepthData& in_target_depth_data,
	const int in_num_descriptors
	) 
	: _hwnd(in_hwnd)
	, _back_buffer_count(in_back_buffer_count)
	, _d3d_feature_level(in_d3d_feature_level)
	, _options(in_options)
	, _target_format_data(in_target_format_data)
	, _target_depth_data(in_target_depth_data)
{
	_heap_wrapper_cbv_srv_uav = std::make_shared < HeapWrapper > (
		this,
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		true,
		in_num_descriptors
		);
	_heap_wrapper_sampler = std::make_shared < HeapWrapper > (
		this,
		D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
		false,
		in_num_descriptors
		);
	_heap_wrapper_render_target_view = std::make_shared < HeapWrapper > (
		this,
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		false,
		in_num_descriptors
		);
	_heap_wrapper_depth_stencil_view = std::make_shared < HeapWrapper > (
		this,
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
		false,
		in_num_descriptors
		);
	CreateDeviceResources();
}

DscRender::DrawSystem::~DrawSystem()
{
	WaitForGpu();
	for (auto iter : _list_resource)
	{
		iter->OnDeviceLost();
	}
	_list_resource.clear();
	//_resource_list.clear();
	_device_resources.reset();
}

/*
DirectX::GraphicsResource DscRender::DrawSystem::AllocateConstant(
	const std::size_t in_size,
	void* const in_constants
	)
{
	return AllocateUpload(
		in_size,
		in_constants,
		D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT
		);
}

DirectX::GraphicsResource DscRender::DrawSystem::AllocateUpload(
	const std::size_t in_size,
	void* const in_data_or_nullptr,
	size_t in_alignment
	)
{
	if (_device_resources)
	{
		return _device_resources->AllocateUpload(
			in_size,
			in_data_or_nullptr,
			in_alignment
			);
	}
	return DirectX::GraphicsResource();
}

void DscRender::DrawSystem::ResizeRenderTargetTexture(
	ID3D12GraphicsCommandList* const in_command_list,
	RenderTargetTexture* const in_render_target_texture,
	const VectorInt2& in_size
	)
{
	if ((nullptr != in_render_target_texture) && (nullptr != _device_resources))
	{
		in_render_target_texture->Resize(
			in_command_list,
			_device_resources->GetD3dDevice(),
			in_size
			);
	}
}

void DscRender::DrawSystem::ForceRestore(
	ID3D12GraphicsCommandList* const in_command_list,
	IResource* const in_resource
	)
{
	if (nullptr != _device_resources)
	{
		_device_resources->ForceRestore(
			in_command_list,
			in_resource
			);
	}
}

std::shared_ptr<CustomCommandList> DscRender::DrawSystem::CreateCustomCommandList(
	ID3D12PipelineState* const in_pipeline_state_object_or_null
	)
{
	if (_device_resources)
	{
		auto command_list = _device_resources->GetCustomCommandList(in_pipeline_state_object_or_null);
		return std::make_shared<CustomCommandList>(
			* this,
			command_list
			);
	}
	return nullptr;
}

void DscRender::DrawSystem::CustomCommandListFinish(ID3D12GraphicsCommandList* in_command_list)
{
	if (_device_resources)
	{
		_device_resources->CustomCommandListFinish(in_command_list);
	}
	return;
}
*/
ID3D12GraphicsCommandList* DscRender::DrawSystem::CreateCustomCommandList(
	ID3D12PipelineState* const in_pipeline_state_object_or_null
)
{
	if (_device_resources)
	{
		ID3D12GraphicsCommandList* command_list = _device_resources->GetCustomCommandList(in_pipeline_state_object_or_null);
		return command_list;
	}
	return nullptr;
}

void DscRender::DrawSystem::CustomCommandListFinish(ID3D12GraphicsCommandList* in_command_list)
{
	if (_device_resources)
	{
		_device_resources->CustomCommandListFinish(in_command_list);
	}
	return;
}

void DscRender::DrawSystem::Prepare(ID3D12GraphicsCommandList*& in_command_list)
{
	DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_RENDER, "Prepare\n");

	//RemoveCompletedResourceList();

	if (nullptr != _device_resources)
	{
		_device_resources->Prepare(in_command_list);
	}
	return;
}

void DscRender::DrawSystem::Present()
{
	DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_RENDER, "Present\n");

	if (nullptr == _device_resources)
	{
		return;
	}
	if (false == _device_resources->Present())
	{
		CreateDeviceResources();
	}
}

DscRender::IRenderTarget* DscRender::DrawSystem::GetRenderTargetBackBuffer()
{
	if (_device_resources)
	{
		return _device_resources->GetRenderTargetBackBuffer();
	}
	return nullptr;
}

std::shared_ptr < DscRender::HeapWrapperItem > DscRender::DrawSystem::MakeHeapWrapperCbvSrvUav(const int in_length)
{
	return HeapWrapperItem::Factory(
		_device_resources ? _device_resources->GetD3dDevice() : nullptr,
		_heap_wrapper_cbv_srv_uav,
		in_length
		);
}

std::shared_ptr < DscRender::HeapWrapperItem > DscRender::DrawSystem::MakeHeapWrapperSampler(const int in_length)
{
	return HeapWrapperItem::Factory(
		_device_resources ? _device_resources->GetD3dDevice() : nullptr,
		_heap_wrapper_sampler,
		in_length
		);
}

std::shared_ptr < DscRender::HeapWrapperItem > DscRender::DrawSystem::MakeHeapWrapperRenderTargetView(const int in_length)
{
	return HeapWrapperItem::Factory(
		_device_resources ? _device_resources->GetD3dDevice() : nullptr,
		_heap_wrapper_render_target_view,
		in_length
		);
}

std::shared_ptr < DscRender::HeapWrapperItem > DscRender::DrawSystem::MakeHeapWrapperDepthStencilView(const int in_length)
{
	return HeapWrapperItem::Factory(
		_device_resources ? _device_resources->GetD3dDevice() : nullptr,
		_heap_wrapper_depth_stencil_view,
		in_length
		);
}

void DscRender::DrawSystem::WaitForGpu() noexcept
{
	if (_device_resources)
	{
		_device_resources->WaitForGpu();
	}
}

void DscRender::DrawSystem::OnResize()
{
	bool resized = false;
	int32 size_width = 0;
	int32 size_height = 0;
	if (_device_resources)
	{
		resized = _device_resources->OnResize(
			this,
			_hwnd,
			size_width,
			size_height
			);
	}
	if ((true == resized) && (0 < _list_resource.size()))
	{
		auto command_list = CreateCustomCommandList();
		for (auto iter : _list_resource)
		{
			iter->OnResize(
				command_list,
				_device_resources->GetD3dDevice(),
				size_width,
				size_height
				);
		}
		CustomCommandListFinish(command_list);
	}
}

void DscRender::DrawSystem::AddResource(IResource* const in_resource)
{
	if (in_resource)
	{
		_list_resource.push_back(in_resource);
	}
}

void DscRender::DrawSystem::RemoveResource(IResource* const in_resource)
{
	// If RemoveResource is called by the dtor, then not a good idea to call a virutal func on the pResource?
	// If (pResource)
	// {
	// PResource->OnDeviceLost();
	// }
	_list_resource.remove(in_resource);
}

const int DscRender::DrawSystem::GetBackBufferIndex() const
{
	if (nullptr != _device_resources)
	{
		_device_resources->GetBackBufferIndex();
	}
	return 0;
}

void DscRender::DrawSystem::CreateDeviceResources()
{
	for (auto iter : _list_resource)
	{
		iter->OnDeviceLost();
	}
	_device_resources.reset();
	_device_resources = std::make_unique<DeviceResources>(
		2,
		_d3d_feature_level,
		_options,
		_target_format_data,
		_target_depth_data
		);
	// Two pass construction as rendertargetbackbuffer calls MakeHeapWrapperRenderTargetView, MakeHeapWrapperDepthStencilView which need m_pDeviceResources assigned
	_device_resources->CreateWindowSizeDependentResources(
		this,
		_hwnd
		);
	if (0 < _list_resource.size())
	{
		auto command_list = CreateCustomCommandList();
		for (auto iter : _list_resource)
		{
			iter->OnDeviceRestored(
				command_list,
				_device_resources->GetD3dDevice()
				);
		}
		CustomCommandListFinish(command_list);
	}
}

/*
void DscRender::DrawSystem::RemoveCompletedResourceList()
{
	// Release completed frame assets
	_resource_list.erase(
		std::remove_if(
			_resource_list.begin(), 
			_resource_list.end(),
			[](const std::shared_ptr<DrawSystemResourceList>& item) { 
				return item->GetFinished();
			}), 
		_resource_list.end());
}
*/