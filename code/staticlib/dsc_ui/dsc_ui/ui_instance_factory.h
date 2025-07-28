#pragma once
#include "dsc_ui.h"

namespace DscDag
{
	class DagCollection;
	class IDagNode;
	typedef IDagNode* NodeToken;
}

namespace DscRender
{
	class DrawSystem;
}

namespace DscDag
{
	class DagCollection;

#if defined(_DEBUG)
	struct DagNodeGroupMetaData;
	template <typename IN_ENUM>
	const DagNodeGroupMetaData& GetDagNodeGroupMetaData(const IN_ENUM);
#endif //#if defined(_DEBUG)
}

namespace DscUi
{
	class IUiInstance;
	class UiManager;
	class UiRenderTarget;

	enum class TUiNodeGroupDataSource : uint8
	{
		TTemplateName,
		TCount
	};

	class UiInstanceFactory
	{
	public:
		typedef std::function<std::shared_ptr<IUiInstance>(
			UiInstanceFactory&,
			UiManager&, 
			DscRender::DrawSystem&,
			DscDag::DagCollection&, 
			const std::shared_ptr<UiRenderTarget>&, // root external render target or null
			DscDag::NodeToken, // data source
			DscDag::NodeToken // parent node or null
			)> TUiInstanceFactory;
		void AddFactory(
			const std::string& in_template_name,
			const TUiInstanceFactory& in_factory
			);

		// make a DagNode with a std::shared_ptr<IUiInstance> value, but how will this work with recurion, 
		// how to call again for children ui inside the context of the TUiInstanceFactory, IUiInstance own the child returned NodeToken (calc child IUiInstance)
		DscDag::NodeToken BuildInstance(
			DscDag::NodeToken in_data_source,
			UiManager& in_ui_manager,
			DscRender::DrawSystem& in_draw_system,
			DscDag::DagCollection& in_dag_collection,
			const std::shared_ptr<UiRenderTarget>& in_root_external_render_target_or_null = nullptr,
			DscDag::NodeToken in_parent_ui_node_group_or_null = nullptr
		);

	private:
		std::map<std::string, TUiInstanceFactory> _name_factory_map = {};

	};
}

#if defined(_DEBUG)
template <>
const DscDag::DagNodeGroupMetaData& DscDag::GetDagNodeGroupMetaData(const DscUi::TUiNodeGroupDataSource in_value);
#endif //#if defined(_DEBUG)




