#pragma once
#include "dsc_dag_2.h"
#include "node.h"
#include <dsc_common\dsc_common.h>

namespace DscDag2
{
	/*
		hold a dynamic array of nodes for a given type, and hook the internal node to our dirty chain
	*/
	template <typename IN_TYPE>
	class NodeArrayNode : public Node<std::vector<Node<IN_TYPE>*>>
	{
	public:
		typedef std::vector<Node<IN_TYPE>*> TNodeArray;

		//NodeArrayNode() = delete;
		NodeArrayNode& operator=(const NodeArrayNode&) = delete;
		NodeArrayNode(const NodeArrayNode&) = delete;

		explicit NodeArrayNode(
			DSC_DEBUG_ONLY(const std::string& in_debug_name = "")
			)
		: Node<std::vector<Node<IN_TYPE>*>>(
			TNodeArray(),
			&CallbackNever<TNodeArray>::Function,
			std::unique_ptr<ICalculateComponent<TNodeArray>>()
			DSC_DEBUG_ONLY(DSC_COMMA in_debug_name)
			)
		{
			//NOP
		}

		void Push(Node<IN_TYPE>& in_node)
		{
			AddLink(in_node);
			this->_value.push_back(&in_node);
			this->_dirty_component.MarkDirtyFlag();
			return;
		}

		void Pop()
		{
			if (this->_value.size() <= 0)
			{
				return;
			}
			this->_value.pop_back();
			this->_dirty_component.MarkDirtyFlag();
			return;
		}
		void Remove(Node<IN_TYPE>& in_node)
		{
			auto copy_value = this->_value;
			this->_value.clear();
			bool dirty = false;
			for (const auto& iter : copy_value)
			{
				if (iter == &in_node)
				{
					RemoveLink(*iter);
					dirty = true;
				}
				else
				{
					this->_value.push_back(iter);
				}
			}
			if (true == dirty)
			{
				this->_dirty_component.MarkDirtyFlag();
			}

			return;
		}

		void Clear()
		{
			if (this->_value.size() <= 0)
			{
				return;
			}
			for (const auto& iter : this->_value)
			{
				RemoveLink(*iter);
			}
			this->_value.clear();
			this->_dirty_component.MarkDirtyFlag();
		}
	private:
		void AddLink(INode& in_node)
		{
			DirtyComponent::Link(in_node.GetDirtyComponent(), this->_dirty_component);
			return;
		}
		void RemoveLink(INode& in_node)
		{
			DirtyComponent::Unlink(in_node.GetDirtyComponent(), this->_dirty_component);
			return;
		}
	};
}

