#pragma once
#include "dsc_dag_2.h"

namespace DscDag2
{
	template <typename IN_RESULT_TYPE>
	class ICalculateComponent
	{
	public:
		virtual ~ICalculateComponent(){};
		virtual void Update(IN_RESULT_TYPE& in_out_result) = 0;

#if defined(_DEBUG)
		virtual const std::type_info& DebugGetTypeListTypeInfo() const = 0;
#endif //if defined(_DEBUG)
	};
}