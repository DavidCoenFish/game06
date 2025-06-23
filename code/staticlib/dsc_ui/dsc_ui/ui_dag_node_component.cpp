#include "ui_dag_node_component.h"
#include "i_ui_component.h"

DscUi::UiDagNodeComponent::UiDagNodeComponent(std::unique_ptr<IUiComponent>&& in_component DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name))
	: IDagNode(DSC_DEBUG_ONLY(in_debug_name))
{
	_ui_component = std::move(in_component);
}

DscUi::IUiComponent& DscUi::UiDagNodeComponent::GetComponent() const
{
	DSC_ASSERT(nullptr != _ui_component, "invalid param");
	return *_ui_component;
}

void DscUi::UiDagNodeComponent::AddOutput(DscDag::NodeToken in_nodeID)
{
	DSC_ASSERT(nullptr != in_nodeID, "invalid param");
	if (nullptr != in_nodeID)
	{
		_output.insert(in_nodeID);
		in_nodeID->MarkDirty();
	}
}

void DscUi::UiDagNodeComponent::RemoveOutput(DscDag::NodeToken in_nodeID)
{
	DSC_ASSERT(nullptr != in_nodeID, "invalid param");
	if (nullptr != in_nodeID)
	{
		_output.erase(in_nodeID);
		in_nodeID->MarkDirty();
	}
}

const bool DscUi::UiDagNodeComponent::GetHasNoLinks() const
{
	return (0 == _output.size());
}

void DscUi::UiDagNodeComponent::MarkDirty()
{
	for (auto& item : _output)
	{
		item->MarkDirty();
	}
}

const std::any& DscUi::UiDagNodeComponent::GetValue()
{
	static const std::any kDummy = {};
	return kDummy;
}
