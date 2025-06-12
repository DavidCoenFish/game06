@ECHO OFF
PUSHD %~dp0

::git rev-parse HEAD > code\staticlib\dsc_version\dsc_version\git_revision.txt

::for /f "delims=" %%A in ('git rev-parse HEAD') do set "var=%%A"
::echo "%var%">>code\staticlib\dsc_version\dsc_version\git_revision.txt

set /p VERSION_MAJOR=<code\staticlib\dsc_version\dsc_version\version_major.txt
set VERSION_MAJOR=%VERSION_MAJOR:"=%
set /p VERSION_MINOR=<code\staticlib\dsc_version\dsc_version\version_minor.txt
set VERSION_MINOR=%VERSION_MINOR:"=%
set /p VERSION_PATCH=<code\staticlib\dsc_version\dsc_version\version_patch.txt
set VERSION_PATCH=%VERSION_PATCH:"=%

set /A VERSION_PATCH=VERSION_PATCH+1

echo "%VERSION_MAJOR%.%VERSION_MINOR%.%VERSION_PATCH%">code\staticlib\dsc_version\dsc_version\version_string.txt
echo "%VERSION_PATCH%">code\staticlib\dsc_version\dsc_version\version_patch.txt

POPD
