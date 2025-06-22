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
		virtual const bool HasCustomGeometry() const override;
		virtual void DrawCustomGeometry(
			DscRenderResource::Frame& in_frame,
			const DscCommon::VectorInt2& in_target_size
		) override;

		virtual const int32 GetParentChildIndex() const override; // what child index are we of out parent

	private:
		int32 _parent_child_index = 0;
		std::shared_ptr<DscRenderResource::Shader> _shader = {};
		std::shared_ptr<DscRenderResource::ShaderConstantBuffer> _shader_constant_buffer = {};
		std::shared_ptr<DscRenderResource::GeometryGeneric> _full_target_quad = {};
	};
}