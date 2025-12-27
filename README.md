<!-- https://github.com/darsaveli/Readme-Markdown-Syntax -->

This is a hobbiest project to test out fastbuild and d3dx12 on windows, vs2019.<br>
<br>
A simple game engine, makes use of [Freetype, Harfbuzz, ...] packages, added via<br>
`vcpkg install harfbuzz`<br>
which also pulled in some zip, png packages.<br>
<br>
Uses RapidJSON, which is licensed under the MIT License.<br>
<br>
// todo, replace NLohmann with RapidJSON<br>
`vcpkg install nlohmann`<br> 
<br>
Uses of the font code2000.ttf  <https://www.code2001.com/index.htm><br>
<br>
<br>
to create the vcxproj, .sln<br>
`.\generate_solution.cmd`<br>
<br>
inside the solution file, there are support static lib and applications<br>
`.\open_solution.sln`<br>

<br>
to build everything<br>
`.\build_all.cmd`<br>

<br>
hint<br>
setting SolutionBuildProject in .\code\dsc.bff effects what the visual studio solution is set to build by default, and in visual studio, setting the startup project controls what is run.
It is very easy to have visual studio set to run something that is not being built. Have yet to find a setting in bff to enable every project to build, or other ways to deal with this issue.
<br>
<br>
this is not production ready code, the ui system is very prototype, using lost of dynamic casts and way too many allocation, as well as excessive gpu resources (render targets).
additionally the inheritance of uirootnode from uinode, and the poor cross mingling of draw nodes and the ui nodes is not ideal. Ui2 was the start at addressing these concenrs.
