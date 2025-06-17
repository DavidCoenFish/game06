#pragma once
#include "dsc_ui.h"
#include <dsc_dag\i_dag_node.h>

namespace DscDag
{
	class IDagNode;
	typedef IDagNode* NodeToken;
}

namespace DscUi
{
	class IUiComponent;

	class UiDagNodeComponent : public DscDag::IDagNode
	{
	public:
		UiDagNodeComponent() = delete;
		UiDagNodeComponent& operator=(const UiDagNodeComponent&) = delete;
		UiDagNodeComponent(const UiDagNodeComponent&) = delete;

		UiDagNodeComponent(std::unique_ptr<IUiComponent>&& in_component);

		IUiComponent& GetComponent() const;

	private:
		virtual void AddOutput(DscDag::NodeToken in_nodeID) override;
		virtual void RemoveOutput(DscDag::NodeToken in_nodeID) override;
		virtual const bool GetHasNoLinks() const override;
	private:
		std::set<DscDag::NodeToken> _output = {};
		std::unique_ptr<IUiComponent> _ui_component = {};
	};
}