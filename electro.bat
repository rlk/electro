rem ---------------------------------------------------------------------------
rem This batch file acts as a drag-and-drop target for Electro Lua scripts.

%~d1
cd "%~dp1"
"%~dp0electro.exe" -f %1

rem ---------------------------------------------------------------------------
rem Configure with a default display config as follows:
rem
rem "%~dp0electro.exe" -f "%~dp0\config\geowall-side-by-side.lua" -f %1
