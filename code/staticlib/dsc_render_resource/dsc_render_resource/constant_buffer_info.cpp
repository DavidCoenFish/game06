#include "constant_buffer_info.h"

DscRenderResource::ConstantBufferInfo::ConstantBufferInfo(
	const std::vector<uint8_t>& in_data,
	const D3D12_SHADER_VISIBILITY in_visiblity
	) 
	: _data(in_data)
	, _visiblity(in_visiblity)
{
	// Nop
}
