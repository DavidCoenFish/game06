#include "dsc_dag_2.h"
#include <dsc_common/dsc_common.h>
#include <dsc_dag_2/dag_2_node.h>
#include "test_util.h"

namespace
{
const bool TestValue()
{
	bool ok = true;

	DscDag2::Dag2Node<int32> dag_node_0(
		0, 
		&DscDag2::CallbackOnValueChange<int32>::Function,
		std::unique_ptr<DscDag2::IDag2CalculateComponentBase<int32>>()
		);

	ok = TEST_UTIL_EQUAL(ok, 0, dag_node_0.GetValue());
	dag_node_0.SetValue(7);
	ok = TEST_UTIL_EQUAL(ok, 7, dag_node_0.GetValue());

	return ok;
}

}//namespace

const bool DscDag2Test()
{
	bool ok = true;

	ok &= TestValue();

	return ok;
}