#pragma once
#include "dsc_render_resource.h"

#include <dsc_common/dsc_common.h>
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
	class ShaderConstantBuffer;
	struct ConstantBufferInfo;
	struct ShaderResourceInfo;
	struct UnorderedAccessInfo;
	struct ShaderPipelineStateData;

	class Shader : public DscRender::IResource
	{
	public:
		Shader(
			DscRender::DrawSystem* const in_draw_system,
			const ShaderPipelineStateData& in_pipeline_state_data,
			const std::vector<uint8_t>& in_vertex_shader_data,
			const std::vector<uint8_t>& in_geometry_shader_data,
			const std::vector<uint8_t>& in_pixel_shader_data,
			const std::vector<std::shared_ptr<ShaderResourceInfo>>& in_array_shader_resource_info = std::vector<std::shared_ptr<ShaderResourceInfo>>(),
			const std::vector<std::shared_ptr<ConstantBufferInfo>>& in_array_shader_constants_info = std::vector<std::shared_ptr<ConstantBufferInfo>>(),
			const std::vector<uint8_t>& in_compute_shader_data = std::vector<uint8_t>(),
			const std::vector<std::shared_ptr<UnorderedAccessInfo>>& in_array_unordered_access_info = std::vector<std::shared_ptr<UnorderedAccessInfo>>()
		);
		virtual ~Shader();
		void SetDebugName(const std::string& in_name);
		void SetActive(
			ID3D12GraphicsCommandList* const in_command_list,
			ShaderConstantBuffer* const in_shader_constant_buffer
		);
		void SetShaderResourceViewHandle(
			const int in_index,
			const std::shared_ptr<DscRender::HeapWrapperItem>& in_shader_resource_view_handle
		);
		void SetUnorderedAccessViewHandle(
			const int in_index,
			const std::shared_ptr < DscRender::HeapWrapperItem >& in_unordered_access_view_handle
		);

		std::shared_ptr<ShaderConstantBuffer> MakeShaderConstantBuffer(
			DscRender::DrawSystem* const in_draw_system
		) const;

	private:
		virtual void OnDeviceLost() override;
		virtual void OnDeviceRestored(
			ID3D12GraphicsCommandList* const,
			ID3D12Device2* const in_device
		) override;

		void DeviceRestored(
			ID3D12Device2* const in_device
		);

	private:
		std::vector<uint8_t> _vertex_shader_data;
		std::vector<uint8_t> _geometry_shader_data;
		std::vector<uint8_t> _pixel_shader_data;
		std::vector<uint8_t> _compute_shader_data;
		ShaderPipelineStateData _pipeline_state_data;
		std::vector<std::shared_ptr<ShaderResourceInfo>> _array_shader_resource_info;
		std::vector<std::shared_ptr<ConstantBufferInfo>> _array_constants_buffer_info;
		std::vector<std::shared_ptr<UnorderedAccessInfo>> _array_unordered_access_info;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> _root_signature;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> _pipeline_state;
		std::string _debug_name;
	};
}//namespace DscRenderResource
