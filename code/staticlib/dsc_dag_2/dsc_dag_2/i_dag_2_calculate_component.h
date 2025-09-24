#pragma once
#include "dsc_dag_2.h"

namespace DscDag2
{
	class IDag2NodeBase;

	template <typename IN_RESULT_TYPE>
	class IDag2CalculateComponent
	{
	public:
		virtual ~IDag2CalculateComponent(){};
		virtual void Update(IN_RESULT_TYPE& in_out_result) = 0;
	};
}