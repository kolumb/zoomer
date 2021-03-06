@echo off
set CFLAGS=/std:c11 /O2 /FC /W4 /WX /Zl /D_USE_MATH_DEFINES /wd4996 /wd4204 /nologo
set INCLUDES=/I Dependencies\GLFW\include /I Dependencies\GLEW\include
set LIBS=Dependencies\GLFW\lib\glfw3.lib Dependencies\GLEW\lib\glew32s.lib opengl32.lib User32.lib Gdi32.lib Shell32.lib

if not exist build mkdir build
cl %CFLAGS% %INCLUDES% src\zoomer.c /Fo:build\ /Fe:build\zoomer.exe %LIBS% /link /NODEFAULTLIB:libcmt.lib /SUBSYSTEM:WINDOWS

if "%1" == "-run" (
    .\build\zoomer.exe 2>&1 1 > .\build\output.log
    @echo on
    type .\build\output.log
)
