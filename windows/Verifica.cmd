@echo off
setlocal
cd /d "%~dp0"
echo Esecuzione test AiloEMU con la demo originale...
start "" /wait "%~dp0AiloEMU.exe" --self-test
set "test_result=%errorlevel%"
if exist "%~dp0test-results\report.txt" (
  start "" notepad.exe "%~dp0test-results\report.txt"
) else (
  echo Il programma non ha generato il rapporto.
  pause
)
exit /b %test_result%
