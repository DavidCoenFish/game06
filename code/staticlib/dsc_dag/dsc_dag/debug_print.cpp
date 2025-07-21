#include "debug_print.h"
#include "i_dag_node.h"

#if defined(_DEBUG)
void DscDag::DebugSetNodeName(NodeToken in_input, const std::string& in_debug_name)
{
	DSC_ASSERT(nullptr != in_input, "invalid param");
	if (nullptr != in_input)
	{
		in_input->DebugSetNodeName(in_debug_name);
	}

	return;
}

void DscDag::DebugPrintRecurseInputs(NodeToken in_input)
{
	const std::string fullMessage = in_input->DebugPrintRecurseInputs();
	std::string line = {};
	for (const auto c : fullMessage)
	{
		line += c;
		if (c == '\n')
		{
			OutputDebugString(std::wstring(line.begin(), line.end()).c_str());
			line = {};
		}
	}
	if (false == line.empty())
	{
		OutputDebugString(std::wstring(line.begin(), line.end()).c_str());
	}

	return;
}

void DscDag::DebugPrintRecurseOutputs(NodeToken in_input)
{
	const std::string fullMessage = in_input->DebugPrintRecurseOutputs();
	std::string line = {};
	for (const auto c : fullMessage)
	{
		line += c;
		if (c == '\n')
		{
			OutputDebugString(std::wstring(line.begin(), line.end()).c_str());
			line = {};
		}
	}
	if (false == line.empty())
	{
		OutputDebugString(std::wstring(line.begin(), line.end()).c_str());
	}

	return;
}

#endif //_DEBUG