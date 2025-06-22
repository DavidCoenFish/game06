#include "ui_component_debug_fill.h"
#include "ui_manager.h"
#include <dsc_common\vector_int2.h>
#include <dsc_render_resource\frame.h>
#include <dsc_render_resource\geometry_generic.h>
#include <dsc_render_resource\shader.h>
#include <dsc_render_resource\shader_constant_buffer.h>

DscUi::UiComponentDebugFill::UiComponentDebugFill(
	const int32 in_parent_child_index,
	const std::shared_ptr<DscRenderResource::Shader>& in_shader,
	const std::shared_ptr<DscRenderResource::ShaderConstantBuffer>& in_shader_constant_buffer,
	const std::shared_ptr<DscRenderResource::GeometryGeneric>& in_full_target_quad
)
	: _parent_child_index(in_parent_child_index)
	, _shader(in_shader)
	, _shader_constant_buffer(in_shader_constant_buffer)
	, _full_target_quad(in_full_target_quad)
{
	// nop
}

const bool DscUi::UiComponentDebugFill::HasCustomGeometry() const
{
	return true;
}

void DscUi::UiComponentDebugFill::DrawCustomGeometry(
	DscRenderResource::Frame& in_frame,
	const DscCommon::VectorInt2& in_target_size
)
{
	DSC_ASSERT(nullptr != _shader, "invalid state");
	DSC_ASSERT(nullptr != _shader_constant_buffer, "invalid state");
	DSC_ASSERT(nullptr != _full_target_quad, "invalid state");
	
	{
		DscUi::UiManager::TSizeShaderConstantBuffer& buffer = _shader_constant_buffer->GetConstant<DscUi::UiManager::TSizeShaderConstantBuffer>(0);
		buffer._value[0] = static_cast<float>(in_target_size.GetX());
		buffer._value[1] = static_cast<float>(in_target_size.GetY());
	}

	in_frame.SetShader(_shader, _shader_constant_buffer);
	in_frame.Draw(_full_target_quad);
}

const int32 DscUi::UiComponentDebugFill::GetParentChildIndex() const
{
	return _parent_child_index;
}
