@echo off

if "%PLATFORM%" == "" (
  echo Platform not defined!
  exit /b 1
)

if "%PLATFORM%" == "Win32" (
  set BITS=32
) else if "%PLATFORM%" == "x64" (
  set BITS=64
) else (
  echo Unknown platform!
  exit /b 2
)

nuget install boost-vc140 -version 1.61.0 -outputdirectory boost_1_61_0

pushd boost_1_61_0
mkdir include
mkdir lib

move /y boost.1.61.0.0\lib\native\include include

for /d %%i in (boost_*) do (
  move /y %%i\lib\native\address-model-%BITS%\lib\* lib
)

popd
