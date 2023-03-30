@echo off
setlocal enabledelayedexpansion

set cc_flags=
for /f "delims=" %%x in (compile_flags.txt) do (set cc_flags=!cc_flags! %%x)

set "l_flags=-luser32.lib -lgdi32.lib -l..\third_party\vulkan\Lib\vulkan-1.lib"

clang!cc_flags! -D_DEBUG --debug -O0 ../src/main.cpp !l_flags! -Wl,-nodefaultlib:libcmt -lmsvcrtd.lib --output=debug/app.exe

endlocal
