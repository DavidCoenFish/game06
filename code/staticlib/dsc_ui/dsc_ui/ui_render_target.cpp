#include "ui_render_target.h"
#include <dsc_render\i_render_target.h>
#include <dsc_render\render_target_depth_data.h>
#include <dsc_render\render_target_format_data.h>
#include <dsc_render_resource\frame.h>
#include <dsc_render_resource\render_target_pool.h>
#include <dsc_render_resource\render_target_texture.h>

DscUi::UiRenderTarget::UiRenderTarget(
	const bool in_allow_clear_on_set
)
	: _allow_clear_on_set(in_allow_clear_on_set)
	, _enabled(true)
{
	//nop
}

// assert if _external_render_target is not null
void DscUi::UiRenderTarget::UpdateRenderTargetPool(
	DscRender::DrawSystem& in_draw_system,
	DscRenderResource::RenderTargetPool& in_render_target_pool,
	const DscCommon::VectorInt2& in_request_size,
	const DscCommon::VectorFloat4& in_clear_colour
)
{
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

const bool DscUi::UiRenderTarget::ActivateRenderTarget(
	DscRenderResource::Frame& in_frame
)
{
	if (false == _enabled)
	{
		return false;
	}

	if (nullptr != _render_target_pool_texture)
	{
		in_frame.SetRenderTargetTexture(_render_target_pool_texture->_render_target_texture, _allow_clear_on_set);
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

	if (nullptr != _render_target_pool_texture)
	{
		DSC_ASSERT(nullptr != _render_target_pool_texture->_render_target_texture, "invalid state");
		return _render_target_pool_texture->_render_target_texture->GetSize();
	}

	return DscCommon::VectorInt2::s_zero;
}

const DscCommon::VectorInt2 DscUi::UiRenderTarget::GetViewportSize() const
{
	if (false == _enabled)
	{
		return DscCommon::VectorInt2::s_zero;
	}

	if (nullptr != _render_target_pool_texture)
	{
		DSC_ASSERT(nullptr != _render_target_pool_texture->_render_target_texture, "invalid state");
		return _render_target_pool_texture->_render_target_texture->GetViewportSize();
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

	return nullptr;
}

