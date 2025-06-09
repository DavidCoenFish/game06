#pragma once
#include "dsc_common.h"

namespace DscCommon
{
	template<class ENUM>
	class EnumSoftBind
	{
	public:
		static const ENUM EnumFromString(const std::string& in_string);
		static const std::string EnumToString(const ENUM in_enum);

	private:
		EnumSoftBind() = delete;
		EnumSoftBind(const EnumSoftBind&) = delete;
		EnumSoftBind(EnumSoftBind&&) = delete;
		EnumSoftBind& operator=(const EnumSoftBind&) = delete;

	};
}