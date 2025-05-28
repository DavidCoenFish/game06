#include "geometry_generic.h"
#include <dsc_render/draw_system.h>
#include <dsc_render/dsc_render.h>
#include <dsc_render/heap_wrapper_item.h>
#include <dsc_render/d3dx12.h>

DscRenderResource::GeometryGeneric::GeometryGeneric(
	DscRender::DrawSystem* const in_draw_system,
	const D3D_PRIMITIVE_TOPOLOGY in_primitive_topology,
	const std::vector<D3D12_INPUT_ELEMENT_DESC>& in_input_element_desc_array,
	const std::vector<uint8_t>& in_vertex_data_raw,
	const int in_float_per_vertex
	) 
	: IResource(in_draw_system)
	, _primitive_topology(in_primitive_topology)
	, _input_element_desc_array(in_input_element_desc_array)
	, _vertex_raw_data(in_vertex_data_raw)
	, _float_per_vertex(in_float_per_vertex)
	, _vertex_buffer()
	, _vertex_buffer_view{}
{
	// Nop
}

void DscRenderResource::GeometryGeneric::Draw(ID3D12GraphicsCommandList* const in_command_list)
{
	if (_vertex_buffer)
	{
		in_command_list->IASetPrimitiveTopology(_primitive_topology);
		in_command_list->IASetVertexBuffers(
			0,
			1,
			&_vertex_buffer_view
		);
		in_command_list->DrawInstanced(
			(UINT)(_vertex_raw_data.size() / (sizeof(float) * _float_per_vertex)),
			1,
			0,
			0
		);

	}
}

void DscRenderResource::GeometryGeneric::UpdateVertexData(
	DscRender::DrawSystem* const in_draw_system,
	ID3D12GraphicsCommandList* const in_command_list,
	ID3D12Device2* const in_device,
	const std::vector<uint8_t>& in_vertex_data_raw
	)
{
	DSC_ASSERT(_vertex_raw_data.size() == in_vertex_data_raw.size(), "it is not safe to resize if the geometry is in use on a command list, possibly need a way to test if in use if we want to make this safe?");

	_vertex_raw_data = in_vertex_data_raw;
	if (0 == _vertex_raw_data.size())
	{
		_vertex_buffer.Reset();
		return;
	}

	if (nullptr == _vertex_buffer)
	{
		OnDeviceRestored(in_command_list, in_device);
	}
	else
	{
		if (nullptr == in_command_list)
		{
			return;
		}

		const int byte_vertex_size = sizeof(float) * _float_per_vertex;

		const size_t byte_total_size = _vertex_raw_data.size();
		auto buffer_resource_desc = CD3DX12_RESOURCE_DESC::Buffer(byte_total_size);
		auto upload_memory = in_draw_system->AllocateUpload(byte_total_size);

		{
			auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
				_vertex_buffer.Get(),
				D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
				D3D12_RESOURCE_STATE_COPY_DEST
			);
			in_command_list->ResourceBarrier(
				1,
				&barrier
			);
		}

		D3D12_SUBRESOURCE_DATA vertex_data = {};
		vertex_data.pData = _vertex_raw_data.data();
		vertex_data.RowPitch = byte_total_size;
		vertex_data.SlicePitch = byte_total_size;
		UpdateSubresources(
			in_command_list,
			_vertex_buffer.Get(),
			upload_memory.Resource(),
			upload_memory.ResourceOffset(),
			0,
			1,
			&vertex_data
		);

		{
			auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
				_vertex_buffer.Get(),
				D3D12_RESOURCE_STATE_COPY_DEST,
				D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
			);
			in_command_list->ResourceBarrier(
				1,
				&barrier
			);
		}


	}
	return;
}

const size_t DscRenderResource::GeometryGeneric::GetVertexDataByteSize() const
{
	return _vertex_raw_data.size();
}

void DscRenderResource::GeometryGeneric::OnDeviceLost()
{
	_vertex_buffer.Reset();
}

void DscRenderResource::GeometryGeneric::OnDeviceRestored(
	ID3D12GraphicsCommandList* const in_command_list,
	ID3D12Device2* const in_device
	)
{
	const int byte_vertex_size = sizeof(float) * _float_per_vertex;
	const int byte_total_size = _vertex_raw_data.size();
	// We could return null vertex buffer on no geometry, but what then happens on an update
	if (0 == byte_total_size)
	{
		return;
	}
	auto buffer_resource_desc = CD3DX12_RESOURCE_DESC::Buffer(byte_total_size);

	{
		auto heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		in_device->CreateCommittedResource(
			&heap_properties,
			D3D12_HEAP_FLAG_NONE,
			&buffer_resource_desc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(in_vertex_buffer.ReleaseAndGetAddressOf())
		);
		_vertex_buffer->SetName(L"GeometryVertexBuffer");
	}

	{
		if (in_command_list)
		{
			auto upload_memory = in_draw_system->AllocateUpload(byte_total_size);
			D3D12_SUBRESOURCE_DATA vertex_data = {};
			vertex_data.pData = _vertex_raw_data.data();
			vertex_data.RowPitch = byte_total_size;
			vertex_data.SlicePitch = byte_total_size;
			UpdateSubresources(
				in_command_list,
				_vertex_buffer.Get(),
				upload_memory.Resource(),
				upload_memory.ResourceOffset(),
				0,
				1,
				&vertex_data
			);
		}
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			_vertex_buffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
		);
		in_command_list->ResourceBarrier(
			1,
			&barrier
		);
	}
	_vertex_buffer_view.BufferLocation = _vertex_buffer->GetGPUVirtualAddress();
	_vertex_buffer_view.StrideInBytes = byte_vertex_size;
	_vertex_buffer_view.SizeInBytes = byte_total_size;
	return;

}

