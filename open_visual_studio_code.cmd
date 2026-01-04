::open_visual_studio_code.cmd
@ECHO OFF

PUSHD %~dp0
START /B /D . code .
POPD

EXIT /b 0
