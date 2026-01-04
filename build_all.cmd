::build_all.cmd
@ECHO OFF

ECHO %time% %~nx0 %*

PUSHD %~dp0\code
::..\bin\FBuild.exe all -config dsc.bff -summary -verbose -dbfile ..\build\dsc.fdb
..\bin\FBuild.exe all -config dsc.bff -dbfile ..\build\dsc.fdb
POPD

EXIT /B %ERRORLEVEL%