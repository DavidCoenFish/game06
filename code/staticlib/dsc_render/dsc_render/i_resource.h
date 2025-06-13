#pragma once
#include <dsc_common/dsc_common.h>

//#define DRAW_SYSTEM_RESOURCE_ALIVE_COUNT
namespace DscCommon
{
	template <typename TYPE>
	class Vector2;
	typedef Vector2<int32> VectorInt2;
}

namespace DscRender
{
	class DrawSystem;
	class IResource
	{
	public:
		IResource(DrawSystem* const in_draw_system);
		virtual ~IResource();
		virtual void OnDeviceLost() = 0;
		virtual void OnDeviceRestored(
			ID3D12GraphicsCommandList* const in_command_list,
			ID3D12Device2* const in_device
		) = 0;
		virtual void OnResize(
			ID3D12GraphicsCommandList* const in_command_list,
			ID3D12Device2* const in_device,
			const DscCommon::VectorInt2& in_size
		);
		virtual void OnResourceBarrier(
			ID3D12GraphicsCommandList* const in_command_list,
			D3D12_RESOURCE_STATES in_after_state
		);

	protected:
		DrawSystem* _draw_system;
	};
}//namespace DscRender
