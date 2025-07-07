@echo off
SET PKG_CONFIG_PATH=c:\msys64\mingw64\lib\pkgconfig
set p=;c:/msys64/mingw64/bin/;c:/msys64/usr/bin/;C:\desk\missionsave\robot_driver\llama.cpp\bldwin\bin;
if not defined alreadysetp  SET PATH=%p%%PATH%
SET alreadysetp=1

@REM PATH=;c:/msys64/mingw64/bin/;c:/msys64/usr/bin/;C:\WINDOWS\system32;C:\WINDOWS;C:\WINDOWS\System32\Wbem;C:\WINDOWS\System32\WindowsPowerShell\v1.0\;C:\WINDOWS\System32\OpenSSH\;C:\Program Files (x86)\NVIDIA Corporation\PhysX\Common;C:\Program Files\dotnet\;C:\xampp\php;C:\ProgramData\ComposerSetup\bin;C:\Program Files\Git\cmd;C:\Program Files\NVIDIA Corporation\NVIDIA app\NvDLISR;C:\Users\Super\AppData\Local\Microsoft\WindowsApps;C:\Users\Super\AppData\Local\Programs\Microsoft VS Code\bin;C:\Users\Super\AppData\Roaming\Composer\vendor\bin;C:\Users\Super\.lmstudio\bin


set CC="C:/msys64/mingw64/bin/gcc.exe"
set CXX=C:/msys64/mingw64/bin/g++.exe


c:/msys64/mingw64/bin/mingw32-make.exe %*

REM SET PATH=%PATH%;c:/msys64/mingw64/bin/