::generate_solution.cmd

@ECHO OFF
PUSHD %~dp0\code
cls

::..\bin\FBuild.exe all -config dsc.bff -summary -verbose -dbfile ..\build\dsc.fdb
..\bin\FBuild.exe all -config dsc.bff -summary -dbfile ..\build\dsc.fdb

POPD

