#pragma once
#include <dsc_common/common.h>
#include <dsc_common/vector_float4.h>

namespace DscRender
{
	struct RenderTargetFormatData
	{
	public:
		explicit RenderTargetFormatData(
			const DXGI_FORMAT in_format = DXGI_FORMAT_UNKNOWN,
			const bool in_clear_on_set = false,
			const DscCommon::VectorFloat4& in_clear_color = DscCommon::VectorFloat4()
		);
		D3D12_CLEAR_VALUE MakeClearValue() const;

	public:
		DXGI_FORMAT _format;
		bool _clear_on_set;
		DscCommon::VectorFloat4 _clear_color;
	};
} //namespace DscRender
