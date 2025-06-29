#pragma once
#include "dsc_ui.h"
#include "i_ui_component.h"
#include "ui_enum.h"
#include <dsc_dag\dag_group.h>
#include <dsc_common\vector_float4.h>

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
	class UiComponentEffectDropShadow : public IUiComponent
	{
	public:
		UiComponentEffectDropShadow() = delete;
		UiComponentEffectDropShadow& operator=(const UiComponentEffectDropShadow&) = delete;
		UiComponentEffectDropShadow(const UiComponentEffectDropShadow&) = delete;

		UiComponentEffectDropShadow(
			const std::shared_ptr<DscRenderResource::Shader>& in_shader,
			const std::shared_ptr<DscRenderResource::ShaderConstantBuffer>& in_shader_constant_buffer,
			const std::shared_ptr<DscRenderResource::GeometryGeneric>& in_full_target_quad,
			// data[offset_x[-n..n], offset_y[-n..n] // following not used yet, strength[0...1], radius (* ui_scale)[0...1]]
			const DscCommon::VectorFloat4& in_param,
			const DscCommon::VectorFloat4& in_shadow_colour
			);

		void AddChild(
			IUiComponent* const in_child_component, // we don't keep a reference, we just set the parent child index to zero
			DscDag::NodeToken in_render_node
		);

	private:
		virtual const bool IsAllowedToBeTopLevelUiComponent() override
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

	private:
		std::shared_ptr<DscRenderResource::Shader> _shader = {};
		std::shared_ptr<DscRenderResource::ShaderConstantBuffer> _shader_constant_buffer = {};
		std::shared_ptr<DscRenderResource::GeometryGeneric> _full_target_quad = {};
		DscCommon::VectorFloat4 _param = {};
		DscCommon::VectorFloat4 _shadow_colour = {};

		DscDag::NodeToken _child_render_node = {};

		DagGroupUiComponent _ui_component_group = {};

	};
}