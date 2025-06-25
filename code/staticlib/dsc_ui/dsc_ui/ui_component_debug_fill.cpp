#include "ui_component_debug_fill.h"
#include "ui_manager.h"
#include <dsc_common\vector_int2.h>
#include <dsc_dag\dag_collection.h>
#include <dsc_render\i_render_target.h>
#include <dsc_render_resource\frame.h>
#include <dsc_render_resource\geometry_generic.h>
#include <dsc_render_resource\shader.h>
#include <dsc_render_resource\shader_constant_buffer.h>

DscUi::UiComponentDebugFill::UiComponentDebugFill(
	const std::shared_ptr<DscRenderResource::Shader>& in_shader,
	const std::shared_ptr<DscRenderResource::ShaderConstantBuffer>& in_shader_constant_buffer,
	const std::shared_ptr<DscRenderResource::GeometryGeneric>& in_full_target_quad
)
	: _shader(in_shader)
	, _shader_constant_buffer(in_shader_constant_buffer)
	, _full_target_quad(in_full_target_quad)
{
	// nop
}

void DscUi::UiComponentDebugFill::Draw(
	DscRenderResource::Frame& in_frame,
	DscRender::IRenderTarget& in_render_target,
	const float
)
{
	DSC_ASSERT(nullptr != _shader, "invalid state");
	DSC_ASSERT(nullptr != _shader_constant_buffer, "invalid state");
	DSC_ASSERT(nullptr != _full_target_quad, "invalid state");

	const DscCommon::VectorInt2 viewport_size = in_render_target.GetViewportSize();
	{
		DscUi::TSizeShaderConstantBuffer& buffer = _shader_constant_buffer->GetConstant<DscUi::TSizeShaderConstantBuffer>(0);
		buffer._value[0] = static_cast<float>(viewport_size.GetX());
		buffer._value[1] = static_cast<float>(viewport_size.GetY());
	}

	in_frame.SetRenderTarget(&in_render_target);
	in_frame.SetShader(_shader, _shader_constant_buffer);
	in_frame.Draw(_full_target_quad);
	in_frame.SetRenderTarget(nullptr);
}

void DscUi::UiComponentDebugFill::SetParentChildIndex(const int32 in_parent_child_index)
{
	DSC_ASSERT(nullptr != _parent_child_index, "invalid state");
	DscDag::DagCollection::SetValueType<int32>(_parent_child_index, in_parent_child_index);
	return;
}

void DscUi::UiComponentDebugFill::SetNode(
	DscDag::NodeToken in_parent_child_index,
	DscDag::NodeToken,
	DscDag::NodeToken,
	DscDag::NodeToken
)
{
	_parent_child_index = in_parent_child_index;
}