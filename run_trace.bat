@echo off
setlocal EnableDelayedExpansion

echo Started: %DATE% %TIME%

set "trace_file=trace_file.perfetto-trace"

:: Get epoch timestamp in Windows using PowerShell
for /f "usebackq" %%i in (`powershell -Command "Get-Date -UFormat %%s"`) do set "epoch=%%i"

:: Rename existing trace file if it exists
if exist "%trace_file%" (
    move /y "%trace_file%" "trace_%epoch%.trace" >nul
)

start "" c:\perfetto\tracebox.exe -o "%trace_file%" --txt -c scheduling.cfg -d

:: Wait a while for it to start
timeout /t 1 /nobreak >nul

:: Run test program from release directory.
call build/Release/test.exe

:: Wait for trace file to be generated
:WAIT_LOOP
if not exist "%trace_file%" (
    timeout /t 1 /nobreak >nul
    goto WAIT_LOOP
)

:: Windows handles file permissions differently than Linux (chmod is not needed)

echo Completed: %DATE% %TIME%
endlocal