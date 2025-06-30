#include "ui_component_margin.h"
#include "ui_manager.h"
#include <dsc_common\vector_int2.h>
#include <dsc_dag\dag_collection.h>
#include <dsc_dag\dag_group.h>
#include <dsc_render\i_render_target.h>
#include <dsc_render_resource\frame.h>
#include <dsc_render_resource\geometry_generic.h>
#include <dsc_render_resource\render_target_texture.h>
#include <dsc_render_resource\shader.h>
#include <dsc_render_resource\shader_constant_buffer.h>

DscUi::UiComponentMargin::UiComponentMargin(
	const std::shared_ptr<DscRenderResource::Shader>& in_ui_panel_shader,
	const std::shared_ptr<DscRenderResource::GeometryGeneric>& in_ui_panel_geometry,
	const UiCoord& in_left,
	const UiCoord& in_top,
	const UiCoord& in_right,
	const UiCoord& in_bottom
	)
	: _ui_panel_shader(in_ui_panel_shader)
	, _ui_panel_geometry(in_ui_panel_geometry)
	, _left(in_left)
	, _top(in_top)
	, _right(in_right)
	, _bottom(in_bottom)
{
	// nop
}

void DscUi::UiComponentMargin::AddChild(
	IUiComponent* const in_child_component, // we don't keep a reference, we just set the parent child index
	DscRender::DrawSystem& in_draw_system,
	DscDag::NodeToken in_render_node,
	DscDag::NodeToken in_ui_panel_shader_constant_node
	)
{
	auto shader_constant_buffer = _ui_panel_shader->MakeShaderConstantBuffer(&in_draw_system);
	const int32 index = static_cast<int32>(_child_slot_array.size());
	in_child_component->SetParentChildIndex(index);
	_child_slot_array.push_back(ChildSlot({
		in_render_node,
		in_ui_panel_shader_constant_node,
		shader_constant_buffer
		}));
	return;
}

void DscUi::UiComponentMargin::Draw(
	DscRenderResource::Frame& in_frame,
	DscRender::IRenderTarget& in_render_target,
	const float
	)
{
	in_frame.SetRenderTarget(&in_render_target);
	for (const auto& item : _child_slot_array)
	{
		std::shared_ptr<DscRenderResource::RenderTargetTexture> render_texture = DscDag::DagCollection::GetValueType<std::shared_ptr<DscRenderResource::RenderTargetTexture>>(item._render_node);
		_ui_panel_shader->SetShaderResourceViewHandle(0, render_texture->GetShaderResourceHeapWrapperItem(0));
		
		auto& constant_buffer = item._shader_constant_buffer->GetConstant<TUiPanelShaderConstantBuffer>(0);
		constant_buffer = DscDag::DagCollection::GetValueType<TUiPanelShaderConstantBuffer>(item._ui_panel_shader_constant_node);

		in_frame.SetShader(_ui_panel_shader, item._shader_constant_buffer);
		in_frame.Draw(_ui_panel_geometry);
	}
	in_frame.SetRenderTarget(nullptr);
}

const DscCommon::VectorInt2 DscUi::UiComponentMargin::GetChildAvaliableSize(const DscCommon::VectorInt2& in_parent_avaliable_size, const int32, const float in_ui_scale) const
{
	const int32 width_taken_by_padding = _left.Evaluate(in_parent_avaliable_size.GetX(), in_parent_avaliable_size.GetY(), in_ui_scale) +
		_right.Evaluate(in_parent_avaliable_size.GetX(), in_parent_avaliable_size.GetY(), in_ui_scale);
	const int32 height_taken_by_padding = _top.Evaluate(in_parent_avaliable_size.GetY(), in_parent_avaliable_size.GetX(), in_ui_scale) +
		_bottom.Evaluate(in_parent_avaliable_size.GetY(), in_parent_avaliable_size.GetX(), in_ui_scale);

	DscCommon::VectorInt2 result(
		in_parent_avaliable_size.GetX() - width_taken_by_padding,
		in_parent_avaliable_size.GetY() - height_taken_by_padding
		);
	return result;
}

const DscCommon::VectorInt2 DscUi::UiComponentMargin::GetChildGeometrySize(const DscCommon::VectorInt2& in_child_desired_size, const DscCommon::VectorInt2& in_child_avaliable_size) const
{
	// for padding, the child gets the suggested avalaible size
	(void*)&in_child_desired_size;
	return in_child_avaliable_size;
}

const DscCommon::VectorInt2 DscUi::UiComponentMargin::GetChildGeometryOffset(const DscCommon::VectorInt2& in_parent_avaliable_size, const int32, const float in_ui_scale) const
{
	const int32 left = _left.Evaluate(in_parent_avaliable_size.GetX(), in_parent_avaliable_size.GetY(), in_ui_scale);
	const int32 top = _top.Evaluate(in_parent_avaliable_size.GetY(), in_parent_avaliable_size.GetX(), in_ui_scale);
	DscCommon::VectorInt2 result(left, top);
	return result;
}
