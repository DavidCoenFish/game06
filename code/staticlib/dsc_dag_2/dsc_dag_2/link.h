#pragma once
#include "dsc_dag_2.h"
#include "node.h"
#include "calculate_component.h"
#include <dsc_common\dsc_common.h>

namespace DscDag2
{
	struct Link {
		template<
			typename IN_INPUT_TYPE,
			int32 IN_INDEX,
			typename IN_TYPE,
			typename ... IN_TYPE_LIST
			>
		static void SetInput(
			Node<IN_TYPE>& in_node,
			Node<IN_INPUT_TYPE>* const in_input_or_null = nullptr
			)
		{
			DSC_ASSERT(nullptr != in_node._calculate_component, "invalid state");
			typedef std::tuple<IN_TYPE_LIST...> TypeList;
			DSC_ASSERT(typeid(TypeList) == (*in_node._calculate_component).DebugGetTypeListTypeInfo(), "invalid call");

			typedef CalculateComponent<IN_TYPE, IN_TYPE_LIST...> TCalculateComponent;
			TCalculateComponent* const calculate_component = (TCalculateComponent*)(in_node._calculate_component.get());

			INode* old = calculate_component->SetInput<IN_INPUT_TYPE, IN_INDEX>(in_input_or_null);
			// nop if no change
			if (old != in_input_or_null)
			{
				// unlink old
				if (nullptr != old)
				{
					DSC_ASSERT(typeid(IN_INPUT_TYPE) == old->DebugGetTypeInfo(), "invalid type");
					Node<IN_INPUT_TYPE>* const old_node = (Node<IN_INPUT_TYPE>*)(old);
					DirtyComponent::Unlink(
						old_node->_dirty_component, 
						in_node._dirty_component
					);
				}
				// link new
				if (nullptr != in_input_or_null)
				{
					DirtyComponent::Link(
						in_input_or_null->_dirty_component, 
						in_node._dirty_component
					);
				}
			}
		}
	};
}

