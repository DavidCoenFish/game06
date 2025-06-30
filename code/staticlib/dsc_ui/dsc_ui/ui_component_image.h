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
	class ShaderResource;
}

namespace DscUi
{
	class UiComponentImage : public IUiComponent
	{
	public:
		UiComponentImage(
			const std::shared_ptr<DscRenderResource::Shader>& in_shader,
			const std::shared_ptr<DscRenderResource::ShaderResource>& in_texture,
			const std::shared_ptr<DscRenderResource::GeometryGeneric>& in_full_target_quad
			// bool use image size as desired size?
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

		virtual void SetParentChildIndex(const int32 in_parent_child_index) override;

		virtual void SetNode(const DagGroupUiComponent& in_ui_component_group) override;

	private:
		std::shared_ptr<DscRenderResource::Shader> _shader = {};
		std::shared_ptr<DscRenderResource::ShaderResource> _texture = {};
		std::shared_ptr<DscRenderResource::GeometryGeneric> _full_target_quad = {};

		DagGroupUiComponent _ui_component_group;

	};
}