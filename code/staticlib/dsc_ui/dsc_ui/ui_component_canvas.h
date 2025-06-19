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
	class UiComponentCanvas : public IUiComponent
	{
	public:
		UiComponentCanvas(
			const std::shared_ptr<DscRenderResource::Shader>& in_shader,
			const std::shared_ptr<DscRenderResource::ShaderConstantBuffer>& in_shader_constant_buffer,
			const std::shared_ptr<DscRenderResource::GeometryGeneric>& in_full_target_quad
		);

		//void AddChild(const VectorUiCoord2 in_child_size, const VectorUiCoord2& in_child_pivot, const VectorUiCoord2& in_attach_point, );

	private:
		virtual void Draw(
			DscRenderResource::Frame& in_frame,
			const DscCommon::VectorInt2& in_target_size//,
			//const std::vector<DscRender::IRenderTarget*>& in_child_render_target_array
		) override;

	private:
		std::shared_ptr<DscRenderResource::Shader> _shader = {};
		std::shared_ptr<DscRenderResource::ShaderConstantBuffer> _shader_constant_buffer = {};
		std::shared_ptr<DscRenderResource::GeometryGeneric> _full_target_quad = {};

		// feels a little mixed up to have the render target for the children in the parent, but trying to allow root node. so then is non root nodes, do they have their own render targets?
		//array of data for each input [render target, size, pivot, attach]
	};
}