@ECHO OFF
PUSHD %~dp0\code
cls

..\bin\FBuild.exe solution -config dsc.bff -summary -verbose -dbfile ..\build\dsc.fdb

POPD
