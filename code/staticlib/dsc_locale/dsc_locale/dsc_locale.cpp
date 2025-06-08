#include "dsc_locale.h"
#include <dsc_common\enum_soft_bind.h>

namespace
{
	struct Data
	{
		Data() = delete;
		Data& operator=(const Data&) = delete;
		Data(const Data&) = delete;

		const char* const _data;
		const DscLocale::LocaleISO_639_1 _enum;
	};

	const char* const s_default_data = "";

	static const Data s_data[] = {
		{s_default_data, DscLocale::LocaleISO_639_1::Default},

		{"ab", DscLocale::LocaleISO_639_1::Abkhazian},
		{"aa", DscLocale::LocaleISO_639_1::Afar},
		{"af", DscLocale::LocaleISO_639_1::Afrikaans},
		{"ak", DscLocale::LocaleISO_639_1::Akan},
		{"sq", DscLocale::LocaleISO_639_1::Albanian},
		{"am", DscLocale::LocaleISO_639_1::Amharic},
		{"ar", DscLocale::LocaleISO_639_1::Arabic},
		{"an", DscLocale::LocaleISO_639_1::Aragonese},
		{"hy", DscLocale::LocaleISO_639_1::Armenian},
		{"as", DscLocale::LocaleISO_639_1::Assamese},
		{"av", DscLocale::LocaleISO_639_1::Avaric},
		{"ae", DscLocale::LocaleISO_639_1::Avestan},
		{"ay", DscLocale::LocaleISO_639_1::Aymara},
		{"az", DscLocale::LocaleISO_639_1::Azerbaijani},
		{"bm", DscLocale::LocaleISO_639_1::Bambara},
		{"ba", DscLocale::LocaleISO_639_1::Bashkir},
		{"eu", DscLocale::LocaleISO_639_1::Basque},
		{"be", DscLocale::LocaleISO_639_1::Belarusian},
		{"bn", DscLocale::LocaleISO_639_1::Bengali},
		{"bi", DscLocale::LocaleISO_639_1::Bislama},
		{"bs", DscLocale::LocaleISO_639_1::Bosnian},
		{"br", DscLocale::LocaleISO_639_1::Breton},
		{"bg", DscLocale::LocaleISO_639_1::Bulgarian},
		{"my", DscLocale::LocaleISO_639_1::Burmese},
		{"ca", DscLocale::LocaleISO_639_1::Catalan},
		{"ch", DscLocale::LocaleISO_639_1::Chamorro},
		{"ce", DscLocale::LocaleISO_639_1::Chechen},
		{"ny", DscLocale::LocaleISO_639_1::Chichewa},
		{"zh-cn", DscLocale::LocaleISO_639_1::Chinese_Simplified},
		{"zh-tw", DscLocale::LocaleISO_639_1::Chinese_Traditional},
		{"cu", DscLocale::LocaleISO_639_1::Old_Slavonic},
		{"cv", DscLocale::LocaleISO_639_1::Chuvash},
		{"kw", DscLocale::LocaleISO_639_1::Cornish},
		{"co", DscLocale::LocaleISO_639_1::Corsican},
		{"cr", DscLocale::LocaleISO_639_1::Cree},
		{"hr", DscLocale::LocaleISO_639_1::Croatian},
		{"cs", DscLocale::LocaleISO_639_1::Czech},
		{"da", DscLocale::LocaleISO_639_1::Danish},
		{"dv", DscLocale::LocaleISO_639_1::Divehi},
		{"nl", DscLocale::LocaleISO_639_1::Dutch},
		{"dz", DscLocale::LocaleISO_639_1::Dzongkha},
		{"en", DscLocale::LocaleISO_639_1::English},
		{"eo", DscLocale::LocaleISO_639_1::Esperanto},
		{"et", DscLocale::LocaleISO_639_1::Estonian},
		{"ee", DscLocale::LocaleISO_639_1::Ewe},
		{"fo", DscLocale::LocaleISO_639_1::Faroese},
		{"fj", DscLocale::LocaleISO_639_1::Fijian},
		{"fi", DscLocale::LocaleISO_639_1::Finnish},
		{"fr", DscLocale::LocaleISO_639_1::French},
		{"fy", DscLocale::LocaleISO_639_1::Frisian},
		{"ff", DscLocale::LocaleISO_639_1::Fulah},
		{"gd", DscLocale::LocaleISO_639_1::Gaelic},
		{"gl", DscLocale::LocaleISO_639_1::Galician},
		{"lg", DscLocale::LocaleISO_639_1::Ganda},
		{"ka", DscLocale::LocaleISO_639_1::Georgian},
		{"de", DscLocale::LocaleISO_639_1::German},
		{"el", DscLocale::LocaleISO_639_1::Greek},
		{"kl", DscLocale::LocaleISO_639_1::Kalaallisut},
		{"gn", DscLocale::LocaleISO_639_1::Guarani},
		{"gu", DscLocale::LocaleISO_639_1::Gujarati},
		{"ht", DscLocale::LocaleISO_639_1::Haitian},
		{"ha", DscLocale::LocaleISO_639_1::Hausa},
		{"he", DscLocale::LocaleISO_639_1::Hebrew},
		{"hz", DscLocale::LocaleISO_639_1::Herero},
		{"hi", DscLocale::LocaleISO_639_1::Hindi},
		{"ho", DscLocale::LocaleISO_639_1::Hiri_Motu},
		{"hu", DscLocale::LocaleISO_639_1::Hungarian},
		{"is", DscLocale::LocaleISO_639_1::Icelandic},
		{"io", DscLocale::LocaleISO_639_1::Ido},
		{"ig", DscLocale::LocaleISO_639_1::Igbo},
		{"id", DscLocale::LocaleISO_639_1::Indonesian},
		{"ia", DscLocale::LocaleISO_639_1::Interlingua},
		{"ie", DscLocale::LocaleISO_639_1::Interlingue},
		{"iu", DscLocale::LocaleISO_639_1::Inuktitut},
		{"ik", DscLocale::LocaleISO_639_1::Inupiaq},
		{"ga", DscLocale::LocaleISO_639_1::Irish},
		{"it", DscLocale::LocaleISO_639_1::Italian},
		{"ja", DscLocale::LocaleISO_639_1::Japanese},
		{"jv", DscLocale::LocaleISO_639_1::Javanese},
		{"kn", DscLocale::LocaleISO_639_1::Kannada},
		{"kr", DscLocale::LocaleISO_639_1::Kanuri},
		{"ks", DscLocale::LocaleISO_639_1::Kashmiri},
		{"kk", DscLocale::LocaleISO_639_1::Kazakh},
		{"km", DscLocale::LocaleISO_639_1::Central_Khmer},
		{"ki", DscLocale::LocaleISO_639_1::Kikuyu},
		{"rw", DscLocale::LocaleISO_639_1::Kinyarwanda},
		{"ky", DscLocale::LocaleISO_639_1::Kirghiz},
		{"kv", DscLocale::LocaleISO_639_1::Komi},
		{"kg", DscLocale::LocaleISO_639_1::Kongo},
		{"ko", DscLocale::LocaleISO_639_1::Korean},
		{"kj", DscLocale::LocaleISO_639_1::Kuanyama},
		{"ku", DscLocale::LocaleISO_639_1::Kurdish},
		{"lo", DscLocale::LocaleISO_639_1::Lao},
		{"la", DscLocale::LocaleISO_639_1::Latin},
		{"lv", DscLocale::LocaleISO_639_1::Latvian},
		{"li", DscLocale::LocaleISO_639_1::Limburgan},
		{"ln", DscLocale::LocaleISO_639_1::Lingala},
		{"lt", DscLocale::LocaleISO_639_1::Lithuanian},
		{"lu", DscLocale::LocaleISO_639_1::Luba_Katanga},
		{"lb", DscLocale::LocaleISO_639_1::Luxembourgish},
		{"mk", DscLocale::LocaleISO_639_1::Macedonian},
		{"mg", DscLocale::LocaleISO_639_1::Malagasy},
		{"ms", DscLocale::LocaleISO_639_1::Malay},
		{"ml", DscLocale::LocaleISO_639_1::Malayalam},
		{"mt", DscLocale::LocaleISO_639_1::Maltese},
		{"gv", DscLocale::LocaleISO_639_1::Manx},
		{"mi", DscLocale::LocaleISO_639_1::Maori},
		{"mr", DscLocale::LocaleISO_639_1::Marathi},
		{"mh", DscLocale::LocaleISO_639_1::Marshallese},
		{"mn", DscLocale::LocaleISO_639_1::Mongolian},
		{"na", DscLocale::LocaleISO_639_1::Nauru},
		{"nv", DscLocale::LocaleISO_639_1::Navajo},
		{"nd", DscLocale::LocaleISO_639_1::North_Ndebele},
		{"nr", DscLocale::LocaleISO_639_1::South_Ndebele},
		{"ng", DscLocale::LocaleISO_639_1::Ndonga},
		{"ne", DscLocale::LocaleISO_639_1::Nepali},
		{"no", DscLocale::LocaleISO_639_1::Norwegian},
		{"nb", DscLocale::LocaleISO_639_1::Norwegian_Bokmal},
		{"nn", DscLocale::LocaleISO_639_1::Norwegian_Nynorsk},
		{"ii", DscLocale::LocaleISO_639_1::Sichuan_Yi},
		{"oc", DscLocale::LocaleISO_639_1::Occitan},
		{"oj", DscLocale::LocaleISO_639_1::Ojibwa},
		{"or", DscLocale::LocaleISO_639_1::Oriya},
		{"om", DscLocale::LocaleISO_639_1::Oromo},
		{"os", DscLocale::LocaleISO_639_1::Ossetian},
		{"pi", DscLocale::LocaleISO_639_1::Pali},
		{"ps", DscLocale::LocaleISO_639_1::Pashto},
		{"fa", DscLocale::LocaleISO_639_1::Persian},
		{"pl", DscLocale::LocaleISO_639_1::Polish},
		{"pt", DscLocale::LocaleISO_639_1::Portuguese},
		{"pa", DscLocale::LocaleISO_639_1::Punjabi},
		{"qu", DscLocale::LocaleISO_639_1::Quechua},
		{"ro", DscLocale::LocaleISO_639_1::Romanian},
		{"rm", DscLocale::LocaleISO_639_1::Romansh},
		{"rn", DscLocale::LocaleISO_639_1::Rundi},
		{"ru", DscLocale::LocaleISO_639_1::Russian},
		{"se", DscLocale::LocaleISO_639_1::Northern_Sami},
		{"sm", DscLocale::LocaleISO_639_1::Samoan},
		{"sg", DscLocale::LocaleISO_639_1::Sango},
		{"sa", DscLocale::LocaleISO_639_1::Sanskrit},
		{"sc", DscLocale::LocaleISO_639_1::Sardinian},
		{"sr", DscLocale::LocaleISO_639_1::Serbian},
		{"sn", DscLocale::LocaleISO_639_1::Shona},
		{"sd", DscLocale::LocaleISO_639_1::Sindhi},
		{"si", DscLocale::LocaleISO_639_1::Sinhala},
		{"sk", DscLocale::LocaleISO_639_1::Slovak},
		{"sl", DscLocale::LocaleISO_639_1::Slovenian},
		{"so", DscLocale::LocaleISO_639_1::Somali},
		{"st", DscLocale::LocaleISO_639_1::Southern},
		{"es", DscLocale::LocaleISO_639_1::Spanish},
		{"su", DscLocale::LocaleISO_639_1::Sundanese},
		{"sw", DscLocale::LocaleISO_639_1::Swahili},
		{"ss", DscLocale::LocaleISO_639_1::Swati},
		{"sv", DscLocale::LocaleISO_639_1::Swedish},
		{"tl", DscLocale::LocaleISO_639_1::Tagalog},
		{"ty", DscLocale::LocaleISO_639_1::Tahitian},
		{"tg", DscLocale::LocaleISO_639_1::Tajik},
		{"ta", DscLocale::LocaleISO_639_1::Tamil},
		{"tt", DscLocale::LocaleISO_639_1::Tatar},
		{"te", DscLocale::LocaleISO_639_1::Telugu},
		{"th", DscLocale::LocaleISO_639_1::Thai},
		{"bo", DscLocale::LocaleISO_639_1::Tibetan},
		{"ti", DscLocale::LocaleISO_639_1::Tigrinya},
		{"to", DscLocale::LocaleISO_639_1::Tonga},
		{"ts", DscLocale::LocaleISO_639_1::Tsonga},
		{"tn", DscLocale::LocaleISO_639_1::Tswana},
		{"tr", DscLocale::LocaleISO_639_1::Turkish},
		{"tk", DscLocale::LocaleISO_639_1::Turkmen},
		{"tw", DscLocale::LocaleISO_639_1::Twi},
		{"ug", DscLocale::LocaleISO_639_1::Uighur},
		{"uk", DscLocale::LocaleISO_639_1::Ukrainian},
		{"ur", DscLocale::LocaleISO_639_1::Urdu},
		{"uz", DscLocale::LocaleISO_639_1::Uzbek},
		{"ve", DscLocale::LocaleISO_639_1::Venda},
		{"vi", DscLocale::LocaleISO_639_1::Vietnamese},
		{"vo", DscLocale::LocaleISO_639_1::Volapuk},
		{"wa", DscLocale::LocaleISO_639_1::Walloon},
		{"cy", DscLocale::LocaleISO_639_1::Welsh},
		{"wo", DscLocale::LocaleISO_639_1::Wolof},
		{"xh", DscLocale::LocaleISO_639_1::Xhosa},
		{"yi", DscLocale::LocaleISO_639_1::Yiddish},
		{"yo", DscLocale::LocaleISO_639_1::Yoruba},
		{"za", DscLocale::LocaleISO_639_1::Zhuang},
		{"zu", DscLocale::LocaleISO_639_1::Zulu},
	};
	const int s_data_count = ARRAY_LITERAL_SIZE(s_data);

