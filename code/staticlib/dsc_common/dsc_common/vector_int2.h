#pragma once
#include "dsc_common.h"
#include "vector_2.h"
#include "debug_print.h"

namespace DscCommon
{
	typedef Vector2<int32> VectorInt2;
} //namespace DscCommon


#if defined(_DEBUG)
template <>
const std::string DscCommon::DebugPrint::PrintType(const VectorInt2& in_value);
#endif // #if defined(_DEBUG)

