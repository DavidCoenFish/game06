#pragma once
#include "dsc_ui.h"
#include "i_ui_component.h"

namespace DscCommon
{
	template <typename TYPE>
	class Vector2;
	typedef Vector2<int32> VectorInt2;
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
	class UiComponentDebugFill : public IUiComponent
	{
	public:
		UiComponentDebugFill(
			const int32 in_parent_child_index,
			const std::shared_ptr<DscRenderResource::Shader>& in_shader,
			const std::shared_ptr<DscRenderResource::ShaderConstantBuffer>& in_shader_constant_buffer,
			const std::shared_ptr<DscRenderResource::GeometryGeneric>& in_full_target_quad
		);

	private:
		virtual void Draw(
			DscRenderResource::Frame& in_frame,
			DscRender::IRenderTarget& in_render_target
		) override;

		virtual const int32 GetParentChildIndex() const override; // what child index are we of out parent

		virtual void Update(const float in_time_delta) override;
		virtual void SetNode(DscDag::NodeToken in_render_node, DscDag::NodeToken in_desired_size_node, DscDag::NodeToken in_pixel_traversal_size_node, DscDag::NodeToken in_scroll_node, DscDag::NodeToken in_ui_panel_shader_constant_node) override;
		virtual void BuildUiPanelShaderConstant(TUiPanelShaderConstantBuffer& out_shader_constant_buffer) override;
		virtual std::shared_ptr<DscRender::HeapWrapperItem> GetRenderTexture() override;

	private:
		int32 _parent_child_index = 0;
		std::shared_ptr<DscRenderResource::Shader> _shader = {};
		std::shared_ptr<DscRenderResource::ShaderConstantBuffer> _shader_constant_buffer = {};
		std::shared_ptr<DscRenderResource::GeometryGeneric> _full_target_quad = {};

		DscDag::NodeToken _render_node = nullptr;
		DscDag::NodeToken _desired_size_node = nullptr;
		DscDag::NodeToken _pixel_traversal_size_node = nullptr;
		DscDag::NodeToken _scroll_node = nullptr;
		DscDag::NodeToken _ui_panel_shader_constant_node = nullptr;

	};
}