#pragma once
#include "dsc_dag_2.h"
#include "dag_2_dirty_component.h"

namespace DscDag2
{
	class Dag2calculateComponent;

	template <typename IN_TYPE>
	class Dag2Node //: public IDag2Update
	{
	public:
		Dag2Node(
			const IN_TYPE& in_value,
			const EMarkValueDirtyLogic in_dirty_logic//,
			//std::unique_ptr<Dag2CalculateComponent>&& in_calculate_component
			);
		
		//void SetValue(const IN_TYPE in_value);
		void SetValueRef(const IN_TYPE& in_value);
		IN_TYPE& GetValueNonConstRef(const bool in_mark_dirty);
		const IN_TYPE GetValue() const
		{
			return _value;
		}
		const IN_TYPE& GetValueRef() const
		{
			return _value;
		}

	private:
		// enum
		EMarkValueDirtyLogic _dirty_logic = EMarkValueDirtyLogic::TNone;
		Dag2DirtyComponent _dirty_component = {};
		IN_TYPE _value;
		//std::unique_ptr<Dag2CalculateComponent> _calculate_component = {};
	};
}

