@ECHO OFF
PUSHD %~dp0

CLS

ECHO %time% %~nx0 %*

SET GIT="git.exe"

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

SET TIMESTAMP=%_yyyy%-%_mm%-%_dd%T%_hour%:%_minute%:%_second%

FOR /F "usebackq" %%i IN (`hostname`) DO SET HOST_NAME=%%i

ECHO %GIT% push
%GIT% push
IF 0 NEQ %ERRORLEVEL% (
  ECHO ERRORLEVEL %ERRORLEVEL%
  exit /b -1
)

ECHO %GIT% pull
%GIT% pull
IF 0 NEQ %ERRORLEVEL% (
  ECHO ERRORLEVEL %ERRORLEVEL%
  exit /b -1
)

ECHO %GIT% add .
%GIT% add .
IF 0 NEQ %ERRORLEVEL% (
  ECHO ERRORLEVEL %ERRORLEVEL%
  exit /b -1
)

IF "%*" == "" (
SET COMMIT_ARGS="%TIMESTAMP% %HOST_NAME%"
) ELSE (
SET COMMIT_ARGS="%TIMESTAMP% %HOST_NAME% %*"
)

ECHO %GIT% commit -m %COMMIT_ARGS%
%GIT% commit -m %COMMIT_ARGS%
IF 0 NEQ %ERRORLEVEL% (
  ECHO ERRORLEVEL %ERRORLEVEL%
  exit /b -1
  )

ECHO %GIT% push
%GIT% push
IF 0 NEQ %ERRORLEVEL% (
  ECHO ERRORLEVEL %ERRORLEVEL%
  exit /b -1
)

ECHO %GIT% status
%GIT% status
IF 0 NEQ %ERRORLEVEL% (
  ECHO ERRORLEVEL %ERRORLEVEL%
  exit /b -1
)

:::: get the current revision
::for /f "delims=" %%A in ('git rev-parse HEAD') do set "var=%%A"
::echo "%var%">code\staticlib\dsc_version\dsc_version\git_revision.txt
::echo "%TIMESTAMP%">code\staticlib\dsc_version\dsc_version\timestamp.txt
::
::set /p VERSION_MAJOR=<code\staticlib\dsc_version\dsc_version\version_major.txt
::set VERSION_MAJOR=%VERSION_MAJOR:"=%
::set /p VERSION_MINOR=<code\staticlib\dsc_version\dsc_version\version_minor.txt
::set VERSION_MINOR=%VERSION_MINOR:"=%
::set /p VERSION_PATCH=<code\staticlib\dsc_version\dsc_version\version_patch.txt
::set VERSION_PATCH=%VERSION_PATCH:"=%
::
::set /A VERSION_PATCH=VERSION_PATCH+1
::
::echo "%VERSION_MAJOR%.%VERSION_MINOR%.%VERSION_PATCH%">code\staticlib\dsc_version\dsc_version\version_string.txt
::echo "%VERSION_PATCH%">code\staticlib\dsc_version\dsc_version\version_patch.txt


:: to set a git tag
::git tag -a v0.1 -m "start tagged version history"

for /f "delims=" %%A in ('git describe --long') do set "var=%%A"
echo "%var%">code\staticlib\dsc_version\dsc_version\git_revision.txt

echo "%TIMESTAMP%">code\staticlib\dsc_version\dsc_version\timestamp.txt

POPD
