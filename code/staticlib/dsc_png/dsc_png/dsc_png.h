#pragma once
#include <dsc_common/dsc_common.h>

#define LOG_TOPIC_DSC_PNG "DSC_PNG"

namespace DscCommon
{
	template <typename TYPE>
	class Vector2;
	typedef Vector2<int32> VectorInt2;
	class FileSystem;
}

namespace DscPng
{
	// DscCommon::FileSystem::JoinPath("shader", "dsc_ui", "screen_quad_texture_vs.cso")

	/// failure logged, zero size data returned if something goes wrong
	void LoadPng(
		std::vector<uint8>& out_data,
		int32& out_byte_per_pixel,
		DscCommon::VectorInt2& out_size,
		DscCommon::FileSystem& in_file_system,
		const std::string& in_file_path
	);

	// this could be a flag passed to the LoadPng function, but trying keep functions only doing one thing
	void ForceRgba(
		std::vector<uint8>& in_out_data,
		const int32 in_byte_per_pixel,
		const DscCommon::VectorInt2& in_size,
		const uint8 in_alpha_value = 255
		);

	/// failure logged
	void SavePng(
		const std::vector<uint8>& in_data,
		const int32 in_byte_per_pixel,
		const DscCommon::VectorInt2& in_size,
		DscCommon::FileSystem& in_file_system,
		const std::string& in_file_path
	);
}

#ifndef DSC_BFF_BUILD

//G:\development\game06\code\staticlib\dsc_dag\dsc_dag\dsc_dag.h
#include "..\..\dsc_common\dsc_common\dsc_common.h"
#include "..\..\..\sdk\libpng_x64-windows\include\png.h"

#endif //#ifndef DSC_BFF_BUILD

