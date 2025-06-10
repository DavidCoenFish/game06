#include "main.h"
#include <hb.h>

#pragma warning(push) 
#pragma warning(disable: 4668)
#include <iostream>
#pragma warning(pop)

//https://github.com/harfbuzz/harfbuzz-tutorial/blob/master/hello-harfbuzz-freetype.c

//int main(int argc, char* argv[], char* envp[])
int main(int, char*, char*)
{
	//https://harfbuzz.github.io/a-simple-shaping-example.html

    hb_buffer_t* buf;
    buf = hb_buffer_create();
    hb_buffer_add_utf8(buf, "hello world", -1, 0, -1);

    hb_buffer_set_direction(buf, HB_DIRECTION_LTR);
    hb_buffer_set_script(buf, HB_SCRIPT_LATIN);
    hb_buffer_set_language(buf, hb_language_from_string("en", -1));

    hb_blob_t* blob = hb_blob_create_from_file("G:\\development\\game06\\data\\font\\font\\code2002.ttf"); /* or hb_blob_create_from_file_or_fail() */
    hb_face_t* face = hb_face_create(blob, 0);
    hb_font_t* font = hb_font_create(face);

    hb_shape(font, buf, NULL, 0);

    unsigned int glyph_count;
    hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buf, &glyph_count);
    hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(buf, &glyph_count);

    hb_position_t cursor_x = 0;
    hb_position_t cursor_y = 0;
    for (unsigned int i = 0; i < glyph_count; i++) {
        hb_codepoint_t glyphid = glyph_info[i].codepoint;
        hb_position_t x_offset = glyph_pos[i].x_offset;
        hb_position_t y_offset = glyph_pos[i].y_offset;
        hb_position_t x_advance = glyph_pos[i].x_advance;
        hb_position_t y_advance = glyph_pos[i].y_advance;
        /* draw_glyph(glyphid, cursor_x + x_offset, cursor_y + y_offset); */
        std::cout << "i:" << i << " glyphid:" << glyphid << " x_offset:" << x_offset << " y_offset:" << y_offset << " x_advance:" << x_advance << " y_advance:" << y_advance << std::endl;

        cursor_x += x_advance;
        cursor_y += y_advance;
    }

    hb_buffer_destroy(buf);
    hb_font_destroy(font);
    hb_face_destroy(face);
    hb_blob_destroy(blob);

	return 0;
}

