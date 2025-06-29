#include "ui_component_stack.h"
#include "ui_manager.h"
#include <dsc_common\vector_int2.h>
#include <dsc_common\vector_float4.h>
#include <dsc_dag\dag_collection.h>
#include <dsc_dag\dag_group.h>
#include <dsc_render\i_render_target.h>
#include <dsc_render_resource\frame.h>
#include <dsc_render_resource\geometry_generic.h>
#include <dsc_render_resource\render_target_texture.h>
#include <dsc_render_resource\shader.h>
#include <dsc_render_resource\shader_constant_buffer.h>

DscUi::UiComponentStack::UiComponentStack(
	const std::shared_ptr<DscRenderResource::Shader>& in_ui_panel_shader,
	const std::shared_ptr<DscRenderResource::GeometryGeneric>& in_ui_panel_geometry,
	const UiCoord& in_item_gap,
	const TUiFlow in_ui_flow
)
	: _ui_panel_shader(in_ui_panel_shader)
	, _ui_panel_geometry(in_ui_panel_geometry)
	, _item_gap(in_item_gap)
	, _ui_flow(in_ui_flow)
{
	// nop
}

void DscUi::UiComponentStack::AddChild(
	IUiComponent* const in_child_component,
	DscRender::DrawSystem& in_draw_system,
	DscDag::NodeToken in_render_node,
	DscDag::NodeToken in_ui_panel_shader_constant_node,
	DscDag::NodeToken in_geometry_size
	)
{
	auto shader_constant_buffer = _ui_panel_shader->MakeShaderConstantBuffer(&in_draw_system);
	const int32 index = static_cast<int32>(_child_slot_array.size());
	in_child_component->SetParentChildIndex(index);
	_child_slot_array.push_back(ChildSlot({
		in_render_node,
		in_ui_panel_shader_constant_node,
		in_geometry_size,
		shader_constant_buffer
		}));
	return;
}

const DscCommon::VectorInt2 DscUi::UiComponentStack::ConvertAvaliableSizeToDesiredSize(const DscCommon::VectorInt2& in_parent_avaliable_size, const DscCommon::VectorInt2&, const float in_ui_scale)
{
	DscCommon::VectorInt2 result = {};

	if (0 < _child_slot_array.size())
	{
		const DscCommon::VectorInt2 last_geometry_offset = GetChildGeometryOffset(in_parent_avaliable_size, static_cast<int32>(_child_slot_array.size() - 1), in_ui_scale);
		const DscCommon::VectorInt2 last_geometry_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(_child_slot_array.back()._geometry_size);

		// currently works for horizontal or vertical flow, if we center or right align, then this will need a switch statement
		result.Set(
			last_geometry_offset.GetX() + last_geometry_size.GetX(),
			last_geometry_offset.GetY() + last_geometry_size.GetY()
		);
	}

	return result;
}

void DscUi::UiComponentStack::Draw(
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

void DscUi::UiComponentStack::SetClearColour(const DscCommon::VectorFloat4& in_colour)
{
	DscDag::DagCollection::SetValueType(_ui_component_group.GetNodeToken(TUiComponentGroup::TClearColourNode), in_colour);
	return;
}

void DscUi::UiComponentStack::SetParentChildIndex(const int32 in_parent_child_index)
{
	DscDag::DagCollection::SetValueType<int32>(_ui_component_group.GetNodeToken(TUiComponentGroup::TParentChildIndex), in_parent_child_index);
	return;
}

void DscUi::UiComponentStack::SetNode(const DagGroupUiComponent& in_ui_component_group)
{
	_ui_component_group = in_ui_component_group;
	return;
}

const DscCommon::VectorInt2 DscUi::UiComponentStack::GetChildAvaliableSize(const DscCommon::VectorInt2& in_parent_avaliable_size, const int32, const float) const
{
	DscCommon::VectorInt2 result = {};
	switch (_ui_flow)
	{
	default:
		break;
	case TUiFlow::THorizontal:
		result[1] = in_parent_avaliable_size[1];
		break;
	case TUiFlow::TVertical:
		result[0] = in_parent_avaliable_size[0];
		break;
	}
	return result;
}

const DscCommon::VectorInt2 DscUi::UiComponentStack::GetChildGeometrySize(const DscCommon::VectorInt2& in_child_desired_size, const DscCommon::VectorInt2& in_child_avaliable_size) const
{
	switch (_ui_flow)
	{
	default:
		break;
	case TUiFlow::THorizontal:
		return DscCommon::VectorInt2(
			in_child_desired_size.GetX(),
			in_child_avaliable_size.GetY()
		);
	case TUiFlow::TVertical:
		return DscCommon::VectorInt2(
			in_child_avaliable_size.GetX(),
			in_child_desired_size.GetY()
		);
	}
	return DscCommon::VectorInt2::s_zero;
}

const DscCommon::VectorInt2 DscUi::UiComponentStack::GetChildGeometryOffset(const DscCommon::VectorInt2& in_parent_avaliable_size, const int32 in_child_index, const float in_ui_scale) const
{
	DscCommon::VectorInt2 result(in_parent_avaliable_size);
	if ((0 <= in_child_index) && (in_child_index < static_cast<int32>(_child_slot_array.size())))
	{
		int32 trace = 0;
		for (int32 index = 0; index < in_child_index; ++index)
		{
			const DscCommon::VectorInt2 geometry_size = DscDag::DagCollection::GetValueType<DscCommon::VectorInt2>(_child_slot_array[index]._geometry_size);
			switch (_ui_flow)
			{
			default:
				break;
			case TUiFlow::THorizontal:
				trace += geometry_size.GetX();
				trace += _item_gap.Evaluate(in_parent_avaliable_size.GetX(), in_parent_avaliable_size.GetY(), in_ui_scale);
				break;
			case TUiFlow::TVertical:
				trace += geometry_size.GetY();
				trace += _item_gap.Evaluate(in_parent_avaliable_size.GetY(), in_parent_avaliable_size.GetX(), in_ui_scale);
				break;
			}
		}

		switch (_ui_flow)
		{
		default:
			break;
		case TUiFlow::THorizontal:
			result.Set(trace, 0);
			break;
		case TUiFlow::TVertical:
			result.Set(0, trace);
			break;
		}
	}
	else
	{
		DSC_ASSERT_ALWAYS("invalid param");
	}
	return result;
}
