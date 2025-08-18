#pragma once
#include "dsc_ui.h"

namespace DscCommon
{
	class FileSystem;
}

namespace DscRender
{
	class DrawSystem;
	class IRenderTarget;
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
	class CelticKnot
	{
	public:
		CelticKnot() = delete;
		CelticKnot& operator=(const CelticKnot&) = delete;
		CelticKnot(const CelticKnot&) = delete;

		CelticKnot(
			DscRender::DrawSystem& in_draw_system, 
			DscCommon::FileSystem& in_file_system
		);
		~CelticKnot();
		// make render node
/*
			DscRender::DrawSystem& in_draw_system,
			DscDag::DagCollection& in_dag_collection,
			DscDag::NodeToken in_frame_node,
			DscDag::NodeToken in_ui_render_target_node,
			DscDag::NodeToken in_ui_scale,
			DscDag::NodeToken in_component_resource_group
*/

	private:
		std::shared_ptr<DscRenderResource::Shader> _knot_shader;
		std::shared_ptr<DscRenderResource::ShaderConstantBuffer> _knot_shader_constant_buffer;

		std::shared_ptr<DscRenderResource::Shader> _data_shader;
		std::shared_ptr<DscRenderResource::ShaderConstantBuffer> _data_shader_constant_buffer;

		std::shared_ptr<DscRenderResource::Shader> _fill_knot_shader;
		std::shared_ptr<DscRenderResource::ShaderConstantBuffer> _fill_knot_shader_constant_buffer;
	};

} // DscUi
