#pragma once
#include "dsc_ui.h"
#include "i_ui_component.h"
#include "ui_coord.h"
#include <dsc_dag\dag_group.h>

namespace DscCommon
{
	template <typename TYPE>
	class Vector2;
	typedef Vector2<int32> VectorInt2;
}

namespace DscRender
{
	class DrawSystem;
}

namespace DscRenderResource
{
	class Frame;
	class GeometryGeneric;
	class Shader;
	class ShaderConstantBuffer;
}

namespace DscUi
{
	/*
	this is more of a margin than a padding class, it contracts the parents avaliable size
	possibly a padding class should inflate the desired size
	*/
	class UiComponentMargin : public IUiComponent
	{
	public:
		UiComponentMargin(
			const std::shared_ptr<DscRenderResource::Shader>& in_ui_panel_shader,
			const std::shared_ptr<DscRenderResource::GeometryGeneric>& in_ui_panel_geometry,
			const UiCoord& in_left,
			const UiCoord& in_top,
			const UiCoord& in_right,
			const UiCoord& in_bottom
		);

		void AddChild(
			IUiComponent* const in_child_component, // we don't keep a reference, we just set the parent child index
			DscRender::DrawSystem& in_draw_system,
			DscDag::NodeToken in_render_node, 
			DscDag::NodeToken in_ui_panel_shader_constant_node
			);

	private:
		virtual const bool IsAllowedToBeTopLevelUiComponent() override
		{
			return true;
		}

		virtual void Draw(
			DscRenderResource::Frame& in_frame,
			DscRender::IRenderTarget& in_render_target,
			const float in_ui_draw_scale
		) override;

		virtual const DscCommon::VectorInt2 GetChildAvaliableSize(const DscCommon::VectorInt2& in_parent_avaliable_size, const int32 in_child_index, const float in_ui_scale) const override;
		virtual const DscCommon::VectorInt2 GetChildGeometrySize(const DscCommon::VectorInt2& in_child_desired_size, const DscCommon::VectorInt2& in_child_avaliable_size) const override;
		virtual const DscCommon::VectorInt2 GetChildGeometryOffset(const DscCommon::VectorInt2& in_parent_avaliable_size, const int32 in_child_index, const float in_ui_scale) const override;

	private:
		std::shared_ptr<DscRenderResource::Shader> _ui_panel_shader = {};
		std::shared_ptr<DscRenderResource::GeometryGeneric> _ui_panel_geometry = {};
		UiCoord _left;
		UiCoord _top;
		UiCoord _right;
		UiCoord _bottom;

		struct ChildSlot
		{
			DscDag::NodeToken _render_node;
			DscDag::NodeToken _ui_panel_shader_constant_node;
			std::shared_ptr<DscRenderResource::ShaderConstantBuffer> _shader_constant_buffer = {};
		};
		std::vector<ChildSlot> _child_slot_array = {};

		DagGroupUiComponent _ui_component_group;

	};
}