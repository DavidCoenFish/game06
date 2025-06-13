#include <dsc_dag_render/dag_resource.h>
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
	auto dag_node_restored = in_dag_collection->CreateValue(std::any(0));
	auto dag_node_screen_size = in_dag_collection->CreateValue(std::any(in_draw_system->GetRenderTargetBackBuffer()->GetSize()));

	return std::make_unique<DscDagRender::DagResource>(
		in_draw_system, 
		dag_node_restored,
		dag_node_screen_size
		);
}

DscDagRender::DagResource::DagResource(
	DscRender::DrawSystem* const in_draw_system,
	DscDag::IDagNode* in_dag_node_restored,
	DscDag::IDagNode* in_dag_node_screen_size
)
	: DscRender::IResource(in_draw_system)
	, _dag_node_restored(in_dag_node_restored)
	, _dag_node_screen_size(in_dag_node_screen_size)
{
	// nop
}

void DscDagRender::DagResource::OnDeviceLost() {
	DscRender::IResource::OnDeviceLost();
}


void DscDagRender::DagResource::OnDeviceRestored(
	ID3D12GraphicsCommandList* const in_command_list,
	ID3D12Device2* const in_device
	)
{
	DscRender::IResource::OnDeviceRestored(in_command_list, in_device);

	DSC_ASSERT(nullptr != _dag_node_restored, "invalid state");
	_restore_count += 1;
	DscDag::DagCollection::SetValueType(_dag_node_restored, _restore_count);
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
