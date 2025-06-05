#pragma once
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
			DscDag::IDagNode* in_dag_node_screen_width,
			DscDag::IDagNode* in_dag_node_screen_height
			);

		DscDag::IDagNode* GetDagNodeRestored() const {
			return _dag_node_restored;
		}
		DscDag::IDagNode* GetDagNodeScreenWidth() const {
			return _dag_node_screen_width;
		}
		DscDag::IDagNode* GetDagNodeScreenHeight() const {
			return _dag_node_screen_height;
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
			const int32 in_size_width,
			const int32 in_size_height
		) override;
	private:
		int32 _restore_count = 0;
		DscDag::IDagNode* _dag_node_restored = nullptr;
		DscDag::IDagNode* _dag_node_screen_width = nullptr;
		DscDag::IDagNode* _dag_node_screen_height = nullptr;
	};
} //namespace DscDagRender
