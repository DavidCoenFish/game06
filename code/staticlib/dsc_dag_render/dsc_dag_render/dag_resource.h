#pragma once
#include "dsc_dag_render.h"
#include <dsc_common/dsc_common.h>
#include <dsc_render/i_resource.h>

namespace DscDag
{
	class IDagNode;
	class DagCollection;
}

namespace DscRender
{
	class DrawSystem;
}

namespace DscDagRender
{
	class DagResource : public DscRender::IResource
	{
	public:
		static std::unique_ptr<DagResource> Factory(
			DscRender::DrawSystem* const in_draw_system,
			DscDag::DagCollection* const in_dag_collection
			);

		DagResource(
			DscRender::DrawSystem* const in_draw_system,
			DscDag::IDagNode* in_dag_node_restored,
			DscDag::IDagNode* in_dag_node_screen_size
			);

		DscDag::IDagNode* GetDagNodeRestored() const {
			return _dag_node_restored;
		}
		DscDag::IDagNode* GetDagNodeScreenSize() const {
			return _dag_node_screen_size;
		}
	private:
		virtual void OnDeviceLost() override;

		virtual void OnDeviceRestored(
			ID3D12GraphicsCommandList* const in_command_list,
			ID3D12Device2* const in_device
		) override;
		virtual void OnResize(
			ID3D12GraphicsCommandList* const in_command_list,
			ID3D12Device2* const in_device,
			const DscCommon::VectorInt2& in_size
		) override;
	private:
		DscDag::IDagNode* _dag_node_restored = nullptr;
		DscDag::IDagNode* _dag_node_screen_size = nullptr;
	};
} //namespace DscDagRender
