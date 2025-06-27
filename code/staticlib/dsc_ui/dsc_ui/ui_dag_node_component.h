#pragma once
#include "dsc_ui.h"
#include <dsc_dag\i_dag_node.h>

namespace DscCommon
{
	template <typename TYPE>
	class Vector4;
	typedef Vector4<float> VectorFloat4;
}

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

		UiDagNodeComponent(
			std::unique_ptr<IUiComponent>&& in_component 
			DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name = "")
		);

		IUiComponent& GetComponent() const;

		// so if we want to runtime change a background colour or other property of a IUiComponent, then it gets added here as a method, 
		// and if on set the value changes on set, then we call MarkDirty
		//void SetBackgroundColour(const DscCommon::VectorFloat4& in_colour);

	private:
		virtual void AddOutput(DscDag::NodeToken in_nodeID) override;
		virtual void RemoveOutput(DscDag::NodeToken in_nodeID) override;
		virtual const bool GetHasNoLinks() const override;
		virtual void MarkDirty() override;
		virtual const std::any& GetValue() override;

#if defined(_DEBUG)
		virtual const std::string DebugPrint(const int32 in_depth = 0) const override;
#endif //#if defined(_DEBUG)

	private:
		std::set<DscDag::NodeToken> _output = {};
		std::unique_ptr<IUiComponent> _ui_component = {};
		// hold the raw pointer of the _ui_component
		std::any _value = {};
	};
}