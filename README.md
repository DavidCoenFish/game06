  This is a hobbiest project to test out fastbuild and d3dx12

  to build the vcxproj, .sln
.\code\dsc.cmd

  inside the solution file, there are support static lib, sdl static libs [FreeType, HarfBuzz]
.\code\solution.sln

  develop functionalty by exe to focus on each feature
unit_test
render_triangle
render_texture
render_target
//todo render_text
//todo render_version
//todo render_ui

  hint
.\code\dsc.bff
 setting .SolutionBuildProject in the above file to the project that you are working on and then regenerating projects (.\code\dsc.cmd) helps

