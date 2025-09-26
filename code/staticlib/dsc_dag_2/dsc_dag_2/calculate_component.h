#pragma once
#include "dsc_dag_2.h"
#include "i_calculate_component.h"
#include "node.h"
#include <dsc_common\dsc_common.h>

/*

#include <functional>
#include <iostream>
#include <utility> // For std::forward

// A variadic function template to invoke an std::function
template <typename R, typename... Args, typename... CallArgs>
R call_function(std::function<R(Args...)> func, CallArgs&&... call_args) {
    // std::invoke handles calling the callable object (func) with the provided arguments.
    // std::forward preserves the value category (lvalue/rvalue) of the arguments.
    return std::invoke(func, std::forward<CallArgs>(call_args)...);
}

// Example functions with different signatures
void print_sum(int a, int b) {
    std::cout << "Sum: " << a + b << std::endl;
}

double multiply(double a, double b, double c) {
    return a * b * c;
}

void greet(const std::string& name) {
    std::cout << "Hello, " << name << "!" << std::endl;
}

int main() {
    // Create std::function objects
    std::function<void(int, int)> sum_func = print_sum;
    std::function<double(double, double, double)> multiply_func = multiply;
    std::function<void(const std::string&)> greet_func = greet;

    // Invoke with different argument lists using the call_function template
    call_function(sum_func, 5, 10);
    double result = call_function(multiply_func, 2.5, 3.0, 4.0);
    std::cout << "Product: " << result << std::endl;
    call_function(greet_func, "Alice");

    return 0;
}

*/

namespace DscDag2
{
	class INode;

	template<typename IN_RESULT_TYPE, typename ... IN_TYPE_LIST>
	class CalculateComponent : public ICalculateComponent<IN_RESULT_TYPE>
	{
	public:
		typedef std::tuple<IN_TYPE_LIST...> TypeList;
		//typename typedef std::tuple_element<2, TypeList>::type Type02; 
		//typename typedef std::tuple_element<3, TypeList>::type Type03; 

		//typedef std::array<IDag2NodeBase*, sizeof(IN_TYPE_LIST)...> TArrayInput;
		typedef std::array<INode*, sizeof...(IN_TYPE_LIST)> TArrayInput;

		typedef std::function<void(IN_RESULT_TYPE&, const IN_TYPE_LIST* const...)> TCalculateFunction;
		//typedef std::function<void(IN_RESULT_TYPE&, const TArrayInput& in_input)> TCalculateFunction;

		// Constructor to accept arguments of the specified types
		CalculateComponent(const TCalculateFunction& in_calculate_function) 
		: _calculate_function(in_calculate_function)
		{
			DSC_ASSERT(nullptr != _calculate_function, "invalid state");
		}

		//returns the old value
		template <typename IN_TYPE, int32 IN_INDEX>
		INode* SetInput(Node<IN_TYPE>* const in_node_or_nullptr)
		{
			static_assert(std::is_same_v<
				std::tuple_element<IN_INDEX, TypeList>::type,
				IN_TYPE>, "Wrong type at index");
			INode* const result = _input_array[IN_INDEX];
			_input_array[IN_INDEX] = in_node_or_nullptr;
			return result;
		}

#if defined(_DEBUG)
		virtual const std::type_info& DebugGetTypeListTypeInfo() const override
		{
			return typeid(TypeList);
		}
#endif //if defined(_DEBUG)

	private:
		typename typedef std::tuple_element<0, TypeList>::type Type00; 
		typename typedef std::tuple_element<1, TypeList>::type Type01; 

		void Update(IN_RESULT_TYPE& in_out_result)
		{
			//int size = sizeof...(IN_TYPE_LIST);
			//// to to transform 
			//switch(size)
			//{
			//default:
			//case 0:
			//	_calculate_function(in_out_result);
			//	break;
			//case 1:
			//	{
			//		const Type00* value0 = nullptr;
			//		if (nullptr != _input_array[0])
			//		{
			//			Dag2Node<Type00>* pNode = (Dag2Node<Type00>*)(_input_array[0]);
			//			value0 = &(pNode->GetValueRef());
			//		}
			//		_calculate_function(in_out_result, value0);
			//	}
			//	break;
			//case 2:
				{
					const Type00* value0 = (nullptr != _input_array[0]) ? &((Node<Type00>*)_input_array[0])->GetValueRef() : nullptr;  
					const Type01* value1 = (nullptr != _input_array[1]) ? &((Node<Type01>*)_input_array[1])->GetValueRef() : nullptr;  
					_calculate_function(in_out_result, value0, value1);
				}
				//break;
			//}
		}

	private:
		TArrayInput _input_array = {};
		TCalculateFunction _calculate_function = {};
	};
}