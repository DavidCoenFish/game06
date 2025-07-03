#include "ui_texture.h"
#include <dsc_render\render_target_depth_data.h>
#include <dsc_render\render_target_format_data.h>
#include <dsc_render_resource\frame.h>
#include <dsc_render_resource\render_target_pool.h>

DscUi::UiTexture::UiTexture()
{
	//nop
}

// assert if _render_target_pool_texture is not null
void DscUi::UiTexture::UpdateExternalRenderTarget(
	DscRender::IRenderTarget* const in_external_render_target,
	const bool in_allow_clear_on_draw
)
{
	DSC_ASSERT(nullptr == _render_target_pool_texture, "invalid state");
	_external_render_target = in_external_render_target;
	_allow_clear_on_draw = in_allow_clear_on_draw;
	return;
}

// assert if _external_render_target is not null
void DscUi::UiTexture::UpdateRenderTargetPool(
	DscRender::DrawSystem& in_draw_system,
	DscRenderResource::RenderTargetPool& in_render_target_pool,
	const DscCommon::VectorInt2& in_request_size,
	const DscCommon::VectorFloat4& in_clear_colour,
	const bool in_allow_clear_on_draw
)
{
	DSC_ASSERT(nullptr == _external_render_target, "invalid state");
	std::vector<DscRender::RenderTargetFormatData> target_format_data_array = {};
	target_format_data_array.push_back(
		DscRender::RenderTargetFormatData(
			DXGI_FORMAT_B8G8R8A8_UNORM,
			true,
			in_clear_colour
		)
	);

	_render_target_pool_texture = in_render_target_pool.CheckOrMakeRenderTarget(
		_render_target_pool_texture,
		&in_draw_system,
		target_format_data_array,
		DscRender::RenderTargetDepthData(),
		in_request_size
	);
	_allow_clear_on_draw = in_allow_clear_on_draw;
	return;
}

// call SetRenderTarget or SetRenderTargetTexture depending on if we have a _external_render_target or a _render_target_pool_texture
// asserts if neither is set
void DscUi::UiTexture::ActivateRenderTarget(
	DscRenderResource::Frame& in_frame
)
{
	if (nullptr != _external_render_target)
	{
		in_frame.SetRenderTarget(_external_render_target, _allow_clear_on_draw);
	}
	else if (nullptr != _render_target_pool_texture)
	{
		in_frame.SetRenderTargetTexture(_render_target_pool_texture->_render_target_texture, _allow_clear_on_draw);
	}
	else
	{
		DSC_ASSERT_ALWAYS("invalid state");
	}
	return;
}
