#pragma once
#include "dsc_dag_2.h"

namespace DscDag2
{
	//
	template<typename... IN_TYPES>
	class Dag2CalculateComponent {
	public:
		// Constructor to accept arguments of the specified types
		Dag2CalculateComponent(IN_TYPES... args) {
			// You can process the arguments here
			// For example, store them in a tuple or a vector of variant
			// or perform operations based on their types.
			// This example just prints the number of arguments.
			std::cout << "Number of arguments: " << sizeof...(args) << std::endl;
		}

}