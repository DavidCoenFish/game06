#pragma once
#include "dsc_render_resource.h"

#include <dsc_common/dsc_common.h>
#include <dsc_common/vector_int2.h>
#include <dsc_render/i_render_target.h>
#include <dsc_render/i_resource.h>

namespace DscRender
{
	class DrawSystem;
	class HeapWrapperItem;
	struct RenderTargetFormatData;
	struct RenderTargetDepthData;
}//DscRender

namespace DscRenderResource
{
	class RenderTargetTexture : public DscRender::IRenderTarget, public DscRender::IResource
	{
	public:
		struct Resource
		{
		public:
			explicit Resource(
				const D3D12_CLEAR_VALUE& in_clear_value,
				const DXGI_FORMAT in_format = DXGI_FORMAT_UNKNOWN,
				const Microsoft::WRL::ComPtr<ID3D12Resource>& in_render_target = nullptr,
				const std::shared_ptr<DscRender::HeapWrapperItem>& in_render_target_view_descriptor = nullptr,
				const std::shared_ptr<DscRender::HeapWrapperItem>& in_shader_resource_view_descriptor = nullptr,
				const bool in_clear_color = false,
				const bool in_clear_depth = false,
				const bool in_clear_stencil = false
			);

		public:
			D3D12_CLEAR_VALUE _clear_value;
			DXGI_FORMAT _format;
			Microsoft::WRL::ComPtr < ID3D12Resource > _render_target;
			std::shared_ptr < DscRender::HeapWrapperItem > _render_target_view_descriptor;
			std::shared_ptr < DscRender::HeapWrapperItem > _shader_resource_view_descriptor;
			bool _clear_color;
			bool _clear_depth;
			bool _clear_stencil;
		};
		RenderTargetTexture(
			DscRender::DrawSystem* const in_draw_system,
			const std::vector < DscRender::RenderTargetFormatData >& in_target_format_data_array,
			const DscRender::RenderTargetDepthData& in_target_depth_data,
			const DscCommon::VectorInt2& in_size,
			const bool in_resize_with_screen = false,
			const bool in_use_sub_size  = false,
			const DscCommon::VectorInt2& in_sub_size = DscCommon::VectorInt2::s_zero // allow reused of approximatly size render targets, allow the view and sizzor rect to be optionally set
		);
		virtual ~RenderTargetTexture();
		std::shared_ptr<DscRender::HeapWrapperItem> GetShaderResourceHeapWrapperItem(const int in_index = 0) const;
		std::shared_ptr<DscRender::HeapWrapperItem> GetDepthResourceHeapWrapperItem() const;
		std::shared_ptr<DscRender::HeapWrapperItem> GetDepthShaderResourceHeapWrapperItem() const;
		void Resize(
			ID3D12GraphicsCommandList* const in_command_list,
			ID3D12Device2* const in_device,
			const DscCommon::VectorInt2& in_size
		);

	private:
		virtual void OnDeviceLost() override;
		virtual void OnDeviceRestored(
			ID3D12GraphicsCommandList* const in_command_list,
			ID3D12Device2* const in_device
		) override;
		void DeviceRestored(
			ID3D12Device2* const in_device
		);
		virtual void OnResize(
			ID3D12GraphicsCommandList* const in_command_list,
			ID3D12Device2* const in_device,
			const DscCommon::VectorInt2& in_size
		) override;
		virtual void StartRender(ID3D12GraphicsCommandList* const in_command_list, const bool in_allow_clear) override;
		virtual void EndRender(ID3D12GraphicsCommandList* const in_command_list) override;
		void TransitionResource(
			ID3D12GraphicsCommandList* const in_command_list,
			const D3D12_RESOURCE_STATES in_new_state_render_target,
			const D3D12_RESOURCE_STATES in_new_state_depth_resource
		);
		// D3D12_RESOURCE_STATE_DEPTH_WRITE	= 0x10,
		// D3D12_RESOURCE_STATE_DEPTH_READ	= 0x20,
		// Virtual void GetPipelineState(D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc) const override;
		virtual void GetFormatData(
			DXGI_FORMAT& in_depth_format,
			int& in_render_target_view_format_count,
			const DXGI_FORMAT*& in_render_target_view_format
		) const override;
		virtual const DscCommon::VectorInt2 GetSize() const override;

	public:
		std::vector < std::shared_ptr < Resource > > _target_resource_array;
		std::vector < D3D12_CPU_DESCRIPTOR_HANDLE > _array_render_target_descriptors;
		// Cache the cpu handle of the target resource
		std::shared_ptr < Resource > _depth_resource;
		D3D12_VIEWPORT _screen_viewport;
		D3D12_RECT _scissor_rect;
		D3D12_RESOURCE_STATES _current_state_render_target;
		D3D12_RESOURCE_STATES _current_state_depth_resource;

	private:
		DscCommon::VectorInt2 _size;
		bool _resize_with_screen;
		std::vector < DXGI_FORMAT > _target_format_array;
		int _id;

	};
}//namespace DscRenderResource
