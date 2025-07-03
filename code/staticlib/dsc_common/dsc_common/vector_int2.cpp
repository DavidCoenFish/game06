#include "vector_int2.h"

const DscCommon::Vector2<int32> DscCommon::Vector2<int32>::s_zero(0, 0);

#if defined(_DEBUG)
template <>
const std::string DscCommon::DebugPrint::PrintType(const VectorInt2& in_value)
{
	std::string result = {};
	result += "[";
	result += std::to_string(in_value.GetX());
	result += ", ";
	result += std::to_string(in_value.GetY());
	result += "]";
	return result;
}
#endif // #if defined(_DEBUG)

