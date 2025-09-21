#include "dsc_dag_2.h"
#include <dsc_common/dsc_common.h>
#include <dsc_dag_2/dag_2_node.h>
#include "test_util.h"

namespace
{
const bool TestSanity()
{
	bool ok = true;

	DscDag2::DscNode<int32> dag_node_0 = {};
	dag_node_0.S

	ok = TEST_UTIL_EQUAL(ok, 3, DscDag::GetValueType<int32>(n2));
	DscDag::SetValueType(n0, 3);
	ok = TEST_UTIL_EQUAL(ok, 5, DscDag::GetValueType<int32>(n2));

	return ok;
}

}//namespace

const bool DscDag2Test()
{
	bool ok = true;

	ok &= TestSanity();

	return ok;
}