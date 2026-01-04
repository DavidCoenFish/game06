::sync.cmd
@ECHO OFF
PUSHD %~dp0

ECHO %time% %~nx0 %~1

SET GIT="git.exe"

ECHO %GIT% pull
%GIT% pull
IF 0 NEQ %ERRORLEVEL% (
  ECHO ERRORLEVEL %ERRORLEVEL%
  exit /b -1
)

POPD