#pragma once
#include "dsc_dag.h"
#include "i_dag_node.h"
#include "dag_enum.h"
#include <dsc_common\dsc_common.h>
#include <dsc_common\debug_print.h>

namespace DscDag
{
	template <typename IN_TYPE>
	class DagNodeValueUnique : public IDagNode
	{
	public:
		DagNodeValueUnique() = delete;
		DagNodeValueUnique& operator=(const DagNodeValueUnique&) = delete;
		DagNodeValueUnique(const DagNodeValueUnique&) = delete;

		DagNodeValueUnique(
			std::unique_ptr<IN_TYPE>&& in_value
			DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name = ""))
			: IDagNode(DSC_DEBUG_ONLY(in_debug_name))
			, _value(std::move(in_value))
		{
			// Nop
		}

		IN_TYPE* const GetValue() const
		{
			return _value.get();
		}

	private:
		virtual void AddOutput(NodeToken in_nodeID) override
		{
			DSC_ASSERT(nullptr != in_nodeID, "invalid param");
			if (nullptr != in_nodeID)
			{
				_output.insert(in_nodeID);
				in_nodeID->MarkDirty();
			}
		}

		virtual void RemoveOutput(NodeToken in_nodeID) override
		{
			DSC_ASSERT(nullptr != in_nodeID, "invalid param");
			if (nullptr != in_nodeID)
			{
				_output.erase(in_nodeID);
				in_nodeID->MarkDirty();
			}
		}

		virtual const bool GetHasNoLinks() const override
		{
			return (0 == _output.size());
		}

		virtual const std::type_info& GetTypeInfo() const override
		{
			return typeid(IN_TYPE);
		}

#if defined(_DEBUG)
		virtual const std::string DebugPrint(const int32 in_depth = 0) const override
		{
			std::string result = DscCommon::DebugPrint::TabDepth(in_depth);

			result += "ValueUnique:\"";
			result += _debug_name;
			result += "\"";
			result += " type:";
			result += typeid(IN_TYPE).name();
			result += " value:";
			result += (_value ? "<not null>" : "<null>");
			result += "\n";

			return result;
		}
#endif //#if defined(_DEBUG)

	private:
		std::unique_ptr<IN_TYPE> _value = {};
		std::set<NodeToken> _output = {};

	};

} //DscDag
