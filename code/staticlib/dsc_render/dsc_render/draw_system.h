#pragma once
#include <dsc_common/common.h>
#include "render_target_format_data.h"
#include "render_target_depth_data.h"
//#include "graphics_memory.h"

namespace DirectX
{
	class GraphicsResource;
}

namespace DscRender
{
	class DeviceResources;
	class HeapWrapper;
	class HeapWrapperItem;
	class IRenderTarget;
	class IResource;
	class ResourceList;

	class DrawSystem
	{
	public:
		DrawSystem() = delete;
		DrawSystem& operator=(const DrawSystem&) = delete;
		DrawSystem(const DrawSystem&) = delete;

		static std::unique_ptr<DrawSystem> Factory(
			const HWND in_hwnd
		);

		DrawSystem(
			const HWND in_hwnd,
			const unsigned int in_back_buffer_count = 2,
			const D3D_FEATURE_LEVEL in_d3d_feature_level = D3D_FEATURE_LEVEL_11_0,
			const unsigned int in_options = 0,
			const RenderTargetFormatData& in_target_format_data = RenderTargetFormatData(DXGI_FORMAT_B8G8R8A8_UNORM),
			const RenderTargetDepthData& in_target_depth_data = RenderTargetDepthData(),
			const int in_heap_num_descriptors_per_page = 256
			);
		~DrawSystem();

		void WaitForGpu() noexcept;
		void OnResize();
		void AddResource(IResource* const in_resource);
		void RemoveResource(IResource* const in_resource);
		const int GetBackBufferIndex() const;
		const int GetBackBufferCount() const
		{
			return _back_buffer_count;
		}

		DirectX::GraphicsResource AllocateConstant(
			const std::size_t in_size,
			void* const in_constants
		);
		DirectX::GraphicsResource AllocateUpload(
			const std::size_t in_size,
			void* const in_data_or_nullptr = nullptr,
			size_t in_alignment = 16
		);

		void ForceRestore(
			ID3D12GraphicsCommandList* const in_command_list,
			IResource* const in_resource
		);
		ID3D12Device2* const GetD3dDevice();

		ID3D12GraphicsCommandList* CreateCommandList(
			ID3D12PipelineState* const in_pipeline_state_object_or_null = nullptr
		);
		void CommandListFinish(ID3D12GraphicsCommandList* in_command_list);

		std::shared_ptr<ResourceList> MakeResourceList();
		/// Mark the resource list as finished and wait for fence before destroying it, transfer ownership
		void FinishResourceList(
			std::shared_ptr<ResourceList>& in_resource_list
		);

		void Prepare(ID3D12GraphicsCommandList*& in_command_list);
		void Present();

		IRenderTarget* GetRenderTargetBackBuffer();

		std::shared_ptr<HeapWrapperItem> MakeHeapWrapperCbvSrvUav(const int in_length = 1);
		std::shared_ptr<HeapWrapperItem> MakeHeapWrapperSampler(const int in_length = 1);
		std::shared_ptr<HeapWrapperItem> MakeHeapWrapperRenderTargetView(const int in_length = 1);
		std::shared_ptr<HeapWrapperItem> MakeHeapWrapperDepthStencilView(const int in_length = 1);
	private:
		void CreateDeviceResources();
		void RemoveCompletedResourceList();

	private:
		/// window handle which hosts the d3dx12 draw surface
		HWND _hwnd;
		unsigned int _back_buffer_count;
		D3D_FEATURE_LEVEL _d3d_feature_level;
		unsigned int _options;
		RenderTargetFormatData _target_format_data;
		RenderTargetDepthData _target_depth_data;

		/// resource internal to the draw system
		std::unique_ptr<DeviceResources> _device_resources;
		/// list of all created resources, to be told if device is reset
		std::list<IResource*> _list_resource;

		std::shared_ptr<HeapWrapper> _heap_wrapper_cbv_srv_uav;
		std::shared_ptr<HeapWrapper> _heap_wrapper_sampler;
		std::shared_ptr<HeapWrapper> _heap_wrapper_render_target_view;
		std::shared_ptr<HeapWrapper> _heap_wrapper_depth_stencil_view;

		// list of resource waiting for gpu usage to finish
		std::vector<std::shared_ptr<ResourceList>> _resource_list;
	};
} // namespace DscRender
