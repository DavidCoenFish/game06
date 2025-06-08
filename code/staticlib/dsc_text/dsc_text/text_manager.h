#pragma once
#include <dsc_common/dsc_common.h>

namespace DscText
{
	class TextManager
	{
	public:
		static const std::vector<D3D12_INPUT_ELEMENT_DESC>& GetInputElementDesc();

		TextManager(
			DrawSystem* const in_draw_system,
			ID3D12GraphicsCommandList* const in_command_list,
			const std::filesystem::path& in_root_path
		);
		~TextManager();

		// Find or make a new text face
		TextFont* const GetTextFont(
			const std::filesystem::path& in_font_rel_path
		);

		// An Icon is a small 4 channel image
		IconFont* const GetIconFont();

		const TextLocale* const GetLocaleToken(const LocaleISO_639_1 in_locale) const;
	};
}
