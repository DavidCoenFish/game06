#include "device_resources.h"
#include "render_target_format_data.h"
#include "render_target_depth_data.h"
#include "i_resource.h"
#include "i_render_target.h"
#include "dsc_render.h"
#include "screen_size_resources.h"
#include "graphics_memory.h"
#include "d3dx12.h"
#include <dsc_common/log_system.h>
#include <dsc_common/utf8.h>

DscRender::DeviceResources::DeviceResources(
	const unsigned int in_back_buffer_count,
	const D3D_FEATURE_LEVEL in_d3d_feature_level,
	const unsigned int in_options,
	const RenderTargetFormatData&in_target_format_data,
	const RenderTargetDepthData&in_target_depth_data
	) 
	: _back_buffer_count(in_back_buffer_count)
	, _options(in_options)
	, _target_format_data(in_target_format_data)
	, _target_depth_data(in_target_depth_data)
	, _dxgi_factory_flags(0)
	, _custom_command_list_fence_value(0)
{
	static int s_count = - 1;
	s_count += 1;

#if defined (_DEBUG)
		// Enable the debug layer (requires the Graphics Tools "optional feature").
		//
		// NOTE: Enabling the debug layer after device creation will invalidate the active device.

		{
			Microsoft::WRL::ComPtr < ID3D12Debug > debug_controller;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(debug_controller.GetAddressOf()))))
			{
				debug_controller->EnableDebugLayer();
			}
			else
			{
				DSC_LOG_WARNING(LOG_TOPIC_DSC_RENDER, "Direct3D Debug Device is not available\n");
			}
			Microsoft::WRL::ComPtr < IDXGIInfoQueue > dxgi_info_queue;
			if (SUCCEEDED(DXGIGetDebugInterface1(
				0,
				IID_PPV_ARGS(dxgi_info_queue.GetAddressOf())
				)))
			{
				_dxgi_factory_flags = DXGI_CREATE_FACTORY_DEBUG;
				dxgi_info_queue->SetBreakOnSeverity(
					DXGI_DEBUG_ALL,
					DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR,
					true
					);
				dxgi_info_queue->SetBreakOnSeverity(
					DXGI_DEBUG_ALL,
					DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION,
					true
					);
				DXGI_INFO_QUEUE_MESSAGE_ID hide[] =
				{
					80, 
					// IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides.
				};
				DXGI_INFO_QUEUE_FILTER filter = {};
				filter.DenyList.NumIDs = _countof(hide);
				filter.DenyList.pIDList = hide;
				dxgi_info_queue->AddStorageFilterEntries(
					DXGI_DEBUG_DXGI,
					&filter
					);
			}
		}
#endif


#if 1
	DirectX::ThrowIfFailed(CreateDXGIFactory2(
		_dxgi_factory_flags,
		IID_PPV_ARGS(_dxgi_factory.ReleaseAndGetAddressOf())
		));
#else
	//https://stackoverflow.com/questions/64903828/proper-way-to-create-a-idxgifactory7
	//use CreateDXGIFactory2() to create an IDXGIFactory2 and use QueryInterface() on that to obtain the one for IDXGIFactory7.
	DirectX::ThrowIfFailed(CreateDXGIFactory2(
		_dxgi_factory_flags,
		IID_PPV_ARGS(_dxgi_factory_2.ReleaseAndGetAddressOf())
		));

	DirectX::ThrowIfFailed(_dxgi_factory_2->QueryInterface(
		__uuidof(IDXGIFactory7), 
		(void**)_dxgi_factory.ReleaseAndGetAddressOf()
		));

#endif

	// Determines whether tearing support is available for fullscreen borderless windows.
	if (_options&c_allow_tearing)
	{
		BOOL allow_tearing = FALSE;
		HRESULT hr = _dxgi_factory->CheckFeatureSupport(
			DXGI_FEATURE_PRESENT_ALLOW_TEARING,
			&allow_tearing,
			sizeof (allow_tearing)
			);
		if (FAILED(hr) || !allow_tearing)
		{
			_options &= ~c_allow_tearing;
			DSC_LOG_WARNING(LOG_TOPIC_DSC_RENDER, "WARNING: Variable refresh rate displays not supported");
		}
	}
	Microsoft::WRL::ComPtr < IDXGIAdapter1 > adapter;
	GetAdapter(
		adapter.GetAddressOf(),
		in_d3d_feature_level
		);
	// Create the DX12 API device object.
	HRESULT hr = D3D12CreateDevice(
		adapter.Get(),
		in_d3d_feature_level,
		IID_PPV_ARGS(_device.ReleaseAndGetAddressOf())
		);
	DirectX::ThrowIfFailed(hr);

	{
		wchar_t name[64] = {};
		swprintf_s(
			name,
			L"Device:%d",
			s_count
			);
		_device->SetName(name);
	}
