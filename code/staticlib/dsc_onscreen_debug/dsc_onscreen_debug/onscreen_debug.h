#pragma once
#include "dsc_onscreen_debug.h"
#include <dsc_render/i_resource.h>

namespace DscCommon
{
	class FileSystem;
}

namespace DscUi
{
	class ScreenQuad;
}

namespace DscText
{
	class GlyphCollectionText;
	class TextManager;
	class Text;
}

namespace DscRenderResource
{
	class Frame;
	class GeometryGeneric;
	class RenderTargetTexture;
	class Shader;
}

namespace DscOnscreenDebug
{
	// not strictly a resource, just wanted to hook into the OnResize
	// draw a panel in the bottom left with the provided debug text
	class OnscreenDebug : public DscRender::IResource
	{
	public:
		OnscreenDebug() = delete;
		OnscreenDebug& operator=(const OnscreenDebug&) = delete;
		OnscreenDebug(const OnscreenDebug&) = delete;

		OnscreenDebug(
			DscRender::DrawSystem& in_draw_system,
			DscCommon::FileSystem& in_file_system,
			DscText::TextManager& in_text_manager
		);

		void SetText(
			const std::string& in_text
			);

		void Update(
			DscRender::DrawSystem& in_draw_system,
			DscRenderResource::Frame& in_frame,
			DscText::TextManager& in_text_manager,
			const bool in_allow_clear_backbuffer = false
		);

	private:
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored(
			ID3D12GraphicsCommandList* const in_command_list,
			ID3D12Device2* const in_device
		);
		virtual void OnResize(
			ID3D12GraphicsCommandList* const in_command_list,
			ID3D12Device2* const in_device,
			const DscCommon::VectorInt2& in_size
		);
	private:
		bool _debug_text_dirty = false;
		std::string _debug_text;
		DscText::GlyphCollectionText* _font = nullptr;
		std::unique_ptr<DscText::Text> _text_run;
		std::shared_ptr<DscRenderResource::RenderTargetTexture> _render_target_texture;
		std::shared_ptr<DscUi::ScreenQuad> _screen_quad;
		std::shared_ptr<DscRenderResource::Shader> _screen_quad_shader;

	};
}