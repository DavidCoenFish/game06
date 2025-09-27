#pragma once
#include "dsc_dag_2.h"
#include "node.h"
#include "node_array_node.h"
#include "calculate_component.h"
#include <dsc_common\dsc_common.h>

namespace DscDag2
{
	class INode;

	class NodeOwner
	{
	public:
		//OnSetValue
		template <typename IN_TYPE>
		Node<IN_TYPE>* const CreateValueOnSetValue(
			const IN_TYPE& in_value
			DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name = "")
		)
		{
			auto node = std::make_unique<Node<IN_TYPE>>(
				in_value, 
				&CallbackOnSetValue<IN_TYPE>::Function,
				nullptr
				DSC_DEBUG_ONLY(DSC_COMMA in_debug_name)
				);
			Node<IN_TYPE>* const node_token = node.get();
			_nodes.insert(std::move(node));
			return node_token;
		}

		//Never
		template <typename IN_TYPE>
		Node<IN_TYPE>* const CreateValueNever(
			const IN_TYPE& in_value
			DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name = "")
		)
		{
			auto node = std::make_unique<Node<IN_TYPE>>(
				in_value, 
				&CallbackNever<IN_TYPE>::Function,
				nullptr
				DSC_DEBUG_ONLY(DSC_COMMA in_debug_name)
				);
			Node<IN_TYPE>* const node_token = node.get();
			_nodes.insert(std::move(node));
			return node_token;
		}

		//OnValueChange
		template <typename IN_TYPE>
		Node<IN_TYPE>* const CreateValueOnValueChange(
			const IN_TYPE& in_value
			DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name = "")
		)
		{
			auto node = std::make_unique<Node<IN_TYPE>>(
				in_value, 
				&CallbackOnValueChange<IN_TYPE>::Function,
				nullptr
				DSC_DEBUG_ONLY(DSC_COMMA in_debug_name)
				);
			Node<IN_TYPE>* const node_token = node.get();
			_nodes.insert(std::move(node));
			return node_token;
		}

		//CallbackNotZero
		template <typename IN_TYPE>
		Node<IN_TYPE>* const CreateValueNotZero(
			const IN_TYPE& in_value
			DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name = "")
		)
		{
			auto node = std::make_unique<Node<IN_TYPE>>(
				in_value, 
				&CallbackNotZero<IN_TYPE>::Function,
				nullptr
				DSC_DEBUG_ONLY(DSC_COMMA in_debug_name)
				);
			Node<IN_TYPE>* const node_token = node.get();
			_nodes.insert(std::move(node));
			return node_token;
		}

		//Calculate
		template <
			typename IN_TYPE
			, typename ... IN_TYPE_LIST
			>
		Node<IN_TYPE>* const CreateCalculate(
			const IN_TYPE& in_value,
			const std::function<void(IN_TYPE&, const IN_TYPE_LIST* const...)>& in_callback
			DSC_DEBUG_ONLY(DSC_COMMA const std::string& in_debug_name = "")
		)
		{
			auto node = std::make_unique<Node<IN_TYPE>>(
				in_value, 
				nullptr,
				std::make_unique<CalculateComponent<IN_TYPE, IN_TYPE_LIST...>>(in_callback)
				DSC_DEBUG_ONLY(DSC_COMMA in_debug_name)
			);
			Node<IN_TYPE>* const node_token = node.get();
			_nodes.insert(std::move(node));
			return node_token;
		}

		//NodeArray
		template <
			typename IN_TYPE
			>
		NodeArrayNode<IN_TYPE>* const CreateNodeArrayNode(
			DSC_DEBUG_ONLY(const std::string& in_debug_name = "")
			)
		{
			auto node = std::make_unique<Node<IN_TYPE>>(
				std::vector<Node<IN_TYPE>*>(), 
				nullptr,
				nullptr
				DSC_DEBUG_ONLY(DSC_COMMA in_debug_name)
				);
			NodeArrayNode<IN_TYPE>* const node_token = node.get();
			_nodes.insert(std::move(node));
			return node_token;
		}

	private:
		struct RawPtrComparator {
			bool operator()(const std::unique_ptr<INode>& a, const std::unique_ptr<INode>& b) const;
		};

		// ownership of nodes
		std::set<std::unique_ptr<INode>, RawPtrComparator> _nodes = {};
	};
}

