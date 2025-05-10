::fast_build.cmd

@ECHO OFF
PUSHD %~dp0
cls
::..\bin\FBuild.exe -verbose -showcmdoutput -showcmds -showdeps -showtargets -showalltargets -summary 

..\bin\FBuild.exe all -summary -verbose 

POPD

