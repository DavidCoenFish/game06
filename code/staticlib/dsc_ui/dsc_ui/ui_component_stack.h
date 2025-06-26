#pragma once
#include "dsc_ui.h"
#include "i_ui_component.h"
#include "ui_coord.h"
#include "ui_enum.h"

namespace DscCommon
{
	template <typename TYPE>
	class Vector2;
	typedef Vector2<int32> VectorInt2;
	template <typename TYPE>
	class Vector4;
	typedef Vector4<float> VectorFloat4;
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
	class UiCoord;

	class UiComponentStack : public IUiComponent
	{
	public:
		UiComponentStack(
			const std::shared_ptr<DscRenderResource::Shader>& in_ui_panel_shader,
			const std::shared_ptr<DscRenderResource::GeometryGeneric>& in_ui_panel_geometry,
			const UiCoord& in_item_gap,
			const TUiFlow in_ui_flow
			// do children get the full width/height of the avaliable? or how to do attach/ pivot
		);

		void AddChild(
			IUiComponent* const in_child_component, // we don't keep a reference, we just set the parent child index
			DscRender::DrawSystem& in_draw_system,
			DscDag::NodeToken in_render_node, 
			DscDag::NodeToken in_ui_panel_shader_constant_node,
			DscDag::NodeToken in_geometry_size
			);

	private:
		virtual void Draw(
			DscRenderResource::Frame& in_frame,
			DscRender::IRenderTarget& in_render_target,
			const float in_ui_draw_scale
		) override;

		virtual void SetClearColour(const DscCommon::VectorFloat4& in_colour) override;

		virtual void SetParentChildIndex(const int32 in_parent_child_index) override;

		virtual void SetNode(
			DscDag::NodeToken in_parent_child_index,
			DscDag::NodeToken in_clear_colour_node,
			DscDag::NodeToken in_manual_scroll_x,
			DscDag::NodeToken in_manual_scroll_y
		) override;

		virtual const DscCommon::VectorInt2 GetChildAvaliableSize(const DscCommon::VectorInt2& in_parent_avaliable_size, const int32 in_child_index, const float in_ui_scale) const override;
		virtual const DscCommon::VectorInt2 GetChildGeometrySize(const DscCommon::VectorInt2& in_child_desired_size, const DscCommon::VectorInt2& in_child_avaliable_size) const override;
		virtual const DscCommon::VectorInt2 GetChildGeometryOffset(const DscCommon::VectorInt2& in_parent_avaliable_size, const int32 in_child_index, const float in_ui_scale) const override;

	private:
		std::shared_ptr<DscRenderResource::Shader> _ui_panel_shader = {};
		std::shared_ptr<DscRenderResource::GeometryGeneric> _ui_panel_geometry = {};
		UiCoord _item_gap = {};
		TUiFlow _ui_flow = {};

		struct ChildSlot
		{
			DscDag::NodeToken _render_node = {};
			DscDag::NodeToken _ui_panel_shader_constant_node = {};
			DscDag::NodeToken _geometry_size = {};
			std::shared_ptr<DscRenderResource::ShaderConstantBuffer> _shader_constant_buffer = {};
		};
		std::vector<ChildSlot> _child_slot_array = {};

		DscDag::NodeToken _parent_child_index = {};
		DscDag::NodeToken _clear_colour_node = {};
		DscDag::NodeToken _manual_scroll_x = {};
		DscDag::NodeToken _manual_scroll_y = {};

	};
}