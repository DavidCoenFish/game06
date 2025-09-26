#pragma once
#include "dsc_dag_2.h"
#include "dirty_component.h"
#include "i_calculate_component.h"
#include "i_node.h"
#include <dsc_common\dsc_common.h>
#include <dsc_common\log_system.h>

namespace DscDag2
{
	class Dag2calculateComponent;
	template <typename IN_RESULT_TYPE>
	class IDag2CalculateComponentBase;

	template <typename IN_TYPE>
	struct CallbackOnSetValue {
		static const bool Function(IN_TYPE& in_new_value, const IN_TYPE& in_old_value)
		{
			if (in_new_value != in_old_value)
			{
				in_new_value = in_old_value;
			}
			return true;
		}
	};

	template <typename IN_TYPE>
	struct CallbackNever {
		static const bool Function(IN_TYPE& in_new_value, const IN_TYPE& in_old_value)
		{
			if (in_new_value != in_old_value)
			{
				in_new_value = in_old_value;
			}
			return false;
		}
	};

	template <typename IN_TYPE>
	struct CallbackOnValueChange {
		static const bool Function(IN_TYPE& in_new_value, const IN_TYPE& in_old_value)
		{
			bool result = false;
			if (in_new_value != in_old_value)
			{
				in_new_value = in_old_value;
				result = true;
			}
			return result;
		}
	};

	template <typename IN_TYPE>
	struct CallbackNotZero {
		static const bool Function(IN_TYPE& in_new_value, const IN_TYPE& in_old_value)
		{
			if (in_new_value != in_old_value)
			{
				in_new_value = in_old_value;
			}
			return (0 != in_new_value);
		}
	};

	template <typename IN_TYPE>
	class Node : public INode
	{
	public:
		//break circular dependency with Dag2CalculateComponent for linking nodes, either exposing Dag2DirtyComponent, IDag2CalculateComponent or friend
		friend struct Link;

		Node() = delete;
		Node& operator=(const Node&) = delete;
		Node(const Node&) = delete;

		typedef const bool (*TValueAssignCallback)(
			IN_TYPE& in_new_value,
			const IN_TYPE& in_old_value
			);

		Node(
			const IN_TYPE& in_value,
			const TValueAssignCallback in_value_assign_callback
			DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name = "")
			)
			: _value_assign_callback(in_value_assign_callback)
			, _value(in_value)
			DSC_DEBUG_ONLY(DSC_COMMA _debug_name(in_debug_name))
		{
			DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_DAG_2, "Node value ctor:%p\n", this);
			DSC_ASSERT(nullptr != _value_assign_callback, "invalid state");
			return;
		}
		
		Node(
			std::unique_ptr<ICalculateComponent<IN_TYPE>>&& in_calculate_component = std::unique_ptr<ICalculateComponent<IN_TYPE>>()
			DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name = "")
			)
			: _calculate_component(std::move(in_calculate_component))
			DSC_DEBUG_ONLY(DSC_COMMA _debug_name(in_debug_name))
		{
			//DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_DAG_2, "Node calculate ctor:%p\n", this);
			DSC_ASSERT(nullptr != _calculate_component, "invalid state");
			return;
		}

		~Node()
		{
			//DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_DAG_2, "Node dtor:%p\n", this);
		}

		void SetValue(const IN_TYPE& in_value)
		{
			DSC_ASSERT(nullptr == _calculate_component, "should not call set value on a calculate node");
			bool mark_dirty = false;
			if (nullptr != _value_assign_callback)
			{
				mark_dirty = (*_value_assign_callback)(_value, in_value);
			}

			if (true == mark_dirty)
			{
				_dirty_component.MarkDirtyFlag();
			}

			return;
		}

		IN_TYPE& GetValueNonConstRef(const bool in_mark_dirty)
		{
			Update();
			if (true == in_mark_dirty)
			{
				_dirty_component.MarkDirtyFlag();
			}

			return _value;
		}

		const IN_TYPE GetValue()
		{
			Update();
			return _value;
		}

		const IN_TYPE& GetValueRef()
		{
			Update();
			return _value;
		}
	private:
#if defined(_DEBUG)
		virtual const std::type_info& DebugGetTypeInfo() const override
		{
			return typeid(IN_TYPE);
		}
#endif //if defined(_DEBUG)

		void Update()
		{
			if (nullptr == _calculate_component)
			{
				return;
			}
			if (true == _dirty_component.GetDirtyFlag())
			{
				_dirty_component.ClearDirtyFlag();
				// this is a lazy update when node is marked dirty, we possibly could do something fancy with result of calculate marking dirty again, but not the current architecture
				_calculate_component->Update(_value);
			}
			return;
		}

	private:
		std::unique_ptr<ICalculateComponent<IN_TYPE>> _calculate_component = {};
		TValueAssignCallback _value_assign_callback = nullptr;
		DirtyComponent _dirty_component = {};
		IN_TYPE _value;
		DSC_DEBUG_ONLY(std::string _debug_name = {});
	};
}

