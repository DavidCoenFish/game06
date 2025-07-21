@ECHO OFF
PUSHD %~dp0

CLS

ECHO %time% %~nx0

FOR /F "skip=1 tokens=1-6" %%G IN ('WMIC Path Win32_LocalTime Get Day^,Hour^,Minute^,Month^,Second^,Year /Format:table') DO (
   IF "%%~L"=="" goto S_DONE
	  SET _yyyy=%%L
	  SET _mm=00%%J
	  SET _dd=00%%G
)
: S_DONE

SET _mm=%_mm:~-2%
SET _dd=%_dd:~-2%

SET TIMESTAMP=%_yyyy%-%_mm%-%_dd%

MKDIR %TIMESTAMP%

