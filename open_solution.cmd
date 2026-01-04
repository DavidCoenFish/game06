::open_solution.cmd
@ECHO OFF

PUSHD %~dp0
START "" /B "G:\bin\Microsoft Visual Studio\2019\Community\Common7\IDE\devenv.exe" code\solution.sln
POPD

EXIT /b 0

