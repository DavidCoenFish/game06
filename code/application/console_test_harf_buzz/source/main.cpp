#include "main.h"

#include <dsc_common/file_system.h>

//#include <ft2build.h>
//#include FT_FREETYPE_H
//
//#include <hb.h>
//#include <hb-ft.h>
#include <hb-ft.h>

#pragma warning(push) 
#pragma warning(disable: 4668)
#include <iostream>
#pragma warning(pop)

//https://github.com/harfbuzz/harfbuzz-tutorial/blob/master/hello-harfbuzz-freetype.c

//int main(int argc, char* argv[], char* envp[])
int main(int, char*, char*)
{
	//https://harfbuzz.github.io/a-simple-shaping-example.html

    FT_Library ft_library;
    FT_Face ft_face;
    FT_F26Dot6 FONT_SIZE = 128;

    if ((FT_Init_FreeType(&ft_library)))
        abort();
    if ((FT_New_Face(ft_library, DscCommon::FileSystem::JoinPath("data", "font", "code2002.ttf").c_str(), 0, &ft_face)))
        abort();
    if ((FT_Set_Char_Size(ft_face, FONT_SIZE * 64, FONT_SIZE * 64, 0, 0)))
        abort();

    /* Create hb-ft font. */
    hb_font_t* hb_font;
    hb_font = hb_ft_font_create_referenced(ft_face);
        //hb_ft_font_create(ft_face, NULL);

    /* Create hb-buffer and populate. */
    hb_buffer_t* hb_buffer;
    hb_buffer = hb_buffer_create();
    hb_buffer_add_utf8(hb_buffer, "yo momma", -1, 0, -1);
    hb_buffer_guess_segment_properties(hb_buffer);

    /* Shape it! */
    hb_shape(hb_font, hb_buffer, NULL, 0);

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

        printf("glyph='%s'	cluster=%d	advance=(%g,%g)	offset=(%g,%g)\n",
            glyphname, cluster, x_advance, y_advance, x_offset, y_offset);
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

            printf("glyph='%s'	cluster=%d	position=(%g,%g)\n",
                glyphname, cluster, x_position, y_position);

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

