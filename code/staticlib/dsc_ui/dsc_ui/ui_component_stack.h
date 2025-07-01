#pragma once
#include "dsc_ui.h"
#include "i_ui_component.h"
#include "ui_coord.h"
#include "ui_enum.h"
#include <dsc_dag\dag_group.h>

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
			DscDag::NodeToken in_geometry_size,
			const UiCoord& in_primary_size,
			const UiCoord& in_primary_pivot,
			const UiCoord& in_attach_point
			);

	private:
		virtual const bool IsAllowedToBeTopLevelUiComponent() override
		{
			return false;
		}

		virtual const DscCommon::VectorInt2 ConvertAvaliableSizeToDesiredSize(const DscCommon::VectorInt2& in_parent_avaliable_size, const DscCommon::VectorInt2& in_avaliable_size, const float in_ui_scale) override;

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
		UiCoord _item_gap = {};
		TUiFlow _ui_flow = {};

		struct ChildSlot
		{
			DscDag::NodeToken _render_node = {};
			DscDag::NodeToken _ui_panel_shader_constant_node = {};
			DscDag::NodeToken _geometry_size = {};
			std::shared_ptr<DscRenderResource::ShaderConstantBuffer> _shader_constant_buffer = {};
			UiCoord _primary_size = {};
			UiCoord _primary_pivot = {};
			UiCoord _attach_point = {};
		};
		std::vector<ChildSlot> _child_slot_array = {};


	};
}