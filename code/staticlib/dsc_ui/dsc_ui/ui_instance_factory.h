#pragma once
#include "dsc_ui.h"

namespace DscUi
{
	class IUiInstance;
	class UiManager;

	class UiInstanceFactory
	{
	public:
		typedef std::function<std::shared_ptr<IUiInstance>(const std::string&, UiManager&, DscDag::DagCollection&, DscDag::NodeToken)> TUiInstanceFactory;
		void AddFactory(
			const std::string& in_template_name,
			const TUiInstanceFactory& in_factory
			);

	private:
		std::map<std::string, TUiInstanceFactory> _name_factory_map = {};
		//std::unique_ptr<DscDag::DagCollection> _dag_collection;
		//std::unique_ptr<UiManger> _ui_manager;

	};
}



