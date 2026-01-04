#pragma once
#include <dsc_common/dsc_common.h>
#include "dsc_statistics.h"
#include "i_event.h"


namespace DscStatistics
{
	class ValueInt : public IEvent
	{
	public:
		ValueInt() = delete;
		ValueInt& operator=(const ValueInt&) = delete;
		ValueInt(const ValueInt&) = delete;

		static std::unique_ptr<ValueInt> Factory(
			const std::string& in_name,
			const int32 in_value
			);

		ValueInt(
			const std::string& in_name,
			const int32 in_value
			);

	private:

		virtual const std::string& GetName() const override { return _name; }
		virtual const std::string GetDescription() const override;

	private:
		const std::string _name;
		const int32 _value;
	};
}
