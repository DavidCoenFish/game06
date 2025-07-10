#include "ui_render_target.h"
#include <dsc_render\i_render_target.h>
#include <dsc_render\render_target_depth_data.h>
#include <dsc_render\render_target_format_data.h>
#include <dsc_render_resource\frame.h>
#include <dsc_render_resource\render_target_pool.h>
#include <dsc_render_resource\render_target_texture.h>

DscUi::UiRenderTarget::UiRenderTarget(
	DscRender::IRenderTarget* const in_external_render_target,
	const bool in_allow_clear_on_draw
) 
	: _allow_clear_on_draw(in_allow_clear_on_draw)
	, _external_render_target(in_external_render_target)
{
	//nop
}

DscUi::UiRenderTarget::UiRenderTarget(
	const std::shared_ptr<DscRenderResource::RenderTargetTexture>& in_render_target_texture,
	const bool in_allow_clear_on_draw
)
	: _allow_clear_on_draw(in_allow_clear_on_draw)
	, _render_target_texture(in_render_target_texture)
{
	//nop
}


DscUi::UiRenderTarget::UiRenderTarget(
	const bool in_allow_clear_on_draw
)
	: _allow_clear_on_draw(in_allow_clear_on_draw)
{
	//nop
}

// assert if _render_target_pool_texture is not null
void DscUi::UiRenderTarget::UpdateExternalRenderTarget(
	DscRender::IRenderTarget* const in_external_render_target
)
{
	DSC_ASSERT(nullptr == _render_target_pool_texture, "invalid state");
	DSC_ASSERT(nullptr == _render_target_texture, "invalid state");
	_external_render_target = in_external_render_target;
	return;
}

void DscUi::UiRenderTarget::UpdateRenderTarget(
	const std::shared_ptr<DscRenderResource::RenderTargetTexture>& in_render_target_texture
)
{
	DSC_ASSERT(nullptr == _render_target_pool_texture, "invalid state");
	DSC_ASSERT(nullptr == _external_render_target, "invalid state");
	_render_target_texture = in_render_target_texture;
	return;
}

// assert if _external_render_target is not null
void DscUi::UiRenderTarget::UpdateRenderTargetPool(
	DscRender::DrawSystem& in_draw_system,
	DscRenderResource::RenderTargetPool& in_render_target_pool,
	const DscCommon::VectorInt2& in_request_size,
	const DscCommon::VectorFloat4& in_clear_colour
)
{
	DSC_ASSERT(nullptr == _external_render_target, "invalid state");
	DSC_ASSERT(nullptr == _render_target_texture, "invalid state");
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
	return;
}

// call SetRenderTarget or SetRenderTargetTexture depending on if we have a _external_render_target or a _render_target_pool_texture
// asserts if neither is set
const bool DscUi::UiRenderTarget::ActivateRenderTarget(
	DscRenderResource::Frame& in_frame
)
{
	if (false == _enabled)
	{
		return false;
	}

	if (nullptr != _external_render_target)
	{
		in_frame.SetRenderTarget(_external_render_target, _allow_clear_on_draw);
		return true;
	}
	else if (nullptr != _render_target_pool_texture)
	{
		in_frame.SetRenderTargetTexture(_render_target_pool_texture->_render_target_texture, _allow_clear_on_draw);
		return true;
	}
	else if (nullptr != _render_target_texture)
	{
		in_frame.SetRenderTargetTexture(_render_target_texture, _allow_clear_on_draw);
		return true;
	}

	// so, if the request size is zero or smaller in any dimention, _render_target_pool_texture can be null
	// so we now return true and false for if there was a valid render target set to the frame, and let client code deal
	//DSC_ASSERT_ALWAYS("invalid state");
	return false;
}

const DscCommon::VectorInt2 DscUi::UiRenderTarget::GetTextureSize() const
{
	if (false == _enabled)
	{
		return DscCommon::VectorInt2::s_zero;
	}

	if (nullptr != _external_render_target)
	{
		return _external_render_target->GetSize();
	}
	else if (nullptr != _render_target_pool_texture)
	{
		DSC_ASSERT(nullptr != _render_target_pool_texture->_render_target_texture, "invalid state");
		return _render_target_pool_texture->_render_target_texture->GetSize();
	}
	else if (nullptr != _render_target_texture)
	{
		return _render_target_texture->GetSize();
	}
	return DscCommon::VectorInt2::s_zero;
}

const DscCommon::VectorInt2 DscUi::UiRenderTarget::GetViewportSize() const
{
	if (false == _enabled)
	{
		return DscCommon::VectorInt2::s_zero;
	}

	if (nullptr != _external_render_target)
	{
		return _external_render_target->GetViewportSize();
	}
	else if (nullptr != _render_target_pool_texture)
	{
		DSC_ASSERT(nullptr != _render_target_pool_texture->_render_target_texture, "invalid state");
		return _render_target_pool_texture->_render_target_texture->GetViewportSize();
	}
	else if (nullptr != _render_target_texture)
	{
		return _render_target_texture->GetViewportSize();
	}
	return DscCommon::VectorInt2::s_zero;
}

// null if we dont have a reference to a render target
std::shared_ptr<DscRender::HeapWrapperItem> DscUi::UiRenderTarget::GetTexture()
{
	if (false == _enabled)
	{
		return nullptr;
	}

	if (nullptr != _render_target_pool_texture)
	{
		return _render_target_pool_texture->_render_target_texture->GetShaderResourceHeapWrapperItem(0);
	}
	else if (nullptr != _render_target_texture)
	{
		return _render_target_texture->GetShaderResourceHeapWrapperItem(0);
	}
	return nullptr;
}

