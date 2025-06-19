#include "render_target_pool.h"
#include "render_target_texture.h"
#include <dsc_render/render_target_format_data.h>
#include <dsc_render/render_target_depth_data.h>
#include <dsc_render/dsc_render.h>
#include <dsc_render/draw_system.h>
#include <dsc_render/d3dx12.h>
#include <dsc_common/math.h>

namespace
{
	const std::size_t CombineHash(const std::size_t in_lhs, const std::size_t in_rhs)
	{
		return in_lhs ^ (in_rhs << 1);
	}
}

template<>
struct std::hash<DscRender::RenderTargetFormatData>
{
	std::size_t operator()(const DscRender::RenderTargetFormatData& in_data) const noexcept
	{
		std::size_t result = std::hash<uint32>{}(in_data._format);
		result = CombineHash(result, std::hash<bool>{}(in_data._clear_on_set));
		if (in_data._clear_on_set)
		{
			result = CombineHash(result, std::hash<float>{}(in_data._clear_color[0]));
			result = CombineHash(result, std::hash<float>{}(in_data._clear_color[1]));
			result = CombineHash(result, std::hash<float>{}(in_data._clear_color[2]));
			result = CombineHash(result, std::hash<float>{}(in_data._clear_color[3]));
		}

		return result; // or use boost::hash_combine
	}
};
template<>
struct std::hash<std::vector<DscRender::RenderTargetFormatData>>
{
	std::size_t operator()(const std::vector<DscRender::RenderTargetFormatData>& in_data) const noexcept
	{
		std::size_t result = 0;
		for (const auto& item : in_data)
		{
			result = CombineHash(result, std::hash<DscRender::RenderTargetFormatData>{}(item));
		}

		return result; // or use boost::hash_combine
	}
};


template<>
struct std::hash<DscRender::RenderTargetDepthData>
{
	std::size_t operator()(const DscRender::RenderTargetDepthData& in_data) const noexcept
	{
		std::size_t result = std::hash<uint32>{}(in_data._format);
		result = CombineHash(result, std::hash<bool>{}(in_data._clear_depth_on_set));
		if (in_data._clear_depth_on_set)
		{
			result = CombineHash(result, std::hash<float>{}(in_data._clear_depth));
		}
		result = CombineHash(result, std::hash<bool>{}(in_data._clear_stencil_on_set));
		if (in_data._clear_stencil_on_set)
		{
			result = CombineHash(result, std::hash<UINT8>{}(in_data._clear_stencil));
		}
		result = CombineHash(result, std::hash<bool>{}(in_data._shader_resource));

		return result; // or use boost::hash_combine
	}
};

const bool DscRenderResource::RenderTargetPool::RenderTargetPoolTexture::AdjustForSize(const DscCommon::VectorInt2& in_requested_size)
{
	if (_requested_size == in_requested_size)
	{
		return true;
	}
	if (in_requested_size <= _render_target_texture->GetSize())
	{
		_render_target_texture->SetSubSize(true, in_requested_size);
		return true;
	}
	return false;
}


DscRenderResource::RenderTargetPool::RenderTargetPool(const int32 in_pixel_alignment)
	: _pixel_alignment(in_pixel_alignment)
{
	//nop
}

std::shared_ptr<DscRenderResource::RenderTargetPool::RenderTargetPoolTexture> DscRenderResource::RenderTargetPool::MakeOrReuseRenderTarget(
	DscRender::DrawSystem* const in_draw_system,
	const std::vector < DscRender::RenderTargetFormatData >& in_target_format_data_array,
	const DscRender::RenderTargetDepthData& in_target_depth_data,
	const DscCommon::VectorInt2& in_size
)
{
	const DscCommon::VectorInt2 ceiling_size(DscCommon::Math::Ceiling(in_size.GetX(), _pixel_alignment), DscCommon::Math::Ceiling(in_size.GetY(), _pixel_alignment));
	const uint32 size_hash = ((ceiling_size.GetX() & 0xffff) << 16) || (in_size.GetY() & 0xffff);
	const std::size_t param_hash = MakeHash(in_target_format_data_array, in_target_depth_data);

	TMapSizeRenderTargetTexture* pMap = FindCreateMapSizeRenderTargetTexture(param_hash);
	auto found = pMap->find(size_hash);
	if (found != pMap->end())
	{
		for (auto& item : *found->second)
		{
			if (1 == item.use_count())
			{
				return std::make_shared<RenderTargetPoolTexture>(item, in_size);
			}
		}
		auto render_target = std::make_shared<RenderTargetTexture>(
			in_draw_system,
			in_target_format_data_array,
			in_target_depth_data,
			ceiling_size
			);
		found->second->push_back(render_target);
		auto new_texture = std::make_shared<RenderTargetPoolTexture>(render_target, in_size);
		return new_texture;
	}

	auto render_target_outer = std::make_shared<RenderTargetTexture>(
		in_draw_system,
		in_target_format_data_array,
		in_target_depth_data,
		ceiling_size
		);
	auto bucket = std::make_unique<TRenderTargetBucket>();
	bucket->push_back(render_target_outer);

	pMap->insert(std::make_pair(size_hash, std::move(bucket)));

	auto texture = std::make_shared<RenderTargetPoolTexture>(render_target_outer, in_size);
	return texture;
}

DscRenderResource::RenderTargetPool::TMapSizeRenderTargetTexture* DscRenderResource::RenderTargetPool::FindCreateMapSizeRenderTargetTexture(const std::size_t in_hash)
{
	auto found = _map_hash_map_size_render_target.find(in_hash);
	if (found != _map_hash_map_size_render_target.end())
	{
		return found->second.get();
	}

	auto new_map = std::make_unique<TMapSizeRenderTargetTexture>();
	TMapSizeRenderTargetTexture* result = new_map.get();
	_map_hash_map_size_render_target.insert(std::make_pair(in_hash, std::move(new_map)));
	return result;
}

const std::size_t DscRenderResource::RenderTargetPool::MakeHash(
	const std::vector < DscRender::RenderTargetFormatData >& in_target_format_data_array,
	const DscRender::RenderTargetDepthData& in_target_depth_data
)
{
	std::size_t result = std::hash<std::vector < DscRender::RenderTargetFormatData >>{}(in_target_format_data_array);
	result = CombineHash(result, std::hash<DscRender::RenderTargetDepthData>{}(in_target_depth_data));
	return result;
}

