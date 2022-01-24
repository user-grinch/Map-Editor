@echo off
echo --------------------------------------------------
echo Building ReleaseSA
echo --------------------------------------------------
echo[
call "tools\Setup.bat"
MsBuild MapEditorSA.sln /property:Configuration=Release 
