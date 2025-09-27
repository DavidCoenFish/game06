#pragma once
#include "dsc_dag_2.h"
#include <dsc_common\dsc_common.h>

namespace DscDag2
{
	class DirtyComponent;

	class INode
	{
	public:
		virtual ~INode(){}

		virtual DirtyComponent& GetDirtyComponent() = 0;

#if defined(_DEBUG)
		virtual const std::type_info& DebugGetTypeInfo() const = 0;
#endif //if defined(_DEBUG)
	};

}

