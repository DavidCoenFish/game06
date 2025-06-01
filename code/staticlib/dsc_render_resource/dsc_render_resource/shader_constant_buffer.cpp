#include "shader_constant_buffer.h"
#include <dsc_render/draw_system.h>

DscRenderResource::ShaderConstantBuffer::ShaderConstantBuffer(
	DscRender::DrawSystem* const in_draw_system,
	const std::vector<std::shared_ptr<ConstantBuffer>>& _array_constant_buffer
	) 
	: IResource(in_draw_system)
	, _array_constant_buffer(_array_constant_buffer)
{
	ID3D12Device2* const device = in_draw_system->GetD3dDevice();
	DeviceRestored(device);
}

DscRenderResource::ShaderConstantBuffer::~ShaderConstantBuffer()
{
	// Nop
}

void DscRenderResource::ShaderConstantBuffer::SetActive(
	ID3D12GraphicsCommandList* const in_command_list,
	int& in_out_root_paramter_index
	)
{
	// B0,b1,b2,...
	for (const auto& iter : _array_constant_buffer)
	{
		iter->Activate(
			in_command_list,
			in_out_root_paramter_index
			);
		in_out_root_paramter_index += 1;
	}

	return;
}

void DscRenderResource::ShaderConstantBuffer::SetConstantBufferData(
	const int in_index,
	const std::vector<float>& in_data
	)
{
	if ((0 <= in_index) && (in_index < (int) _array_constant_buffer.size()))
	{
		auto& constant_buffer =* _array_constant_buffer[in_index];
		const void* const data = &in_data[0];
		constant_buffer.UpdateData(
			data
#if defined(_DEBUG)
			,sizeof(float) * in_data.size()
#endif
		);
	}
	return;
}

void DscRenderResource::ShaderConstantBuffer::OnDeviceLost()
{
	for (auto&constant_buffer : _array_constant_buffer)
	{
		constant_buffer->DeviceLost();
	}
}

void DscRenderResource::ShaderConstantBuffer::OnDeviceRestored(
	ID3D12GraphicsCommandList* const,
	ID3D12Device2* const in_device
	)
{
	DeviceRestored(in_device);
	return;
}

void DscRenderResource::ShaderConstantBuffer::DeviceRestored(
	ID3D12Device2* const in_device
)
{
	for (auto& constant_buffer : _array_constant_buffer)
	{
		constant_buffer->DeviceRestored(in_device);
	}
	return;
}


