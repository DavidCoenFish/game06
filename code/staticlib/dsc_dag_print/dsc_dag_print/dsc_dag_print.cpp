#include "dsc_dag_print.h"
#include <dsc_common\vector_int2.h>
#include <dsc_common\vector_float4.h>
#include <dsc_dag\dag_node_value.h>
#include <dsc_dag\dag_node_calculate.h>

#if defined(_DEBUG)
void DscDagPrint::SetDebugPrintMethods()
{
    DscDag::DagNodeValue<DscCommon::VectorInt2>::_s_debug_print_value = [](const DscCommon::VectorInt2& in_value)
    {
        std::string result;

        result += "[";
        result += std::to_string(in_value.GetX());
        result += ", ";
        result += std::to_string(in_value.GetY());
        result += "]";

        return result;
    };
    DscDag::DagNodeCalculate<DscCommon::VectorInt2>::_s_debug_print_value = DscDag::DagNodeValue<DscCommon::VectorInt2>::_s_debug_print_value;

    DscDag::DagNodeValue<DscCommon::VectorFloat4>::_s_debug_print_value = [](const DscCommon::VectorFloat4& in_value)
    {
        std::string result;

        result += "[";
        result += std::to_string(in_value.GetX());
        result += ", ";
        result += std::to_string(in_value.GetY());
        result += ", ";
        result += std::to_string(in_value.GetZ());
        result += ", ";
        result += std::to_string(in_value.GetW());
        result += "]";

        return result;
    };
    DscDag::DagNodeCalculate<DscCommon::VectorFloat4>::_s_debug_print_value = DscDag::DagNodeValue<DscCommon::VectorFloat4>::_s_debug_print_value;

    DscDag::DagNodeValue<float>::_s_debug_print_value = [](const float& in_value)
    {
        return std::to_string(in_value);
    };
    DscDag::DagNodeCalculate<float>::_s_debug_print_value = DscDag::DagNodeValue<float>::_s_debug_print_value;
}
#endif//#if defined(_DEBUG)
