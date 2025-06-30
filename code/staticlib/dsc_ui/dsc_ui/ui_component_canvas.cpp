#include "ui_component_canvas.h"
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

DscUi::UiComponentCanvas::UiComponentCanvas(
	const std::shared_ptr<DscRenderResource::Shader>& in_ui_panel_shader,
	const std::shared_ptr<DscRenderResource::GeometryGeneric>& in_ui_panel_geometry
	)
	: _ui_panel_shader(in_ui_panel_shader)
	, _ui_panel_geometry(in_ui_panel_geometry)
{
	// nop
}

void DscUi::UiComponentCanvas::AddChild(
	IUiComponent* const in_child_component, // we don't keep a reference, we just set the parent child index
	DscRender::DrawSystem& in_draw_system,
	const VectorUiCoord2& in_child_size,
	const VectorUiCoord2& in_child_pivot,
	const VectorUiCoord2& in_attach_point,
	DscDag::NodeToken in_render_node,
	DscDag::NodeToken in_ui_panel_shader_constant_node
	)
{
	auto shader_constant_buffer = _ui_panel_shader->MakeShaderConstantBuffer(&in_draw_system);
	const int32 index = static_cast<int32>(_child_slot_array.size());
	in_child_component->SetParentChildIndex(index);
	_child_slot_array.push_back(ChildSlot({
		in_child_size,
		in_child_pivot,
		in_attach_point,
		in_render_node,
		in_ui_panel_shader_constant_node,
		shader_constant_buffer
		}));
	return;
}

/*
example: canvas, 2 children
externally provided render target (R0)
root ui node 0 (N0)
	component (C0), canvas

ui node 1 child (N1)
	component (C1), debug fill?
	render target of desired size (R1)
	calculate node that updates (R1) with correct contents of any input change

ui node 2 child (N2)
	component (C2), fill?
	render target of desired size (R2)
	calculate node that updates (R2) with correct contents of any input change

UiManager::Draw
	invokes get value on (N0)
		invokes get value on (N1)
			draw (C1)
				set render target (R1)
				debug fill
		invokes get value on (N2)
			draw (C2)
				set render target (R2)
				fill
		UiComponentCanvas::Draw (C0) 
			set render target (R0)
			use ui panel to draw texture R1 to R0
			use ui panel to draw texture R2 to R0
*/

void DscUi::UiComponentCanvas::Draw(
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

const DscCommon::VectorInt2 DscUi::UiComponentCanvas::GetChildAvaliableSize(const DscCommon::VectorInt2& in_parent_avaliable_size, const int32 in_child_index, const float in_ui_scale) const
{
	DscCommon::VectorInt2 result(in_parent_avaliable_size);
	if ((0 <= in_child_index) && (in_child_index < static_cast<int32>(_child_slot_array.size())))
	{
		result = _child_slot_array[in_child_index]._child_size.EvalueUICoord(in_parent_avaliable_size, in_ui_scale);
	}
	else
	{
		DSC_ASSERT_ALWAYS("invalid param");
	}
	return result;
}

const DscCommon::VectorInt2 DscUi::UiComponentCanvas::GetChildGeometrySize(const DscCommon::VectorInt2& in_child_desired_size, const DscCommon::VectorInt2& in_child_avaliable_size) const
{
	// for canvas, the child gets the suggested avalaible size
	(void*)&in_child_desired_size;
	return in_child_avaliable_size;
}

const DscCommon::VectorInt2 DscUi::UiComponentCanvas::GetChildGeometryOffset(const DscCommon::VectorInt2& in_parent_avaliable_size, const int32 in_child_index, const float in_ui_scale) const
{
	DscCommon::VectorInt2 result(in_parent_avaliable_size);
	if ((0 <= in_child_index) && (in_child_index < static_cast<int32>(_child_slot_array.size())))
	{
		const DscCommon::VectorInt2 avaliable_size = _child_slot_array[in_child_index]._child_size.EvalueUICoord(in_parent_avaliable_size, in_ui_scale);
		const DscCommon::VectorInt2 pivot_point = _child_slot_array[in_child_index]._child_pivot.EvalueUICoord(avaliable_size, in_ui_scale);
		const DscCommon::VectorInt2 attach_point = _child_slot_array[in_child_index]._attach_point.EvalueUICoord(in_parent_avaliable_size, in_ui_scale);

		result.Set(
			attach_point.GetX() - pivot_point.GetX(),
			attach_point.GetY() - pivot_point.GetY()
			);
	}
	else
	{
		DSC_ASSERT_ALWAYS("invalid param");
	}
	return result;
}