	const std::map<std::string, DscLocale::LocaleISO_639_1>& GetStringEnumMap()
	{
		static std::map<std::string, DscLocale::LocaleISO_639_1> s_map;
		if (0 == s_map.size())
		{
			for (int index = 0; index < s_data_count; ++index)
			{
				const Data& data = s_data[index];
				s_map[data._data] = data._enum;
			}
		}
		return s_map;
	}

	const std::map<DscLocale::LocaleISO_639_1, std::string>& GetEnumStringMap()
	{
		static std::map<DscLocale::LocaleISO_639_1, std::string> s_map;
		if (0 == s_map.size())
		{
			for (int index = 0; index < s_data_count; ++index)
			{
				const Data& data = s_data[index];
				s_map[data._enum] = data._data;
			}
		}
		return s_map;
	}
}

template <>
const DscLocale::LocaleISO_639_1 DscCommon::EnumSoftBind<DscLocale::LocaleISO_639_1>::EnumFromString(const std::string& in_locale)
{
	const auto& map = GetStringEnumMap();
	auto found = map.find(in_locale);
	if (found != map.end())
	{
		return found->second;
	}
	return DscLocale::LocaleISO_639_1::Default;
}

template <>
const std::string DscCommon::EnumSoftBind<DscLocale::LocaleISO_639_1>::EnumToString(const DscLocale::LocaleISO_639_1 in_locale)
{
	const auto& map = GetEnumStringMap();
	auto found = map.find(in_locale);
	if (found != map.end())
	{
		return found->second;
	}
	return s_default_data;
}
