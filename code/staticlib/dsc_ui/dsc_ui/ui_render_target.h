#pragma once
#include "dsc_ui.h"
#include <dsc_render_resource\render_target_pool.h>

namespace DscCommon
{
	template <typename TYPE>
	class Vector2;
	typedef Vector2<int32> VectorInt2;
}

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
	hold an externally provided IRenderTarget, possibly the draw system backbuffer
	or hold a reference to a render target pool texture. expect this to be held in a dag node, which on inputs change, feeds through to update
	*/
	class UiRenderTarget
	{
	public:
		UiRenderTarget(
			DscRender::IRenderTarget* const in_external_render_target,
			const bool in_allow_clear_on_draw
		);
		UiRenderTarget(
			const std::shared_ptr<DscRenderResource::RenderTargetTexture>& in_render_target_texture,
			const bool in_allow_clear_on_draw
			);
		UiRenderTarget(
			const bool in_allow_clear_on_draw
			);

		// assert if _render_target_pool_texture is not null
		void UpdateExternalRenderTarget(
			DscRender::IRenderTarget* const in_external_render_target
			);

		void UpdateRenderTarget(
			const std::shared_ptr<DscRenderResource::RenderTargetTexture>& in_render_target_texture
			);

		// assert if _external_render_target is not null
		void UpdateRenderTargetPool(
			DscRender::DrawSystem& in_draw_system,
			DscRenderResource::RenderTargetPool& in_render_target_pool,
			const DscCommon::VectorInt2& in_request_size,
			const DscCommon::VectorFloat4& in_clear_colour
		);

		// call SetRenderTarget or SetRenderTargetTexture depending on if we have a _external_render_target or a _render_target_pool_texture
		// asserts if neither is set
		void ActivateRenderTarget(
			DscRenderResource::Frame& in_frame
			);
		// or just set the render target on the frame to nullptr
		//void DeactivateRenderTarget(
		//	DscRenderResource::Frame& in_frame
		//	);

		const DscCommon::VectorInt2 GetTextureSize() const;
		const DscCommon::VectorInt2 GetViewportSize() const;
		// null if we dont have a reference to a render target
		std::shared_ptr<DscRender::HeapWrapperItem> GetTexture();

	private:
		bool _allow_clear_on_draw = false;

		DscRender::IRenderTarget* _external_render_target = {};
		std::shared_ptr<DscRenderResource::RenderTargetPool::RenderTargetPoolTexture> _render_target_pool_texture = {};
		std::shared_ptr<DscRenderResource::RenderTargetTexture> _render_target_texture = {};

	};
}