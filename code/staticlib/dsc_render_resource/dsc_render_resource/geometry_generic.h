#pragma once
#include <dsc_common/common.h>
#include <dsc_render/i_resource.h>

namespace DscRender
{
	class DrawSystem;
}

namespace DscRenderResource
{
	class GeometryGeneric : public DscRender::IResource
	{
	public:
		GeometryGeneric(
			DrawSystem* const in_draw_system,
			const D3D_PRIMITIVE_TOPOLOGY in_primitive_topology,
			const std::vector<D3D12_INPUT_ELEMENT_DESC>& in_input_element_desc_array,
			const std::vector<uint8_t>& in_vertex_data_raw,
			const int in_float_per_vertex
		);
		void Draw(ID3D12GraphicsCommandList* const in_command_list);

		/// Only allow update if size match for in_vertex_data_raw and _vertex_data_raw
		/// this is why GetVertexDataByteSize is now exposed
		/// can not push this down as GeometryGeneric is the IResource managing the lifespan, so need to check above
		void UpdateVertexData(
			DrawSystem* const in_draw_system,
			ID3D12GraphicsCommandList* const in_command_list,
			ID3D12Device2* const in_device,
			const std::vector<uint8_t>& in_vertex_data_raw
		);

		const size_t GetVertexDataByteSize() const;

	private:
		virtual void OnDeviceLost() override;
		virtual void OnDeviceRestored(
			ID3D12GraphicsCommandList* const in_command_list,
			ID3D12Device2* const in_device
		) override;

	private:
		const D3D_PRIMITIVE_TOPOLOGY _primitive_topology;
		const std::vector<D3D12_INPUT_ELEMENT_DESC> _input_element_desc_array;
		std::vector<uint8_t> _vertex_raw_data;
		int _float_per_vertex;
		Microsoft::WRL::ComPtr<ID3D12Resource> _vertex_buffer;
		D3D12_VERTEX_BUFFER_VIEW _vertex_buffer_view;

	};
}//namespace DscRenderResource
