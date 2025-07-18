#pragma once
#include "dsc_render_resource.h"

#include <dsc_common/dsc_common.h>
#include <dsc_render/i_resource.h>

namespace DscRender
{
	class DrawSystem;
	class HeapWrapperItem;
}

namespace DscRenderResource
{
	/* was ShaderTexture */
	class ShaderResource : public DscRender::IResource
	{
	public:
		ShaderResource(
			DscRender::DrawSystem* const in_draw_system,
			const std::shared_ptr<DscRender::HeapWrapperItem>& in_shader_resource,
			const D3D12_RESOURCE_DESC& in_desc,
			const D3D12_SHADER_RESOURCE_VIEW_DESC& in_shader_resource_view_desc,
			const std::vector<uint8_t>& in_data
		);
		std::shared_ptr<DscRender::HeapWrapperItem> GetHeapWrapperItem() const;

		std::vector<uint8_t>& GetData() { return _data; }
		void UploadData(
			DscRender::DrawSystem* const in_draw_system,
			ID3D12GraphicsCommandList* const in_command_list
		);

		static void UploadResource(
			DscRender::DrawSystem* const in_draw_system,
			ID3D12GraphicsCommandList* const in_command_list,
			Microsoft::WRL::ComPtr<ID3D12Resource>& in_resource,
			const D3D12_RESOURCE_DESC& in_desc,
			const size_t in_data_size,
			void* in_data
		);

		// or do we make a ShaderResourceDirtyUpload
		//void UploadIfDirty()
		//void SetDirty()
		//void SetRegionDirty(const int32 in_height_low, const int32 in_height_high);

	private:
		virtual void OnDeviceLost() override;
		virtual void OnDeviceRestored(
			ID3D12GraphicsCommandList* const in_command_list,
			ID3D12Device2* const in_device
		) override;
		virtual void OnResourceBarrier(
			ID3D12GraphicsCommandList* const in_command_list,
			D3D12_RESOURCE_STATES in_new_state
		) override;

		void UploadResource(
			ID3D12GraphicsCommandList* const in_command_list,
			ID3D12Device2* const in_device
		);

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> _resource;
		std::shared_ptr<DscRender::HeapWrapperItem> _shader_resource;
		D3D12_RESOURCE_DESC _desc; //has width, height 
		D3D12_SHADER_RESOURCE_VIEW_DESC _shader_resource_view_desc;
		std::vector<uint8_t> _data;

		D3D12_RESOURCE_STATES _current_state;

	};
}//namespace DscRenderResource
