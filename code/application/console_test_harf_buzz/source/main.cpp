#include "main.h"

#include <dsc_common/file_system.h>
#include <dsc_common/log_system.h>

//#include <ft2build.h>
//#include FT_FREETYPE_H
//
//#include <hb.h>
//#include <hb-ft.h>
#include <harfbuzz/hb-ft.h>

#pragma warning(push) 
#pragma warning(disable: 4668)
#include <iostream>
#pragma warning(pop)

//https://github.com/harfbuzz/harfbuzz-tutorial/blob/master/hello-harfbuzz-freetype.c
//https://github.com/tangrams/harfbuzz-example/blob/master/src/freetypelib.cpp
namespace
{
    const hb_tag_t KernTag = HB_TAG('k', 'e', 'r', 'n'); // kerning operations
    const hb_tag_t LigaTag = HB_TAG('l', 'i', 'g', 'a'); // standard ligature substitution
    const hb_tag_t CligTag = HB_TAG('c', 'l', 'i', 'g'); // contextual ligature substitution

    static hb_feature_t LigatureOff = { LigaTag, 0, 0, std::numeric_limits<unsigned int>::max() };
    static hb_feature_t LigatureOn = { LigaTag, 1, 0, std::numeric_limits<unsigned int>::max() };
    static hb_feature_t KerningOff = { KernTag, 0, 0, std::numeric_limits<unsigned int>::max() };
    static hb_feature_t KerningOn = { KernTag, 1, 0, std::numeric_limits<unsigned int>::max() };
    static hb_feature_t CligOff = { CligTag, 0, 0, std::numeric_limits<unsigned int>::max() };
    static hb_feature_t CligOn = { CligTag, 1, 0, std::numeric_limits<unsigned int>::max() };
}

//int main(int argc, char* argv[], char* envp[])
int main(int, char*, char*)
{
    DscCommon::LogSystem logSystem(DscCommon::LogLevel::Diagnostic);

    //https://harfbuzz.github.io/a-simple-shaping-example.html

    FT_Library ft_library = {};
    FT_Face ft_face = {};
    FT_F26Dot6 FONT_SIZE = 256;
    FT_Error error = 0;
    FT_UInt deviceHDPI = 256;

    error = FT_Init_FreeType(&ft_library);
    if (error)
    {
        DSC_LOG_ERROR(LOG_TOPIC_APPLICATION, "Freetype FT_Init_FreeType error:%d\n", error);
        return -1;
    }

    error = FT_New_Face(ft_library, DscCommon::FileSystem::JoinPath("data", "font", "code2000.ttf").c_str(), 0, &ft_face);
    if (error)
    {
        DSC_LOG_ERROR(LOG_TOPIC_APPLICATION, "Freetype FT_New_Face error:%d [%s]\n", error, FT_Error_String(error));
        return -1;
    }

    DSC_LOG_INFO(LOG_TOPIC_APPLICATION, "num_faces:%d face_index:%d\n", ft_face->num_faces, ft_face->face_index);

    FT_Bool fixed_size = FT_HAS_FIXED_SIZES(ft_face);
    FT_Bool fixed_width = FT_IS_FIXED_WIDTH(ft_face);
    FT_Bool use_kerning = FT_HAS_KERNING(ft_face);
    std::cout << "fixed_size:" << fixed_size << " fixed_width:" << fixed_width << " use_kerning:" << use_kerning << " ft_face->face_flags:" << ft_face->face_flags << std::endl;

    for (int i = 0; i < ft_face->num_charmaps; i++) {
        if (((ft_face->charmaps[i]->platform_id == 0)
            && (ft_face->charmaps[i]->encoding_id == 3))
            || ((ft_face->charmaps[i]->platform_id == 3)
                && (ft_face->charmaps[i]->encoding_id == 1))) {
            error = FT_Set_Charmap(ft_face, ft_face->charmaps[i]);
            if (error)
            {
                DSC_LOG_ERROR(LOG_TOPIC_APPLICATION, "Freetype FT_Set_Charmap error:%d [%s]\n", error, FT_Error_String(error));
            }
        }
    }
    FT_Set_Char_Size(ft_face, 0, FONT_SIZE, deviceHDPI, deviceHDPI);

    //error = FT_Set_Char_Size(ft_face, FONT_SIZE * 64, FONT_SIZE * 64, 0, 0);
    //if (error)
    //{
    //    DSC_LOG_ERROR(LOG_TOPIC_APPLICATION, "Freetype FT_Set_Char_Size error:%d\n", error);
    //    return -1;
    //}

    /* Create hb-ft font. */
    hb_font_t* hb_font;
    hb_font = hb_ft_font_create_referenced(ft_face);
        //hb_ft_font_create(ft_face, NULL);

    /* Create hb-buffer and populate. */
    hb_buffer_t* hb_buffer;
    hb_buffer = hb_buffer_create();
    hb_buffer_add_utf8(hb_buffer, "yo momma", -1, 0, -1);
    hb_buffer_guess_segment_properties(hb_buffer);

    std::vector<hb_feature_t> _features = {};
    //_features.push_back(LigatureOff);
    //_features.push_back(LigatureOn);
    //_features.push_back(KerningOff);
    //_features.push_back(KerningOn);
    //_features.push_back(CligOff);
    //_features.push_back(CligOn);

    /* Shape it! */
    hb_shape(hb_font, hb_buffer, _features.empty() ? NULL : _features.data(), static_cast<unsigned int>(_features.size()));

    /* Get glyph information and positions out of the buffer. */
    unsigned int len = hb_buffer_get_length(hb_buffer);
    hb_glyph_info_t* info = hb_buffer_get_glyph_infos(hb_buffer, NULL);
    hb_glyph_position_t* pos = hb_buffer_get_glyph_positions(hb_buffer, NULL);

    /* Print them out as is. */
    printf("Raw buffer contents:\n");
    for (unsigned int i = 0; i < len; i++)
    {
        hb_codepoint_t gid = info[i].codepoint;
        unsigned int cluster = info[i].cluster;
        double x_advance = pos[i].x_advance / 64.;
        double y_advance = pos[i].y_advance / 64.;
        double x_offset = pos[i].x_offset / 64.;
        double y_offset = pos[i].y_offset / 64.;

        char glyphname[32];
        hb_font_get_glyph_name(hb_font, gid, glyphname, sizeof(glyphname));
        std::cout << "glyph:" << glyphname << " cluster:" << cluster << " x_advance:" << x_advance << " y_advance:" << y_advance << "x_offset" << x_offset << "y_offset" << y_offset << std::endl;
    }

    printf("Converted to absolute positions:\n");
    /* And converted to absolute positions. */
    {
        double current_x = 0;
        double current_y = 0;
        for (unsigned int i = 0; i < len; i++)
        {
            hb_codepoint_t gid = info[i].codepoint;
            unsigned int cluster = info[i].cluster;
            double x_position = current_x + pos[i].x_offset / 64.;
            double y_position = current_y + pos[i].y_offset / 64.;


            char glyphname[32];
            hb_font_get_glyph_name(hb_font, gid, glyphname, sizeof(glyphname));
            std::cout << "glyph:" << glyphname << " cluster:" << cluster << " x_position:" << x_position << " y_position:" << y_position << std::endl;

            current_x += pos[i].x_advance / 64.;
            current_y += pos[i].y_advance / 64.;
        }
    }

	hb_buffer_destroy(hb_buffer);
	hb_font_destroy(hb_font);

	FT_Done_Face(ft_face);
	FT_Done_FreeType(ft_library);

	return 0;
}

