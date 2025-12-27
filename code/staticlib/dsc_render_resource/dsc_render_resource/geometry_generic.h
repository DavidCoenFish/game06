#pragma once
#include "dsc_render_resource.h"

#include <dsc_common/dsc_common.h>
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
		static void AppendData(std::vector<uint8_t>& in_out_data, const uint8_t* const in_value, const int32_t in_size);

		template<typename IN_TYPE, int N>
		static void AppendArrayLiteral(std::vector<uint8_t>& in_out_data, IN_TYPE const (&in_value)[N])
		{
			const auto address = (uint8_t*)(&in_value[0]);
			const auto length = N * sizeof(IN_TYPE);
			AppendData(in_out_data, address, length);
		}

		template<typename IN_TYPE, int N>
		static const std::vector<uint8_t> FactoryArrayLiteral(IN_TYPE const (&in_value)[N])
		{
			std::vector<uint8_t> result;

			AppendArrayLiteral<IN_TYPE, N>(result, in_value);

			return result;
		}

		GeometryGeneric() = delete;
		GeometryGeneric& operator=(const GeometryGeneric&) = delete;
		GeometryGeneric(const GeometryGeneric&) = delete;

		GeometryGeneric(
			DscRender::DrawSystem* const in_draw_system,
			const D3D_PRIMITIVE_TOPOLOGY in_primitive_topology,
			const std::vector<D3D12_INPUT_ELEMENT_DESC>& in_input_element_desc_array,
			const std::vector<uint8_t>& in_vertex_data_raw,
			const int in_float_per_vertex // TODO: change to bytes per vertex stride, data may not even be float
		);
		void Draw(ID3D12GraphicsCommandList* const in_command_list);

		/// Only allow update if size match for in_vertex_data_raw and _vertex_data_raw
		/// this is why GetVertexDataByteSize is now exposed
		/// can not push this down as GeometryGeneric is the IResource managing the lifespan, so need to check above
		void UpdateVertexData(
			DscRender::DrawSystem* const in_draw_system,
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

		void UploadResources(
			ID3D12GraphicsCommandList* const in_command_list,
			ID3D12Device2* const in_device
		);

	private:
		const D3D_PRIMITIVE_TOPOLOGY _primitive_topology;
		const std::vector<D3D12_INPUT_ELEMENT_DESC> _input_element_desc_array;
		std::vector<uint8_t> _vertex_raw_data;
		int _float_per_vertex;
		Microsoft::WRL::ComPtr<ID3D12Resource> _vertex_buffer;
		D3D12_VERTEX_BUFFER_VIEW _vertex_buffer_view;

	};
}//namespace DscRenderResource
