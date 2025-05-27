#pragma once
#include <dsc_common/common.h>
#include "render_target_format_data.h"
#include "render_target_depth_data.h"

namespace DirectX
{
	class GraphicsMemory;
} // DirectX

namespace DscRender
{
	class DrawSystem;
	class IRenderTarget;
	class IResource;
	class ScreenSizeResources;

class DeviceResources
{
public:
	DeviceResources() = delete;
	DeviceResources& operator=(const DeviceResources&) = delete;
	DeviceResources(const DeviceResources&) = delete;

	DeviceResources(
		const unsigned int in_back_buffer_count,
		const D3D_FEATURE_LEVEL in_d3d_feature_level,
		const unsigned int in_options,
		const RenderTargetFormatData& in_target_format_data,
		const RenderTargetDepthData& in_target_depth_data
		);
	~DeviceResources();
	void WaitForGpu() noexcept;
	const bool OnResize(
		DrawSystem* const in_draw_system,
		const HWND in_hwnd,
		int32& out_width,
		int32& out_Height
		);
	const int GetBackBufferIndex() const;

	void Prepare(ID3D12GraphicsCommandList*& in_command_list);
	const bool Present();

	IRenderTarget* GetRenderTargetBackBuffer();
	ID3D12Device2* const GetD3dDevice();
	ID3D12CommandQueue* const GetCommandQueue() const { return _command_queue.Get(); }

	ID3D12GraphicsCommandList* GetCustomCommandList(
		ID3D12PipelineState* const in_pipeline_state_object_or_null
	);
	void CustomCommandListFinish(ID3D12GraphicsCommandList* in_command_list);

	// Return true if size changed
	const bool CreateWindowSizeDependentResources(
		DrawSystem* const in_draw_system,
		const HWND in_hwnd,
		int32* out_width = nullptr,
		int32* out_height = nullptr
		);

	void ForceRestore(
		ID3D12GraphicsCommandList* const in_command_list,
		IResource* const in_resource
		);

private:
	void GetAdapter(
		IDXGIAdapter1** in_pp_adapter,
		const D3D_FEATURE_LEVEL in_d3d_feature_level
		);
	void MoveToNextFrame();
	void WaitForCustomCommand();

public:
	static const unsigned int c_allow_tearing = 0x1;
	static const unsigned int c_enable_hdr = 0x2;

private:
	unsigned int _back_buffer_count;
	unsigned int _options;
	RenderTargetFormatData _target_format_data;
	RenderTargetDepthData _target_depth_data;
	Microsoft::WRL::ComPtr<IDXGIFactory7> _dxgi_factory;
	DWORD _dxgi_factory_flags;
	Microsoft::WRL::ComPtr<ID3D12Device2> _device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> _command_queue;
	UINT64 _custom_command_list_fence_value;
	Microsoft::WRL::ComPtr<ID3D12Fence> _custom_command_fence;
	Microsoft::WRL::Wrappers::Event _custom_command_fence_event;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> _custom_command_allocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _custom_command_list;
	std::unique_ptr<ScreenSizeResources> _screen_size_resources;
	std::unique_ptr<DirectX::GraphicsMemory> _graphics_memory;

};
} // namespace DscRender
