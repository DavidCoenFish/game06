<!-- https://github.com/darsaveli/Readme-Markdown-Syntax -->

This is a hobbiest project to test out fastbuild and d3dx12 on windows, vs2019. 
A simple game engine, makes use of Freetype and Harfbuzz, packages added via<br>
`vcpkg install harfbuzz`<br>
which also pulled in some zip, png packages. Also makes use of code2000.ttf  <https://www.code2001.com/index.htm><br>
<br>
to build the vcxproj, .sln<br>
`.\code\dsc.cmd`<br>
<br>
inside the solution file, there are support static lib and applications<br>
`.\code\solution.sln`<br>
<br>
develop functionalty by making an exe to focus on each feature
* unit_test
* render_triangle // rgb triangle
* render_texture // texture on a triangle
* render_target // draw a moving triangle to a texture, draw texture to screen
* render_target_viewport // draw to a smaller area of a render texture, confirming subsection rendering for rendertarget reuse via render target pool
* render_glyph // get bitmmap of a character out of a font via freetype and display on screen
* console_test_harf_buzz // loads a font, looks at properties, tries to shape something/ testbed
* render_text // draw some harfbuzz shapped text on screen
* render_version // get verion, data, build type on screen
* // todo render_ui // basic ui components
* // todo render_ui_controls // text, checkbox, buttons, stack, scroll, canvas, combo box


<br>
hint<br>
setting SolutionBuildProject in .\code\dsc.bff (or under) effects what the visual studio solution is set to build by default, and in visual studio, setting the startup project controlles what is run
it is very easy to have visual studio set to run something that is not being built. have yet to fins a setting in bff to enable every project to build


vcpkg install nlohmann-json

