@echo off
pushd %~dp0\..\..\
call ThirdParty\Premake\Windows\Binary\premake5.exe vs2019
popd
PAUSE