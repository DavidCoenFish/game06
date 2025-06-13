#pragma once
#include <dsc_common/dsc_common.h>

namespace DscCommon
{
	template <typename TYPE>
	class Vector2;
	typedef Vector2<int32> VectorInt2;
}

namespace DscRender
{
	// #include "Common/DrawSystem/IResource.h"
	class IRenderTarget // : public IResource
	{
	public:
		IRenderTarget();
		// DrawSystem* const pDrawSystem);
		virtual ~IRenderTarget();
		virtual void StartRender(ID3D12GraphicsCommandList* const in_command_list, const bool in_allow_clear = true) = 0;
		virtual void EndRender(ID3D12GraphicsCommandList* const in_command_list) = 0;
		// Virtual void GetPipelineState(D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc) const = 0;
		// Virtual DXGI_FORMAT GetDepthFormat() const;
		virtual void GetFormatData(
			DXGI_FORMAT& in_depth_format,
			int& in_render_target_view_format_count,
			const DXGI_FORMAT*& in_render_target_view_format
		) const = 0;
		virtual const DscCommon::VectorInt2 GetSize() const = 0;
	};
}//namespace DscRender
