#include "data_helper.h"

void DscCommon::DataHelper::AppendData(std::vector<uint8_t>& in_out_data, const uint8_t* const in_value, const int32_t in_size)
{
	for (int index = 0; index < in_size; ++index)
	{
		in_out_data.push_back(in_value[index]);
	}
	return;
}
