#pragma once
#include "dsc_ui.h"
#include "i_ui_component.h"
#include "vector_ui_coord2.h"
#include <dsc_common\vector_float4.h>

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
			const int32 in_parent_child_index,
			const std::shared_ptr<DscRenderResource::Shader>& in_ui_panel_shader,
			const std::shared_ptr<DscRenderResource::GeometryGeneric>& in_ui_panel_geometry,
			const DscCommon::VectorFloat4& in_clear_colour
		);

		void AddChild(
			DscRender::DrawSystem& in_draw_system,
			const VectorUiCoord2& in_child_size, 
			const VectorUiCoord2& in_child_pivot, 
			const VectorUiCoord2& in_attach_point, 
			IUiComponent* const in_ui_component
			);

	private:
		virtual void Draw(
			DscRenderResource::Frame& in_frame,
			DscRender::IRenderTarget& in_render_target
		) override;

		virtual const DscCommon::VectorFloat4& GetClearColour() const override;
		virtual const int32 GetParentChildIndex() const override; // what child index are we of out parent

		virtual const DscCommon::VectorInt2 GetChildAvaliableSize(const DscCommon::VectorInt2& in_parent_avaliable_size, const int32 in_child_index) const override;
		virtual const DscCommon::VectorInt2 GetChildGeometrySize(const DscCommon::VectorInt2& in_child_desired_size, const DscCommon::VectorInt2& in_child_avaliable_size) const override;
		virtual const DscCommon::VectorInt2 GetChildGeometryOffset(const DscCommon::VectorInt2& in_parent_avaliable_size, const int32 in_child_index) const override;

		virtual void Update(const float in_time_delta) override;
		virtual void SetNode(DscDag::NodeToken in_render_node, DscDag::NodeToken in_desired_size_node, DscDag::NodeToken in_pixel_traversal_size_node, DscDag::NodeToken in_scroll_node, DscDag::NodeToken in_ui_panel_shader_constant_node) override;
		virtual void BuildUiPanelShaderConstant(TUiPanelShaderConstantBuffer& out_shader_constant_buffer) override;
		virtual std::shared_ptr<DscRender::HeapWrapperItem> GetRenderTexture() override;

	private:
		int32 _parent_child_index = 0;
		DscCommon::VectorFloat4 _clear_colour;
		std::shared_ptr<DscRenderResource::Shader> _ui_panel_shader = {};
		std::shared_ptr<DscRenderResource::GeometryGeneric> _ui_panel_geometry = {};

		struct ChildSlot
		{
			VectorUiCoord2 _child_size; 
			VectorUiCoord2 _child_pivot;
			VectorUiCoord2 _attach_point;
			IUiComponent* _ui_component = nullptr;
			std::shared_ptr<DscRenderResource::ShaderConstantBuffer> _shader_constant_buffer = {};
		};
		std::vector<ChildSlot> _child_slot_array = {};

		DscDag::NodeToken _render_node = nullptr;
		DscDag::NodeToken _desired_size_node = nullptr;
		DscDag::NodeToken _pixel_traversal_size_node = nullptr;
		DscDag::NodeToken _scroll_node = nullptr;
		DscDag::NodeToken _ui_panel_shader_constant_node = nullptr;

	};
}