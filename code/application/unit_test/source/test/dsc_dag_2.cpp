#include "dsc_dag_2.h"
#include <dsc_common/dsc_common.h>
#include <dsc_dag_2/dag_2_node.h>
#include <dsc_dag_2/dag_2_calculate_component.h>
#include "test_util.h"

namespace
{
const bool TestValue()
{
	bool ok = true;

	DscDag2::Dag2Node<int32> dag_node_0(
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

	DscDag2::Dag2Node<int32> dag_node_0(0, &DscDag2::CallbackOnValueChange<int32>::Function);
	DscDag2::Dag2Node<int32> dag_node_1(0, &DscDag2::CallbackOnValueChange<int32>::Function);

	DscDag2::Dag2Node<int32> dag_node_calc(
		std::make_unique<DscDag2::Dag2CalculateComponent<int32, int32, int32>>(
			[](int32& out_value, const int32* const in_v0, const int32* const in_v1){
				out_value = (*in_v0) + (*in_v1);
				return;
			}
		));

	dag_node_calc.SetInput(0, &dag_node_0);
	dag_node_calc.SetInput(1, &dag_node_1);

	ok = TEST_UTIL_EQUAL(ok, 0, dag_node_calc.GetValue());

	dag_node_0.SetValue(2);
	dag_node_1.SetValue(3);

	ok = TEST_UTIL_EQUAL(ok, 5, dag_node_calc.GetValue());

	return ok;
}

}//namespace

const bool DscDag2Test()
{
	bool ok = true;

	ok &= TestValue();
	ok &= TestCalculate();

	return ok;
}