::test_all.cmd
@ECHO OFF

ECHO %time% %~nx0 %*

CALL generate_solution.cmd
ECHO.
IF %ERRORLEVEL% LSS 0 (
  ECHO "generate_solution.cmd return error %ERRORLEVEL%"
  GOTO FAIL
)

CALL build_all.cmd
ECHO.
IF %ERRORLEVEL% LSS 0 (
  ECHO "build_all.cmd return error %ERRORLEVEL%"
  GOTO FAIL
)

PUSHD build\unit_test\x64\Release\output
CALL unit_test.exe -unattended
POPD
ECHO.
IF %ERRORLEVEL% LSS 0 (
  ECHO "unit_test.exe return error %ERRORLEVEL%"
  GOTO FAIL
)

ECHO %time% %~nx0 EXIT /b 0
EXIT /b 0

:FAIL
ECHO ------------------------
ECHO ---      FAILED      ---
ECHO ------------------------
ECHO %time% %~nx0 EXIT /b -1
EXIT /b -1


