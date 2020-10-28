@echo off

setlocal
pushd %~dp0

if "%1" == "" goto usage
if not "%2" == "" goto usage

if "x%VSDIR%" == "x" set VSDIR=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community

set BUILD_TYPE=none
if /i %1 == clean goto clean
if /i %1 == Debug set BUILD_TYPE=Debug
if /i %1 == Release set BUILD_TYPE=Release
if %BUILD_TYPE% == none goto usage
goto build

:usage
echo Usage:
echo     %~nx0 {build_type,clean}
echo.
echo Build plugin with specified build_type or clean build temp
echo.
echo Build types (case insensitive):
echo     Debug        Build for debugging
echo     Release      Optimized build
goto exit

:clean
rmdir /s/q build\temp
goto exit

:build
rem Try to use msbuild
msbuild --help >nul 2>&1
if not errorlevel 1 (
    call :build_with_msbuild
    goto exit
)

rem Try to activate Visual Studio 2019 environment
if exist "%VSDIR%\VC\Auxiliary\Build\vcvars32.bat" (
    call "%VSDIR%\VC\Auxiliary\Build\vcvars32.bat"
    call :build_with_msbuild
    goto exit
)

rem Try to use cmake
cmake --help >nul 2>&1
if not errorlevel 1 (
    call :build_with_cmake
    goto exit
)

echo Neither cmake nor msbuild was found.
echo You can try the following:
echo 1) Specify the path to Visual Studio in the VSDIR environment variable
echo 2) Run %~nx0 from Developer Command Prompt environment
goto exit

:build_with_msbuild
msbuild plugin.sln /p:Configuration=%BUILD_TYPE% /p:Platform=win32 /t:Clean,Build
goto :eof

:build_with_cmake
set REPO_ROOT=%~dp0
set REPO_ROOT=%REPO_ROOT:~0,-1%

set PROJECT_NAME=plugin
set OUTPUT_NAME=plugin
set OUTPUT_EXT=dll
call :build_project
goto :eof

:build_project
set PROJECT_DIR=%REPO_ROOT%\src\%PROJECT_NAME%
set OUTPUT_DIR=%REPO_ROOT%\build\%BUILD_TYPE%
set BUILD_DIR=%REPO_ROOT%\build\temp\%BUILD_TYPE%\%PROJECT_NAME%
set CMAKE_BUILD_TYPE=%BUILD_TYPE%
set CMAKE_EXTRA_ARGS=-A Win32

rem Use NMake if possible
nmake /? >NUL 2>&1
if not errorlevel 1 ( set CMAKE_EXTRA_ARGS=-G "NMake Makefiles" ) else cd .

rem Always generate debug info
if %BUILD_TYPE% == Release set CMAKE_BUILD_TYPE=RelWithDebInfo

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
if errorlevel 1 goto :eof
pushd "%BUILD_DIR%"

echo Configuring %PROJECT_NAME%
rem Useful macro -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON
cmake %CMAKE_EXTRA_ARGS% ^
      -DCMAKE_BUILD_TYPE=%CMAKE_BUILD_TYPE% ^
      "%PROJECT_DIR%"
if errorlevel 1 goto build_project_end

echo Building %PROJECT_NAME%
cmake --build . --config "%CMAKE_BUILD_TYPE%"
if errorlevel 1 goto build_project_end

echo Copying files
if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"
if errorlevel 1 goto build_project_end

if exist "%OUTPUT_NAME%.%OUTPUT_EXT%" (
    copy "%OUTPUT_NAME%.%OUTPUT_EXT%" "%OUTPUT_DIR%"
    copy "%OUTPUT_NAME%.pdb" "%OUTPUT_DIR%"
) else (
    copy "%CMAKE_BUILD_TYPE%\%OUTPUT_NAME%.%OUTPUT_EXT%" "%OUTPUT_DIR%"
    copy "%CMAKE_BUILD_TYPE%\%OUTPUT_NAME%.pdb" "%OUTPUT_DIR%"
)
if errorlevel 1 goto build_project_end

echo %PROJECT_NAME% was successfully built

:build_project_end
popd
goto :eof

:copy_fail

:exit
popd
endlocal
