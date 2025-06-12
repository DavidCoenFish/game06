#pragma once
#include "dsc_onscreen_version.h"
#include <dsc_render/i_resource.h>

namespace DscCommon
{
	class FileSystem;
}

namespace DscText
{
	class TextManager;
	class TextRun;
}

namespace DscRenderResource
{
	class Frame;
	class GeometryGeneric;
	class RenderTargetTexture;
	class Shader;
}

namespace DscOnscreenVersion
{
	// not strictly a resource, just wanted to hook into the OnResize
	// draw a panel in the bottom right with the version text and build flavour
	class OnscreenVersion : public DscRender::IResource
	{
	public:
		OnscreenVersion() = delete;
		OnscreenVersion& operator=(const OnscreenVersion&) = delete;
		OnscreenVersion(const OnscreenVersion&) = delete;

		OnscreenVersion(
			DscRender::DrawSystem* const in_draw_system,
			DscCommon::FileSystem* const in_file_system,
			DscText::TextManager* const in_text_manager
		);
		void Update(
			DscRenderResource::Frame& in_frame
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
		std::unique_ptr<DscText::TextRun> _text_run;
		std::shared_ptr<DscRenderResource::RenderTargetTexture> _render_target_texture;
		std::shared_ptr<DscRenderResource::GeometryGeneric> _geometry_present;
		std::shared_ptr<DscRenderResource::Shader> _shader_present;

	};
}