#include "ui_component_image.h"
#include "ui_manager.h"
#include <dsc_common\vector_int2.h>
#include <dsc_dag\dag_collection.h>
#include <dsc_dag\dag_group.h>
#include <dsc_render\i_render_target.h>
#include <dsc_render_resource\frame.h>
#include <dsc_render_resource\geometry_generic.h>
#include <dsc_render_resource\shader.h>
#include <dsc_render_resource\shader_constant_buffer.h>
#include <dsc_render_resource\shader_resource.h>

DscUi::UiComponentImage::UiComponentImage(
	const std::shared_ptr<DscRenderResource::Shader>& in_shader,
	const std::shared_ptr<DscRenderResource::ShaderResource>& in_texture,
	const std::shared_ptr<DscRenderResource::GeometryGeneric>& in_full_target_quad
)
	: _shader(in_shader)
	, _texture(in_texture)
	, _full_target_quad(in_full_target_quad)
{
	// nop
}

void DscUi::UiComponentImage::Draw(
	DscRenderResource::Frame& in_frame,
	DscRender::IRenderTarget& in_render_target,
	const float
)
{
	DSC_ASSERT(nullptr != _shader, "invalid state");
	DSC_ASSERT(nullptr != _texture, "invalid state");
	DSC_ASSERT(nullptr != _full_target_quad, "invalid state");

	_shader->SetShaderResourceViewHandle(0, _texture->GetHeapWrapperItem());
	in_frame.SetRenderTarget(&in_render_target);
	in_frame.Draw(_full_target_quad);
	in_frame.SetRenderTarget(nullptr);
}

void DscUi::UiComponentImage::SetParentChildIndex(const int32 in_parent_child_index)
{
	DscDag::DagCollection::SetValueType<int32>(_ui_component_group.GetNodeToken(TUiComponentGroup::TParentChildIndex), in_parent_child_index);
	return;
}

void DscUi::UiComponentImage::SetNode(const DagGroupUiComponent& in_ui_component_group)
{
	_ui_component_group = in_ui_component_group;
}