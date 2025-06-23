#include "ui_component_canvas.h"
#include "ui_manager.h"
#include <dsc_common\vector_int2.h>
#include <dsc_dag\dag_collection.h>
#include <dsc_render\i_render_target.h>
#include <dsc_render_resource\frame.h>
#include <dsc_render_resource\geometry_generic.h>
#include <dsc_render_resource\shader.h>
#include <dsc_render_resource\shader_constant_buffer.h>

DscUi::UiComponentCanvas::UiComponentCanvas(
	const int32 in_parent_child_index,
	const std::shared_ptr<DscRenderResource::Shader>& in_ui_panel_shader,
	const std::shared_ptr<DscRenderResource::GeometryGeneric>& in_ui_panel_geometry,
	const DscCommon::VectorFloat4& in_clear_colour
	)
	: _parent_child_index(in_parent_child_index)
	, _clear_colour(in_clear_colour)
	, _ui_panel_shader(in_ui_panel_shader)
	, _ui_panel_geometry(in_ui_panel_geometry)
{
	// nop
}

void DscUi::UiComponentCanvas::AddChild(
	DscRender::DrawSystem& in_draw_system,
	const VectorUiCoord2& in_child_size,
	const VectorUiCoord2& in_child_pivot,
	const VectorUiCoord2& in_attach_point,
	IUiComponent* const in_ui_component
	)
{
	const int32 index = in_ui_component->GetParentChildIndex();
	if (_child_slot_array.size() <= static_cast<size_t>(index))
	{
		_child_slot_array.resize(index + 1);
	}
	auto shader_constant_buffer = _ui_panel_shader->MakeShaderConstantBuffer(&in_draw_system);
	_child_slot_array[index] = ChildSlot({
		in_child_size, 
		in_child_pivot, 
		in_attach_point, 
		in_ui_component,
		shader_constant_buffer
		});
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
	DscRender::IRenderTarget& in_render_target
	)
{
	const DscCommon::VectorInt2 viewport_size = in_render_target.GetViewportSize();

	for (const auto& item : _child_slot_array)
	{
		item._ui_component->GetRenderTexture();
	}

	in_frame.SetRenderTarget(&in_render_target);
	for (const auto& item : _child_slot_array)
	{
		_ui_panel_shader->SetShaderResourceViewHandle(0, item._ui_component->GetRenderTexture());
		auto& constant_buffer = item._shader_constant_buffer->GetConstant<TUiPanelShaderConstantBuffer>(0);

		item._ui_component->BuildUiPanelShaderConstant(constant_buffer);

		in_frame.SetShader(_ui_panel_shader, item._shader_constant_buffer);
		in_frame.Draw(_ui_panel_geometry);
	}
	in_frame.SetRenderTarget(nullptr);
}

const DscCommon::VectorFloat4& DscUi::UiComponentCanvas::GetClearColour() const
{
	return _clear_colour;
}

const int32 DscUi::UiComponentCanvas::GetParentChildIndex() const
{
	return _parent_child_index;
}

const DscCommon::VectorInt2 DscUi::UiComponentCanvas::GetChildAvaliableSize(const DscCommon::VectorInt2& in_parent_avaliable_size, const int32 in_child_index) const
{
	DscCommon::VectorInt2 result(in_parent_avaliable_size);
	if ((0 <= in_child_index) && (in_child_index < static_cast<int32>(_child_slot_array.size())))
	{
		result = _child_slot_array[in_child_index]._child_size.EvalueUICoord(in_parent_avaliable_size);
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

const DscCommon::VectorInt2 DscUi::UiComponentCanvas::GetChildGeometryOffset(const DscCommon::VectorInt2& in_parent_avaliable_size, const int32 in_child_index) const
{
	DscCommon::VectorInt2 result(in_parent_avaliable_size);
	if ((0 <= in_child_index) && (in_child_index < static_cast<int32>(_child_slot_array.size())))
	{
		const DscCommon::VectorInt2 avaliable_size = _child_slot_array[in_child_index]._child_size.EvalueUICoord(in_parent_avaliable_size);
		const DscCommon::VectorInt2 pivot_point = _child_slot_array[in_child_index]._child_pivot.EvalueUICoord(avaliable_size);
		const DscCommon::VectorInt2 attach_point = _child_slot_array[in_child_index]._attach_point.EvalueUICoord(in_parent_avaliable_size);

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

void DscUi::UiComponentCanvas::Update(const float in_time_delta)
{
	const float delta = std::max(0.0f, std::min(0.1f, in_time_delta));
	DscCommon::VectorInt2 pixel_range = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(_pixel_traversal_size_node);
	DscCommon::VectorFloat2 scroll = DscDag::DagCollection::GetValueType<DscCommon::VectorFloat2>(_scroll_node);
	float value_x = 0.0f;
	if (0 < pixel_range.GetX())
	{
		value_x = scroll.GetX();
		value_x += (64.0f * delta / static_cast<float>(pixel_range.GetX()));
	}
	float value_y = 0.0f;
	if (0 < pixel_range.GetY())
	{
		value_y = scroll.GetY();
		value_y += (64.0f * delta / static_cast<float>(pixel_range.GetY()));
	}

	DscDag::DagCollection::SetValueType<DscCommon::VectorFloat2>(_scroll_node, DscCommon::VectorFloat2(value_x, value_y));
}

void DscUi::UiComponentCanvas::SetNode(DscDag::NodeToken in_render_node, DscDag::NodeToken in_desired_size_node, DscDag::NodeToken in_pixel_traversal_size_node, DscDag::NodeToken in_scroll_node, DscDag::NodeToken in_ui_panel_shader_constant_node)
{
	_render_node = in_render_node;
	_desired_size_node = in_desired_size_node;
	_pixel_traversal_size_node = in_pixel_traversal_size_node;
	_scroll_node = in_scroll_node;
	_ui_panel_shader_constant_node = in_ui_panel_shader_constant_node;
}

void DscUi::UiComponentCanvas::BuildUiPanelShaderConstant(TUiPanelShaderConstantBuffer& out_shader_constant_buffer)
{
	out_shader_constant_buffer = DscDag::DagCollection::GetValueType<TUiPanelShaderConstantBuffer>(_ui_panel_shader_constant_node);
}

std::shared_ptr<DscRender::HeapWrapperItem> DscUi::UiComponentCanvas::GetRenderTexture()
{
	return DscDag::DagCollection::GetValueType<std::shared_ptr<DscRender::HeapWrapperItem>>(_render_node);
}

