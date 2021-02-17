@echo off

cl src\main.c /Fo:build\ /Fe:build\zoomer.exe /nologo ^
    /link Shell32.lib Gdi32.lib User32.lib /OUT:build\zoomer.exe -SUBSYSTEM:console

if %errorlevel% == 0 (
    .\build\zoomer 2>&1 1 > build\output.log
    SET /p output_variable=<build\output.log
    @echo on
    type build\output.log
)
