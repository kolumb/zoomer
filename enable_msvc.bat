@echo off

if not defined DevEnvDir (
    if "%VSWHERE%"=="" set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
)
if not defined DevEnvDir (
    for /f "usebackq tokens=*" %%i in (`call "%VSWHERE%" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
      set InstallDir=%%i
    )
)
if not defined DevEnvDir (
    if exist "%InstallDir%\VC\Auxiliary\Build\vcvars64.bat" (
        call "%InstallDir%\VC\Auxiliary\Build\vcvars64.bat"
    )
)