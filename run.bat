@echo off
rem /O2
set CFLAGS=/std:c11 /FC /W4 /WX /Zl /D_USE_MATH_DEFINES /wd4996 /nologo
set INCLUDES=/I Dependencies\GLFW\include /I Dependencies\GLEW\include
set LIBS=Dependencies\GLFW\lib\glfw3.lib Dependencies\GLEW\lib\glew32s.lib opengl32.lib User32.lib Gdi32.lib Shell32.lib

cl %CFLAGS% %INCLUDES% src\zoomer.c /Fo:build\ /Fe:build\zoomer.exe %LIBS% /link /NODEFAULTLIB:libcmt.lib

if %errorlevel% == 0 (
    .\build\zoomer 2>&1 1 > build\output.log
    SET /p output_variable=<build\output.log
    @echo on
    type build\output.log
)
