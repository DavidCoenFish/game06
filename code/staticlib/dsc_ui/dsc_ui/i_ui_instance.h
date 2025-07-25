#pragma once
#include "dsc_ui.h"

namespace DscUi
{
	class IUiInstance
	{
	public:
		virtual ~IUiInstance();

		virtual const std::string& GetTemplateName() const = 0;
		virtual void Update() = 0;

	};
}



