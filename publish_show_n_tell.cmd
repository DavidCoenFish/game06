@ECHO OFF
PUSHD %~dp0
CLS
ECHO %time% %~nx0 %*

::Get timestamp

FOR /F "skip=1 tokens=1-6" %%G IN ('WMIC Path Win32_LocalTime Get Day^,Hour^,Minute^,Month^,Second^,Year /Format:table') DO (
   IF "%%~L"=="" goto S_DONE
	  SET _yyyy=%%L
	  SET _mm=00%%J
	  SET _dd=00%%G
	  SET _hour=00%%H
	  SET _minute=00%%I
	  SET _second=00%%K
)
: S_DONE

SET _mm=%_mm:~-2%
SET _dd=%_dd:~-2%
SET _hour=%_hour:~-2%
SET _minute=%_minute:~-2%
SET _second=%_second:~-2%
SET TIMESTAMP=%_yyyy%-%_mm%-%_dd%T%_hour%_%_minute%_%_second%

::Set project

SET PROJECT_NAME=render_ui_button
SET PLATFORM=x64
SET CONFIG=Release

:: invoke fastbuild to build the project, warning, env is slightly different than from being run in VS, possibly need to run vcvars64.bat?
::G:\bin\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat
:: WARNING, added "C:\Program Files (x86)\Windows Kits\10\bin\10.0.19041.0\x64" to the env PATH else was having the linker not find the rc.exe
::C:\Program Files (x86)\Windows Kits\10\bin\10.0.19041.0\x64\rc.exe
PUSHD code
::https://www.fastbuild.org/docs/options.html
::-clean
::-verbose 
::-showcmds
..\bin\fbuild.exe -config dsc.bff -dbfile ..\build\dsc.fdb -vs -cache "%PROJECT_NAME%_%CONFIG%_%PLATFORM%"
POPD

echo %ERRORLEVEL%
if %ERRORLEVEL% NEQ 0 (
	echo Error %ERRORLEVEL%
	EXIT /B 1
)

:: mirror the build project output dir to a show_n_tell dir, two folders above this script

robocopy "build\%PROJECT_NAME%\%PLATFORM%\%CONFIG%\output" "..\..\shown_n_tell\%PROJECT_NAME%\%TIMESTAMP%" /MIR /s /ns /nc /nfl /ndl /np

::echo %ERRORLEVEL%
::Any value equal to or greater than 8 indicates that there was at least one failure during the copy operation.
if %ERRORLEVEL% GEQ 8 (
	echo Error %ERRORLEVEL%
	EXIT /B 1
)

type nul>nul
::echo %ERRORLEVEL%

POPD
