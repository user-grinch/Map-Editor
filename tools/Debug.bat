@echo off
echo --------------------------------------------------
echo Building DebugSA
echo --------------------------------------------------
echo[
call "tools\Setup.bat"
MsBuild MapEditorSA.sln /property:Configuration=Debug 

