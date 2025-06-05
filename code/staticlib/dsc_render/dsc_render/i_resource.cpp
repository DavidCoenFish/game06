#include <dsc_render/i_resource.h>
#include "draw_system.h"

#if defined(DRAW_SYSTEM_RESOURCE_ALIVE_COUNT)
int s_draw_system_resource_alive_count = 0;
#endif

DscRender::IResource::IResource(DrawSystem* const in_draw_system)
	: _draw_system(in_draw_system)
{
	if (nullptr != _draw_system)
	{
		_draw_system->AddResource(this);
	}

#if defined(DRAW_SYSTEM_RESOURCE_ALIVE_COUNT)
	s_draw_system_resource_alive_count += 1;
#endif

	return;
}

DscRender::IResource::~IResource()
{
	if (nullptr != _draw_system)
	{
		_draw_system->RemoveResource(this);
	}

#if defined(DRAW_SYSTEM_RESOURCE_ALIVE_COUNT)
	s_draw_system_resource_alive_count -= 1;
#endif

}

void DscRender::IResource::OnResize(
	ID3D12GraphicsCommandList* const,
	ID3D12Device2* const,
	const int32,
	const int32
	)
{
	return;
}

void DscRender::IResource::OnResourceBarrier(
	ID3D12GraphicsCommandList* const,
	D3D12_RESOURCE_STATES
	)
{
	return;
}