#ifndef NDEBUG
		// Configure debug device (if active).
		Microsoft::WRL::ComPtr < ID3D12InfoQueue > d3d_info_queue;
		if (SUCCEEDED(_device.As(&d3d_info_queue)))
		{
#ifdef _DEBUG
				d3d_info_queue->SetBreakOnSeverity(
					D3D12_MESSAGE_SEVERITY_CORRUPTION,
					true
					);
				d3d_info_queue->SetBreakOnSeverity(
					D3D12_MESSAGE_SEVERITY_ERROR,
					true
					);
#endif

			D3D12_MESSAGE_ID hide[] =
			{
				D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE, D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE, \
					D3D12_MESSAGE_ID_EXECUTECOMMANDLISTS_WRONGSWAPCHAINBUFFERREFERENCE};
			D3D12_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = _countof(hide);
			filter.DenyList.pIDList = hide;
			d3d_info_queue->AddStorageFilterEntries(&filter);
		}
#endif

	// Create the command queue.
	D3D12_COMMAND_QUEUE_DESC queue_desc = {};
	queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	DirectX::ThrowIfFailed(_device->CreateCommandQueue(
		&queue_desc,
		IID_PPV_ARGS(_command_queue.ReleaseAndGetAddressOf())
		));

	{
		wchar_t name[64] = {};
		swprintf_s(
			name,
			L"CommandQueue:%d",
			s_count
			);
		_command_queue->SetName(name);
	}

	_graphics_memory = std::make_unique < DirectX::GraphicsMemory > (_device.Get());
	if (nullptr == _graphics_memory)
	{
		throw std::exception("CreateEvent");
	}

	DirectX::ThrowIfFailed(_device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(_custom_command_allocator.ReleaseAndGetAddressOf())
		));

	{
		wchar_t name[64] = {};
		swprintf_s(
			name,
			L"CustomCommandAllocator:%d",
			s_count
			);
		_custom_command_allocator->SetName(name);
	}
	DirectX::ThrowIfFailed(_device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		_custom_command_allocator.Get(),
		nullptr,
		IID_PPV_ARGS(_custom_command_list.ReleaseAndGetAddressOf())
		));

	{
		wchar_t name[64] = {};
		swprintf_s(
			name,
			L"CustomCommandList:%d",
			s_count
			);
		_custom_command_list->SetName(name);
	}
	_custom_command_list->Close();
	DirectX::ThrowIfFailed(_device->CreateFence(
		0,
		D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(_custom_command_fence.ReleaseAndGetAddressOf())
		));
	_custom_command_fence_event.Attach(CreateEventEx(
		nullptr,
		nullptr,
		0,
		EVENT_MODIFY_STATE | SYNCHRONIZE
		));
	if (!_custom_command_fence_event.IsValid())
	{
		throw std::exception("CustomCommandFence");
	}

	D3D12_FEATURE_DATA_SHADER_MODEL shader_model =
	{
		D3D_SHADER_MODEL_6_5};
	if (FAILED(_device->CheckFeatureSupport(
		D3D12_FEATURE_SHADER_MODEL,
		&shader_model,
		sizeof (shader_model)
		)) || (shader_model.HighestShaderModel < D3D_SHADER_MODEL_6_5))
	{
		DSC_LOG_WARNING(LOG_TOPIC_DSC_RENDER, "Shader Model 6.5 is not supported");
	}
	D3D12_FEATURE_DATA_D3D12_OPTIONS7 feature_options7 = {};
	if (FAILED(_device->CheckFeatureSupport(
		D3D12_FEATURE_D3D12_OPTIONS7,
		&feature_options7,
		sizeof (feature_options7)
		)) || (feature_options7.MeshShaderTier == D3D12_MESH_SHADER_TIER_NOT_SUPPORTED))
	{
		DSC_LOG_WARNING(LOG_TOPIC_DSC_RENDER, "Mesh Shaders aren't supported");
	}
}

DscRender::DeviceResources::~DeviceResources()
{
	WaitForGpu();
	// Can go accross frames
	WaitForCustomCommand();
	// Make sure any outstanding command list are completed
	_custom_command_allocator.Reset();
	_custom_command_list.Reset();
	_custom_command_fence.Reset();
	_command_queue.Reset();
	_screen_size_resources.reset();
	_graphics_memory.reset();
	_device.Reset();
	_dxgi_factory.Reset();

#ifdef _DEBUG
		{
			Microsoft::WRL::ComPtr < IDXGIDebug1 > dxgi_debug;
			if (SUCCEEDED(DXGIGetDebugInterface1(
				0,
				IID_PPV_ARGS(&dxgi_debug)
				)))
			{
				dxgi_debug->ReportLiveObjects(
					DXGI_DEBUG_ALL,
					DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL)
					);
			}
		}
