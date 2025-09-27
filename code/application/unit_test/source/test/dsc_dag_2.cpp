#include "dsc_dag_2.h"
#include <dsc_common/dsc_common.h>
#include <dsc_dag_2/node.h>
#include <dsc_dag_2/node_array_node.h>
#include <dsc_dag_2/calculate_component.h>
#include <dsc_dag_2/link.h>
#include "test_util.h"

namespace
{

const bool TestValue()
{
	bool ok = true;

	DscDag2::Node<int32> dag_node_0(
		0, 
		&DscDag2::CallbackOnValueChange<int32>::Function//,
		//std::unique_ptr<DscDag2::IDag2CalculateComponent<int32>>()
		);

	ok = TEST_UTIL_EQUAL(ok, 0, dag_node_0.GetValue());
	dag_node_0.SetValue(7);
	ok = TEST_UTIL_EQUAL(ok, 7, dag_node_0.GetValue());

	return ok;
}

const bool TestCalculate()
{
	bool ok = true;

	DscDag2::Node<int32> dag_node_0(0, &DscDag2::CallbackOnValueChange<int32>::Function);
	DscDag2::Node<int32> dag_node_1(0, &DscDag2::CallbackOnValueChange<int32>::Function);

	DscDag2::Node<int32> dag_node_calc(
		0, 
		nullptr,
		std::make_unique<DscDag2::CalculateComponent<int32, int32, int32>>(
			[](int32& out_value, const int32* const in_v0, const int32* const in_v1){
				out_value = (*in_v0) + (*in_v1);
				return;
			}
		));

	DscDag2::Link::SetInput<int32, 0, int32, int32, int32>(dag_node_calc, &dag_node_0);
	DscDag2::Link::SetInput<int32, 1, int32, int32, int32>(dag_node_calc, &dag_node_1);

	ok = TEST_UTIL_EQUAL(ok, 0, dag_node_calc.GetValue());

	dag_node_0.SetValue(2);
	dag_node_1.SetValue(3);

	ok = TEST_UTIL_EQUAL(ok, 5, dag_node_calc.GetValue());

	return ok;
}

const bool TestNodeArrayNode()
{
	bool ok = true;

	DscDag2::Node<int32> dag_node_0(0, &DscDag2::CallbackOnValueChange<int32>::Function);
	DscDag2::Node<int32> dag_node_1(0, &DscDag2::CallbackOnValueChange<int32>::Function);
	DscDag2::NodeArrayNode<int32> dag_node_array(DSC_DEBUG_ONLY("node array"));
	dag_node_array.Push(dag_node_0);
	dag_node_array.Push(dag_node_1);

	int calc_count = 0;
	DscDag2::Node<int32> dag_node_calc(
		0, nullptr, 
		std::make_unique<DscDag2::CalculateComponent<int32, std::vector<DscDag2::Node<int32>*>>>(
			[&calc_count](int32& out_value, const std::vector<DscDag2::Node<int32>*>* const){
				calc_count += 1;
				out_value = calc_count;
				return;
			}
		));

	DscDag2::Link::SetInput<
		std::vector<DscDag2::Node<int32>*>, 
		0, 
		int32, 
		std::vector<DscDag2::Node<int32>*>
		>(dag_node_calc, &dag_node_array);

	ok = TEST_UTIL_EQUAL(ok, 0, calc_count);
	dag_node_calc.GetValue();
	ok = TEST_UTIL_EQUAL(ok, 1, calc_count);
	dag_node_calc.GetValue();
	dag_node_calc.GetValue();
	ok = TEST_UTIL_EQUAL(ok, 1, calc_count);

	dag_node_0.SetValue(2);
	dag_node_calc.GetValue();
	ok = TEST_UTIL_EQUAL(ok, 2, calc_count);

	return ok;
}


}//namespace

const bool DscDag2Test()
{

	bool ok = true;

	ok &= TestValue();
	ok &= TestCalculate();
	ok &= TestNodeArrayNode();

	return ok;
}