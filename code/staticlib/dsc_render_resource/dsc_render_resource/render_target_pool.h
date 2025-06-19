#pragma once
#include "dsc_render_resource.h"
#include <dsc_common/dsc_common.h>
#include <dsc_common/vector_int2.h>

namespace DscRender
{
	class DrawSystem;
	class HeapWrapperItem;
	struct RenderTargetFormatData;
	struct RenderTargetDepthData;
}//DscRender

namespace DscRenderResource
{
	class RenderTargetTexture;
	constexpr int32 s_default_pixel_alignment = 128;

	class RenderTargetPool
	{
	public:
		RenderTargetPool() = delete;
		RenderTargetPool& operator=(const RenderTargetPool&) = delete;
		RenderTargetPool(const RenderTargetPool&) = delete;

		explicit RenderTargetPool(const int32 in_pixel_alignment);

		struct RenderTargetPoolTexture
		{
			RenderTargetPoolTexture(const std::shared_ptr<RenderTargetTexture>& in_render_target_texture, const DscCommon::VectorInt2& in_requested_size) 
				: _render_target_texture(in_render_target_texture)
				, _requested_size(in_requested_size)
			{
				//nop
			}
			std::shared_ptr<RenderTargetTexture> _render_target_texture;
			DscCommon::VectorInt2 _requested_size;
			// actual size should be in _render_target_texture?

			// return true if we can accomidate the requested size
			const bool AdjustForSize(const DscCommon::VectorInt2& in_requested_size);
		};

		std::shared_ptr<RenderTargetPoolTexture> MakeOrReuseRenderTarget(
			DscRender::DrawSystem* const in_draw_system,
			const std::vector < DscRender::RenderTargetFormatData >& in_target_format_data_array,
			const DscRender::RenderTargetDepthData& in_target_depth_data,
			const DscCommon::VectorInt2& in_size
		);

	private:
		typedef std::vector< std::shared_ptr<RenderTargetTexture>> TRenderTargetBucket;
		typedef std::map<uint32, std::unique_ptr<TRenderTargetBucket>> TMapSizeRenderTargetTexture;
		TMapSizeRenderTargetTexture* FindCreateMapSizeRenderTargetTexture(const std::size_t in_hash);
		const std::size_t MakeHash(
			const std::vector < DscRender::RenderTargetFormatData >& in_target_format_data_array,
			const DscRender::RenderTargetDepthData& in_target_depth_data
			);

	private:
		const int32 _pixel_alignment = {};
		std::map<std::size_t, std::unique_ptr<TMapSizeRenderTargetTexture>> _map_hash_map_size_render_target = {};
	};


}//namespace DscRenderResource