#endif
}

void DscRender::DeviceResources::WaitForGpu() noexcept
{
	if (_screen_size_resources)
	{
		_screen_size_resources->WaitForGpu(_command_queue);
	}
	return;
}

void DscRender::DeviceResources::WaitForCustomCommand()// Make sure any outstanding command list are completed
{
	if (_command_queue && _custom_command_fence && _custom_command_fence_event.IsValid())
	{
		if (SUCCEEDED(_command_queue->Signal(
			_custom_command_fence.Get(),
			_custom_command_list_fence_value
			)))
		{
			// Wait until the Signal has been processed.
			if (SUCCEEDED(_custom_command_fence->SetEventOnCompletion(
				_custom_command_list_fence_value,
				_custom_command_fence_event.Get()
				)))
			{
				WaitForSingleObjectEx(
					_custom_command_fence_event.Get(),
					INFINITE,
					FALSE
					);
				// Increment the fence value for the current frame.
				_custom_command_list_fence_value += 1;
			}
		}
	}
}

const bool DscRender::DeviceResources::OnResize(
	DrawSystem* const in_draw_system,
	const HWND in_hwnd,
	int32& out_width,
	int32& out_Height
	)
{
	return CreateWindowSizeDependentResources(
		in_draw_system,
		in_hwnd,
		&out_width,
		&out_Height
		);
}

const int DscRender::DeviceResources::GetBackBufferIndex() const
{
	if (nullptr != _screen_size_resources)
	{
		return _screen_size_resources->GetBackBufferIndex();
	}
	return 0;
}

DirectX::GraphicsResource DscRender::DeviceResources::AllocateUpload(
	const std::size_t in_size,
	void* const in_data_or_nullptr,
	size_t in_alignment
	)
{
	const size_t aligned_size = (in_size + in_alignment - 1) &~ (in_alignment - 1);
	auto graphics_resource = _graphics_memory->Allocate(
		aligned_size,
		in_alignment
		);
	if (in_data_or_nullptr)
	{
		memcpy(
			graphics_resource.Memory(),
			in_data_or_nullptr,
			in_size
			);
	}
	return graphics_resource;
}

void DscRender::DeviceResources::GetAdapter(
	IDXGIAdapter1** in_pp_adapter,
	const D3D_FEATURE_LEVEL in_d3d_feature_level
	)
{
	* in_pp_adapter = nullptr;
	Microsoft::WRL::ComPtr < IDXGIAdapter1 > adapter;
	for (UINT adapter_index = 0; SUCCEEDED(_dxgi_factory->EnumAdapterByGpuPreference(
		adapter_index,
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
		IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf())
		)); adapter_index++)
	{
		DXGI_ADAPTER_DESC1 desc;
		DirectX::ThrowIfFailed(adapter->GetDesc1(&desc));
		if (desc.Flags&DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// Don't select the Basic Render Driver adapter.
			continue;
		}
		// Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
		if (SUCCEEDED(D3D12CreateDevice(
			adapter.Get(),
			in_d3d_feature_level,
			_uuidof(ID3D12Device),
			nullptr
			)))
		{
#ifdef _DEBUG
				DSC_LOG_DIAGNOSTIC(
					LOG_TOPIC_DSC_RENDER,
					"Direct3D Adapter (%u): VID:%04X, PID:%04X - %s\n",
					adapter_index,
					desc.VendorId,
					desc.DeviceId,
					DscCommon::Utf8::Utf16ToUtf8(desc.Description).c_str()
					);
#endif

			break;
		}
	}
#if !defined (NDEBUG)
		if (!adapter)
		{
			// Try WARP12 instead
			if (FAILED(_dxgi_factory->EnumWarpAdapter(IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf()))))
			{
				throw std::exception("WARP12 not available. Enable the 'Graphics Tools' optional feature");
			}
			DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_RENDER,"Direct3D Adapter - WARP12\n");
		}
#endif

	if (!adapter)
	{
		throw std::exception("No Direct3D 12 device found");
	}
	* in_pp_adapter = adapter.Detach();
	return;
}

