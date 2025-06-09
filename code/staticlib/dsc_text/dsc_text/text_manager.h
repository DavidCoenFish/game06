#pragma once
#include <dsc_common/dsc_common.h>
#include <dsc_locale/dsc_locale.h>

typedef struct FT_LibraryRec_* FT_Library;

namespace DscCommon
{
	class FileSystem;
}

namespace DscRender
{
	class DrawSystem;
	class FileSystem;
}

namespace DscRenderResource
{
	class Shader;
}

namespace DscText
{
	class Glyph;
	class GlyphAtlasTexture;
	class GlyphCollectionIcon;
	class GlyphCollectionText;
	class TextLocale;

	class TextManager
	{
	public:
		static const std::vector<D3D12_INPUT_ELEMENT_DESC>& GetInputElementDesc();

		TextManager() = delete;
		TextManager& operator=(const TextManager&) = delete;
		TextManager(const TextManager&) = delete;


		TextManager(DscRender::DrawSystem& drawSystem, DscCommon::FileSystem& in_file_system);
		~TextManager();

		const TextLocale* const GetLocaleToken(const DscLocale::LocaleISO_639_1 in_locale) const;

		// Find or make a new text face
		GlyphCollectionText* LoadFont(DscCommon::FileSystem& in_file_system, const std::string& in_font_path);


		//TextFont* const GetTextFont(
		//	const std::filesystem::path& in_font_rel_path
		//);

		//// An Icon is a small 4 channel image
		//IconFont* const GetIconFont();


	private:
		FT_Library _library;

		std::map<std::string, std::shared_ptr<GlyphCollectionText>> _map_path_font;

		std::map<uint32_t, std::unique_ptr<Glyph>> _map_icon_cell;
		std::unique_ptr<GlyphCollectionIcon> _icon_font;

		std::unique_ptr<DscRenderResource::Shader> _shader;
		std::unique_ptr<GlyphAtlasTexture> _texture;

	};
}
