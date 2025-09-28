#pragma once
#include "dsc_ui_2.h"

namespace DscCommon
{
	class FileSystem;
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
	class IRenderTarget;
}

namespace DscRenderResource
{
	class Frame;
	class GeometryGeneric;
	class RenderTargetPool;
	class RenderTargetTexture;
	class Shader;
	class ShaderConstantBuffer;
	class ShaderResource;
}

namespace DscText
{
	class TextManager;
	class TextRun;
}

namespace DscUi2
{
	class Manager
	{
	public:
		Manager() = delete;
		Manager& operator=(const Manager&) = delete;
		Manager(const Manager&) = delete;

		Manager(
			DscRender::DrawSystem& in_draw_system, 
			DscCommon::FileSystem& in_file_system
			);
		~Manager();

	private:
		std::shared_ptr<DscRenderResource::Shader> _ui_panel_shader = {};
		std::shared_ptr<DscRenderResource::Shader> _debug_grid_shader = {};

		// panel geometry is vertex of pos range [0 ... 1] intended to be affected by shader variables
		std::shared_ptr<DscRenderResource::GeometryGeneric> _ui_panel_geometry = {};

	};
}
