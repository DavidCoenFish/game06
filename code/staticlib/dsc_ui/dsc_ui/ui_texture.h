#pragma once
#include "dsc_ui.h"
#include <dsc_render_resource\render_target_pool.h>

namespace DscRender
{
	class DrawSystem;
	class IRenderTarget;
}

namespace DscRenderResource
{
	class Frame;
}

namespace DscUi
{

	/*
	hold and externally provided IRenderTarget, possibly the draw system backbuffer
	or hold a reference to a render target pool texture. expect this to be held in a dag node, which on inputs change, feeds through to update
	*/
	class UiTexture
	{
	public:
		UiTexture();

		// assert if _render_target_pool_texture is not null
		void UpdateExternalRenderTarget(
			DscRender::IRenderTarget* const in_external_render_target,
			const bool in_allow_clear_on_draw
			);

		// assert if _external_render_target is not null
		void UpdateRenderTargetPool(
			DscRender::DrawSystem& in_draw_system,
			DscRenderResource::RenderTargetPool& in_render_target_pool,
			const DscCommon::VectorInt2& in_request_size,
			const DscCommon::VectorFloat4& in_clear_colour,
			const bool in_allow_clear_on_draw
		);

		// call SetRenderTarget or SetRenderTargetTexture depending on if we have a _external_render_target or a _render_target_pool_texture
		// asserts if neither is set
		void ActivateRenderTarget(
			DscRenderResource::Frame& in_frame
			);

	private:
		bool _allow_clear_on_draw = false;

		DscRender::IRenderTarget* _external_render_target = {};
		std::shared_ptr<DscRenderResource::RenderTargetPool::RenderTargetPoolTexture> _render_target_pool_texture = {};
	};
}