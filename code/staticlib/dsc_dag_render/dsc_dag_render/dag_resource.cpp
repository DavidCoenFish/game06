#include "dag_resource.h"
#include <dsc_dag/dag_collection.h>
#include <dsc_dag/i_dag_node.h>
#include <dsc_render/draw_system.h>
#include <dsc_render/i_render_target.h>
#include <dsc_common/vector_int2.h>

std::unique_ptr<DscDagRender::DagResource> DscDagRender::DagResource::Factory(
	DscRender::DrawSystem* const in_draw_system,
	DscDag::DagCollection* const in_dag_collection
)
{
	auto dag_node_restored = in_dag_collection->CreateValue<int>(
		0, 
		DscDag::CallbackOnValueChange<int>::Function,
		nullptr
		DSC_DEBUG_ONLY(DSC_COMMA "dag_node_restored")
		);
	auto dag_node_screen_size = in_dag_collection->CreateValue<DscCommon::VectorInt2>(
		in_draw_system->GetRenderTargetBackBuffer()->GetSize(),
		DscDag::CallbackOnValueChange<DscCommon::VectorInt2>::Function,
		nullptr
		DSC_DEBUG_ONLY(DSC_COMMA "dag_node_screen_size")
		);

	return std::make_unique<DscDagRender::DagResource>(
		in_draw_system, 
		dag_node_restored,
		dag_node_screen_size
		);
}

DscDagRender::DagResource::DagResource(
	DscRender::DrawSystem* const in_draw_system,
	DscDag::NodeToken in_dag_node_restored,
	DscDag::NodeToken in_dag_node_screen_size
)
	: DscRender::IResource(in_draw_system)
	, _dag_node_restored(in_dag_node_restored)
	, _dag_node_screen_size(in_dag_node_screen_size)
{
	// nop
}

void DscDagRender::DagResource::OnDeviceLost() 
{
	//nop
}


void DscDagRender::DagResource::OnDeviceRestored(
	ID3D12GraphicsCommandList* const,// in_command_list,
	ID3D12Device2* const //in_device
	)
{
	DSC_ASSERT(nullptr != _dag_node_restored, "invalid state");
	DscDag::DagCollection::SetValueType<int32>(_dag_node_restored, 0);
	return;
}

void DscDagRender::DagResource::OnResize(
	ID3D12GraphicsCommandList* const in_command_list,
	ID3D12Device2* const in_device,
	const DscCommon::VectorInt2& in_size
	)
{
	DscRender::IResource::OnResize(in_command_list, in_device, in_size);

	DscDag::DagCollection::SetValueType(_dag_node_screen_size, in_size);
	return;
}
