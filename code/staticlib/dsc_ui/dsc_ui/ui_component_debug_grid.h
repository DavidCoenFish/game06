#pragma once
#include "dsc_ui.h"
#include "i_ui_component.h"
#include <dsc_dag\dag_group.h>

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
	class UiComponentDebugGrid : public IUiComponent
	{
	public:
		UiComponentDebugGrid(
			const std::shared_ptr<DscRenderResource::Shader>& in_shader,
			const std::shared_ptr<DscRenderResource::ShaderConstantBuffer>& in_shader_constant_buffer,
			const std::shared_ptr<DscRenderResource::GeometryGeneric>& in_full_target_quad
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

	private:
		std::shared_ptr<DscRenderResource::Shader> _shader = {};
		std::shared_ptr<DscRenderResource::ShaderConstantBuffer> _shader_constant_buffer = {};
		std::shared_ptr<DscRenderResource::GeometryGeneric> _full_target_quad = {};

		DagGroupUiComponent _ui_component_group;

	};
}