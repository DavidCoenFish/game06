#include "render_target_format_data.h"

DscRender::RenderTargetFormatData::RenderTargetFormatData(
	const DXGI_FORMAT in_format,
	const bool in_clear_on_set,
	const DscCommon::VectorFloat4& in_clear_color
	) 
	: _format(in_format)
	, _clear_on_set(in_clear_on_set)
	, _clear_color(in_clear_color)
{
	return;
}

D3D12_CLEAR_VALUE DscRender::RenderTargetFormatData::MakeClearValue() const{
	D3D12_CLEAR_VALUE result(
		{
			_format,
			{
				_clear_color[0], _clear_color[1], _clear_color[2], _clear_color[3] 
			}
		}
	);
	return result;
}

