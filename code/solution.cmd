::solution.cmd

@ECHO OFF
PUSHD %~dp0
cls

start "" /B "G:\bin\Microsoft Visual Studio\2019\Community\Common7\IDE\devenv.exe" solution.sln

POPD

