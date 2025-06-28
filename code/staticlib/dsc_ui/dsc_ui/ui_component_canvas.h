#pragma once
#include "dsc_ui.h"
#include "i_ui_component.h"
#include "vector_ui_coord2.h"
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
	class UiComponentCanvas : public IUiComponent
	{
	public:
		UiComponentCanvas(
			const std::shared_ptr<DscRenderResource::Shader>& in_ui_panel_shader,
			const std::shared_ptr<DscRenderResource::GeometryGeneric>& in_ui_panel_geometry
		);

		void AddChild(
			IUiComponent* const in_child_component, // we don't keep a reference, we just set the parent child index
			DscRender::DrawSystem& in_draw_system,
			const VectorUiCoord2& in_child_size, 
			const VectorUiCoord2& in_child_pivot, 
			const VectorUiCoord2& in_attach_point, 
			DscDag::NodeToken in_render_node, 
			DscDag::NodeToken in_ui_panel_shader_constant_node
			);

	private:
		virtual const bool IsAllowedToBeTopLevelUiComponent() override
		{
			return true;
		}
		virtual const bool CanScroll() override
		{
			return false;
		}

		virtual void Draw(
			DscRenderResource::Frame& in_frame,
			DscRender::IRenderTarget& in_render_target,
			const float in_ui_draw_scale
		) override;

		virtual void SetClearColour(const DscCommon::VectorFloat4& in_colour) override;

		virtual void SetParentChildIndex(const int32 in_parent_child_index) override;

		virtual void SetNode(const DagGroupUiComponent& in_ui_component_group) override;

		virtual const DscCommon::VectorInt2 GetChildAvaliableSize(const DscCommon::VectorInt2& in_parent_avaliable_size, const int32 in_child_index, const float in_ui_scale) const override;
		virtual const DscCommon::VectorInt2 GetChildGeometrySize(const DscCommon::VectorInt2& in_child_desired_size, const DscCommon::VectorInt2& in_child_avaliable_size) const override;
		virtual const DscCommon::VectorInt2 GetChildGeometryOffset(const DscCommon::VectorInt2& in_parent_avaliable_size, const int32 in_child_index, const float in_ui_scale) const override;

	private:
		std::shared_ptr<DscRenderResource::Shader> _ui_panel_shader = {};
		std::shared_ptr<DscRenderResource::GeometryGeneric> _ui_panel_geometry = {};

		struct ChildSlot
		{
			VectorUiCoord2 _child_size; 
			VectorUiCoord2 _child_pivot;
			VectorUiCoord2 _attach_point;
			DscDag::NodeToken _render_node;
			DscDag::NodeToken _ui_panel_shader_constant_node;
			std::shared_ptr<DscRenderResource::ShaderConstantBuffer> _shader_constant_buffer = {};
		};
		std::vector<ChildSlot> _child_slot_array = {};

		DagGroupUiComponent _ui_component_group;
	};
}