#pragma once
#include <dsc_common/common.h>
#include <dsc_render/i_resource.h>
#include "shader_pipeline_state_data.h"

namespace DscRender
{
	class HeapWrapperItem;
	class ShaderConstantBuffer;

	struct ConstantBuffer;
	struct ConstantBufferInfo;
	struct ShaderResourceInfo;
	struct UnorderedAccessInfo;
}

namespace DscRenderResource
{
	class Shader : public DscRender::IResource
	{
	public:
		Shader(
			DscRender::DrawSystem* const in_draw_system,
			const DscRenderResource::ShaderPipelineStateData& in_pipeline_state_data,
			const std::shared_ptr<std::vector<uint8_t>>& in_vertex_shader_data,
			const std::shared_ptr<std::vector<uint8_t>>& in_geometry_shader_data,
			const std::shared_ptr<std::vector<uint8_t>>& in_pixel_shader_data,
			const std::vector<std::shared_ptr<DscRender::ShaderResourceInfo>>& in_array_shader_resource_info = std::vector<std::\
			shared_ptr<DscRenderResource::ShaderResourceInfo>>(),
			const std::vector<std::shared_ptr<DscRender::ConstantBufferInfo>>& in_array_shader_constants_info = std::vector<std\
			::shared_ptr<DscRenderResource::ConstantBufferInfo>>(),
			const std::shared_ptr<std::vector<uint8_t>>& in_compute_shader_data = std::shared_ptr<std::vector< \
			uint8_t>>(),
			const std::vector<std::shared_ptr<DscRenderResource::UnorderedAccessInfo>>& in_array_unordered_access_info = std::vector<std\
			::shared_ptr<DscRender::UnorderedAccessInfo>>()
		);
		virtual ~Shader();
		void SetDebugName(const std::string& in_name);
		void SetActive(
			ID3D12GraphicsCommandList* const in_command_list,
			DscRenderResource::ShaderConstantBuffer* const in_shader_constant_buffer
		);
		void SetShaderResourceViewHandle(
			const int in_index,
			const std::shared_ptr<DscRender::HeapWrapperItem>& in_shader_resource_view_handle
		);
		void SetUnorderedAccessViewHandle(
			const int in_index,
			const std::shared_ptr < DscRender::HeapWrapperItem >& in_unordered_access_view_handle
		);

		std::shared_ptr<DscRender::ShaderConstantBuffer> MakeShaderConstantBuffer(
			DscRender::DrawSystem* const in_draw_system
		) const;

	private:
		virtual void OnDeviceLost() override;
		virtual void OnDeviceRestored(
			ID3D12GraphicsCommandList* const,
			ID3D12Device2* const in_device
		) override;

	private:
		std::shared_ptr<std::vector<uint8_t>> _vertex_shader_data;
		std::shared_ptr<std::vector<uint8_t>> _geometry_shader_data;
		std::shared_ptr<std::vector<uint8_t>> _pixel_shader_data;
		std::shared_ptr<std::vector<uint8_t>> _compute_shader_data;
		ShaderPipelineStateData _pipeline_state_data;
		std::vector<std::shared_ptr<DscRenderResource::ShaderResourceInfo>> _array_shader_resource_info;
		std::vector<std::shared_ptr<DscRenderResource::ConstantBufferInfo>> _array_constants_buffer_info;
		std::vector<std::shared_ptr<DscRenderResource::UnorderedAccessInfo>> _array_unordered_access_info;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> _root_signature;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> _pipeline_state;
		std::string _debug_name;
	};
}//namespace DscRenderResource
