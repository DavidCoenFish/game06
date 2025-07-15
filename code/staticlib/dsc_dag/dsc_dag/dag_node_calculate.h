#pragma once
#include "dsc_dag.h"
#include "i_dag_node.h"
#include <dsc_common\debug_print.h>

namespace DscDag
{
	//template <typename IN_TYPE DSC_DEBUG_ONLY(DSC_COMMA typename IN_DEBUG_PRINT = DscCommon::DebugPrintNone<IN_TYPE>)>
	template <typename IN_TYPE>
	class DagNodeCalculate : public IDagNode
	{
	public:
		DagNodeCalculate() = delete;
		DagNodeCalculate& operator=(const DagNodeCalculate&) = delete;
		DagNodeCalculate(const DagNodeCalculate&) = delete;

		typedef std::function<void(IN_TYPE&, std::set<NodeToken>&, std::vector<NodeToken>&)> TCalculateFunction;
		DagNodeCalculate(const TCalculateFunction& in_calculate_function DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name = ""))
			: IDagNode(DSC_DEBUG_ONLY(in_debug_name))
			, _calculate_function(in_calculate_function)
		{
			//nop
		}
		const IN_TYPE& GetValue()
		{
			Update();
			return _value;
		}

	private:
		virtual void MarkDirty() override
		{
			if (true == _dirty)
			{
				return;
			}
			_dirty = true;
			for (auto& item : _output)
			{
				item->MarkDirty();
			}
		}

		virtual void Update() override
		{
			if (true == _dirty)
			{
				_dirty = false;

				// so, currently we could have input that is not used in the calculate, and would keep it's dirty flag as GetValue may not be called.
				// if it was dirtied again, this node would not be marked dirty, skipped as it is already dirty
				// so, on each call to calculate, explicity GetValue on all inputs to flush the dirty state
				for (auto& item : _input)
				{
					if (nullptr != item)
					{
						item->Update();
					}
				}
				for (auto& item : _index_input)
				{
					if (nullptr != item)
					{
						item->Update();
					}
				}

				_calculate_function(_value, _input, _index_input);
			}
		}

		virtual void AddOutput(NodeToken in_nodeID) override
		{
			DSC_ASSERT(nullptr != in_nodeID, "invalid param");
			if (nullptr != in_nodeID)
			{
				in_nodeID->MarkDirty();
			}
			_output.insert(in_nodeID);
		}

		virtual void RemoveOutput(NodeToken in_nodeID) override
		{
			DSC_ASSERT(nullptr != in_nodeID, "invalid param");
			if (nullptr != in_nodeID)
			{
				in_nodeID->MarkDirty();
			}
			_output.erase(in_nodeID);
		}

		virtual const bool SetIndexInput(const int32 in_index, NodeToken in_nodeID = NullToken) override
		{
			DSC_ASSERT(0 <= in_index, "invalid param");
			// null in_nodeID allowed
			if (static_cast<int32>(_index_input.size()) <= in_index)
			{
				_index_input.resize(in_index + 1);
			}
			bool result = false;
			if (_index_input[in_index] != in_nodeID)
			{
				result = true;
				_index_input[in_index] = in_nodeID;
				MarkDirty();
			}
			return result;
		}

		virtual void AddInput(NodeToken in_nodeID) override
		{
			DSC_ASSERT(nullptr != in_nodeID, "invalid param");
			_input.insert(in_nodeID);
			MarkDirty();
		}

		virtual void RemoveInput(NodeToken in_nodeID) override
		{
			DSC_ASSERT(nullptr != in_nodeID, "invalid param");
			_input.erase(in_nodeID);
			MarkDirty();
		}

		virtual const bool GetHasNoLinks() const override
		{
			if (0 != _output.size())
			{
				return false;
			}
			if (0 != _input.size())
			{
				return false;
			}
			for (const auto& item : _index_input)
			{
				if (nullptr != item)
				{
					return false;
				}
			}
			return true;
		}

		virtual void UnlinkInputs() override
		{
			for (const auto& item : _index_input)
			{
				if (nullptr != item)
				{
					item->RemoveOutput(this);
				}
			}
			_index_input.clear();
			for (const auto& item : _input)
			{
				if (nullptr != item)
				{
					item->RemoveOutput(this);
				}
			}
			_input.clear();
		}

		virtual const std::type_info& GetTypeInfo() const override
		{
			return typeid(IN_TYPE);
		}

#if defined(_DEBUG)
		virtual const std::string DebugPrint(const int32 in_depth = 0) const override
		{
			std::string result = DscCommon::DebugPrint::TabDepth(in_depth);

			result += "Calculate:\"";
			result += _debug_name;
			result += "\" dirty:" + std::to_string(_dirty);
			result += " type:";
			result += typeid(IN_TYPE).name();
			if (nullptr != _s_debug_print_value)
			{
				result += " value:";
				result += _s_debug_print_value(_value);
			}
			result += "\n";

			if (0 < _input.size())
			{
				result += DscCommon::DebugPrint::TabDepth(in_depth + 1);
				result += "input:\n";
				for (NodeToken item : _input)
				{
					if (nullptr != item)
					{
						result += item->DebugPrint(in_depth + 2);
					}
				}
			}

			if (0 < _index_input.size())
			{
				result += DscCommon::DebugPrint::TabDepth(in_depth + 1);
				result += "indexInput:\n";
				for (NodeToken item : _index_input)
				{
					if (nullptr != item)
					{
						result += item->DebugPrint(in_depth + 2);
					}
				}
			}

			return result;
		}
public:
	typedef std::function<std::string(const IN_TYPE&)> TDebugPrintValue;
	static inline TDebugPrintValue _s_debug_print_value = {};
#endif //#if defined(_DEBUG)

	private:
		bool _dirty = true;
		const TCalculateFunction _calculate_function = {};
		IN_TYPE _value = {};

		std::set<NodeToken> _input = {};
		std::vector<NodeToken> _index_input = {};
		std::set<NodeToken> _output = {};

	}; // DagNodeCalculate
} //DscDag
