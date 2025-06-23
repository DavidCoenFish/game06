#pragma once
#include "dsc_ui.h"
#include "i_ui_component.h"
#include <dsc_common\vector_float4.h>

namespace DscUi
{
	class UiComponentFill : public IUiComponent
	{
	public:
		UiComponentFill(
			const int32 in_parent_child_index,
			const DscCommon::VectorFloat4& in_fill_colour
			);

	private:
		virtual void Draw(
			DscRenderResource::Frame& in_frame,
			DscRender::IRenderTarget& in_render_target
		) override;

		virtual const DscCommon::VectorFloat4& GetClearColour() const override;
		virtual const bool SetClearColour(const DscCommon::VectorFloat4& in_colour) override;

		virtual const int32 GetParentChildIndex() const override; // what child index are we of out parent

		virtual void Update(const float in_time_delta) override;
		virtual void SetNode(DscDag::NodeToken in_render_node, DscDag::NodeToken in_desired_size_node, DscDag::NodeToken in_pixel_traversal_size_node, DscDag::NodeToken in_scroll_node, DscDag::NodeToken in_ui_panel_shader_constant_node) override;
		virtual void BuildUiPanelShaderConstant(TUiPanelShaderConstantBuffer& out_shader_constant_buffer) override;
		virtual std::shared_ptr<DscRender::HeapWrapperItem> GetRenderTexture() override;

	private:
		int32 _parent_child_index = 0;

		// if this is to be changed, move GetClearColour to a DagNode
		DscCommon::VectorFloat4 _fill_colour;

		DscDag::NodeToken _render_node = nullptr;
		DscDag::NodeToken _desired_size_node = nullptr;
		DscDag::NodeToken _pixel_traversal_size_node = nullptr;
		DscDag::NodeToken _scroll_node = nullptr;
		DscDag::NodeToken _ui_panel_shader_constant_node = nullptr;
	};
}