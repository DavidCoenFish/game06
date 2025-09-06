#pragma once
#include "dsc_dag.h"
#include "i_dag_node.h"
#include <dsc_common\debug_print.h>
#include <dsc_common\log_system.h>

namespace DscDag
{
	class DagNodeCalculateBase : public IDagNode
	{
	public:
		const bool SetIndexInput(const int32 in_index, NodeToken in_node = nullptr);
		void AddInput(NodeToken in_node);
		void RemoveInput(NodeToken in_node);

	protected:
		bool _dirty = true;
		std::set<NodeToken> _input = {};
		std::vector<NodeToken> _index_input = {};
		std::set<NodeToken> _output = {};

	};


	template <typename IN_TYPE>
	class DagNodeCalculate : public DagNodeCalculateBase
	{
	public:
		DagNodeCalculate() = delete;
		DagNodeCalculate& operator=(const DagNodeCalculate&) = delete;
		DagNodeCalculate(const DagNodeCalculate&) = delete;

		typedef std::function<void(IN_TYPE&, std::set<NodeToken>&, std::vector<NodeToken>&)> TCalculateFunction;
		DagNodeCalculate(const TCalculateFunction& in_calculate_function)
			: _calculate_function(in_calculate_function)
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

			//#if defined(_DEBUG)
			//DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_DAG, "Calculate MarkDirty:%s\n", DebugGetNodeName().c_str());
			//#endif// defined(_DEBUG)

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
				// this also allow use of DagNodes to construct the ui render drawlist, where the input update draws children textures to be ready for the calculate to draw
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

		virtual void AddOutput(NodeToken in_node) override
		{
			DSC_ASSERT(nullptr != in_node, "invalid param");
			if (nullptr != in_node)
			{
				in_node->MarkDirty();
			}
			_output.insert(in_node);
		}

		virtual void RemoveOutput(NodeToken in_node) override
		{
			DSC_ASSERT(nullptr != in_node, "invalid param");
			if (nullptr != in_node)
			{
				in_node->MarkDirty();
			}
			_output.erase(in_node);
		}

		virtual const bool GetHasNoLinks() const override
		{
			if (0 != _output.size())
			{
				return false;
			}
			//if (0 != _input.size())
			//{
			//	return false;
			//}
			//for (const auto& item : _index_input)
			//{
			//	if (nullptr != item)
			//	{
			//		return false;
			//	}
			//}
			return true;
		}

		virtual void UnlinkInputs() override
		{
			if (false == _unlinked)
			{
				//DSC_LOG_DIAGNOSTIC(LOG_TOPIC_DSC_DAG, "UnlinkInputs:%s %p\n", DebugGetNodeName().c_str(), this);

				_unlinked = true;
				for (const auto& item : _index_input)
				{
					if (nullptr != item)
					{
						//item->UnlinkInputs();
						item->RemoveOutput(this);
					}
				}
				//_index_input.clear();
				for (const auto& item : _input)
				{
					if (nullptr != item)
					{
						//item->UnlinkInputs();
						item->RemoveOutput(this);
					}
				}
				//_input.clear();
			}
			return;
		}

#if defined(_DEBUG)
		virtual const std::type_info& DebugGetTypeInfo() const override
		{
			return typeid(IN_TYPE);
		}

		virtual const std::string DebugPrintRecurseInputs(const int32 in_depth = 0) const override
		{
			std::string result = DscCommon::DebugPrint::TabDepth(in_depth);

			result += "Calculate:\"";
			result += DebugGetNodeName();
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
						result += item->DebugPrintRecurseInputs(in_depth + 2);
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
						result += item->DebugPrintRecurseInputs(in_depth + 2);
					}
				}
			}

			return result;
		}

		virtual const std::string DebugPrintRecurseOutputs(const int32 in_depth = 0) const override
		{
			std::string result = DscCommon::DebugPrint::TabDepth(in_depth);

			result += "Calculate:\"";
			result += DebugGetNodeName();
			result += "\" dirty:" + std::to_string(_dirty);
			result += " type:";
			result += typeid(IN_TYPE).name();
			if (nullptr != _s_debug_print_value)
			{
				result += " value:";
				result += _s_debug_print_value(_value);
			}
			result += "\n";

			if (0 < _output.size())
			{
				result += DscCommon::DebugPrint::TabDepth(in_depth + 1);
				result += "output:\n";
				for (NodeToken item : _output)
				{
					if (nullptr != item)
					{
						result += item->DebugPrintRecurseOutputs(in_depth + 2);
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
		const TCalculateFunction _calculate_function = {};
		IN_TYPE _value = {};
		bool _unlinked = false;

	}; // DagNodeCalculate
} //DscDag