// Return true if size changed
const bool DscRender::DeviceResources::CreateWindowSizeDependentResources(
	DrawSystem* const in_draw_system,
	const HWND in_hwnd,
	int32* const out_width,
	int32* const out_height
	)
{
	RECT rc;
	GetClientRect(
		in_hwnd,
		&rc
		);
	const int32 width = rc.right - rc.left;
	const int32 height = rc.bottom - rc.top;

	if (nullptr != out_width)
	{
		*out_width = width;
	}
	if (nullptr != out_height)
	{
		*out_height = height;
	}

	// If we don't need to resize, then don't
	if ((nullptr != _screen_size_resources) && 
		(width == _screen_size_resources->GetSizeWidth()) && 
		(height == _screen_size_resources->GetSizeHeight()))
	{
		return false;
	}
	WaitForGpu();
	UINT64 fence_value = 0;
	if (nullptr != _screen_size_resources)
	{
		fence_value = _screen_size_resources->GetFenceValue();
	}
	_screen_size_resources.reset();
	const bool swap_flag = (0 != (_options & c_allow_tearing));
	_screen_size_resources = std::make_unique<ScreenSizeResources>(
		in_draw_system,
		_device,
		_dxgi_factory,
		_command_queue,
		in_hwnd,
		fence_value,
		_back_buffer_count,
		width,
		height,
		swap_flag, //bool
		_target_format_data,
		_target_depth_data
		);

	return true;
}

void DscRender::DeviceResources::MoveToNextFrame()
{
	if (_screen_size_resources)
	{
		_screen_size_resources->MoveToNextFrame(_command_queue);
	}
}

void DscRender::DeviceResources::Prepare(ID3D12GraphicsCommandList*&in_command_list)
{
	if (nullptr != _screen_size_resources)
	{
		_screen_size_resources->Prepare(in_command_list);
	}
	return;
}

ID3D12Device2* const DscRender::DeviceResources::GetD3dDevice()
{
	return _device.Get();
}

ID3D12GraphicsCommandList* DscRender::DeviceResources::GetCustomCommandList(
	ID3D12PipelineState* const in_pipeline_state_object_or_null
	)
{
	WaitForCustomCommand();
	DirectX::ThrowIfFailed(_custom_command_allocator->Reset());
	DirectX::ThrowIfFailed(_custom_command_list->Reset(
		_custom_command_allocator.Get(),
		in_pipeline_state_object_or_null
		));
	DirectX::ThrowIfFailed(_command_queue->Signal(
		_custom_command_fence.Get(),
		_custom_command_list_fence_value
		));
	if (_custom_command_fence->GetCompletedValue() < _custom_command_list_fence_value)
	{
		DirectX::ThrowIfFailed(_custom_command_fence->SetEventOnCompletion(
			_custom_command_list_fence_value,
			_custom_command_fence_event.Get()
			));
		WaitForSingleObjectEx(
			_custom_command_fence_event.Get(),
			INFINITE,
			FALSE
			);
	}

	// Set the fence value for the next frame.
	_custom_command_list_fence_value += 1;
	return _custom_command_list.Get();
}

void DscRender::DeviceResources::CommandListFinish(ID3D12GraphicsCommandList* in_command_list)
{
	in_command_list->Close();
	_command_queue->ExecuteCommandLists(
		1,
		CommandListCast(&in_command_list)
		);
}

const bool DscRender::DeviceResources::Present()
{
	if (nullptr == _screen_size_resources)
	{
		return false;
	}
	HRESULT hr = SEVERITY_SUCCESS;
	if (false == _screen_size_resources->Present(
		hr,
		_command_queue
		))
	{
#ifdef _DEBUG
			char buff[64] = {};
			sprintf_s(
				buff,
				"Device Lost on Present: Reason code 0x%08X\n",
				static_cast < unsigned int > ((hr == DXGI_ERROR_DEVICE_REMOVED) ? _device->GetDeviceRemovedReason() : hr\
					)
				);
			OutputDebugStringA(buff);
#endif

		return false;
	}
	MoveToNextFrame();
	if (!_dxgi_factory->IsCurrent())
	{
		// Output information is cached on the DXGI Factory. If it is stale we need to create a new factory.
		DirectX::ThrowIfFailed(CreateDXGIFactory2(
			_dxgi_factory_flags,
			IID_PPV_ARGS(_dxgi_factory.ReleaseAndGetAddressOf())
			));
	}
	if (_graphics_memory)
	{
		_graphics_memory->Commit(_command_queue.Get());
	}
	return true;
}

DscRender::IRenderTarget* DscRender::DeviceResources::GetRenderTargetBackBuffer()
{
	if (nullptr != _screen_size_resources)
	{
		return _screen_size_resources->GetRenderTargetBackBuffer();
	}
	return nullptr;
}

void DscRender::DeviceResources::ForceRestore(
	ID3D12GraphicsCommandList* const in_command_list,
	IResource* const in_resource
	)
{
	in_resource->OnDeviceRestored(
		in_command_list,
		_device.Get()
		);
}
