if "%PROCESSOR_ARCHITECTURE%" == "AMD64" (
    set _MySuf=x64
) else (
    set _MySuf=x86
)
reg add "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Windows" /v LoadAppInit_DLLs /t REG_DWORD /d 1 /f
reg add "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Windows" /v AppInit_DLLs /t REG_SZ /d %~dp0ExplorerExtSilentRenamingMixIn.%_MySuf%.dll /f
pause
