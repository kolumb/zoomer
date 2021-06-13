# Zoomer

Port of zoomer app [Boomer](https://github.com/tsoding/boomer/) that was created by [Tsoding](https://twitch.tv/tsoding).

Boomer is written in Nim for Linux. Zoomer is ported to C and uses Win32 API.

## Dependencies

[Microsoft Visual Studio compiler](https://visualstudio.microsoft.com/downloads/)

## Build

### MSVC

```console
> # Download and setup OpenGL dependencies
> setup_glfw.bat

> # Enable MSVC console development enviroment.
> # You can skip this step if already launched Visual Studio Developer Command Prompt
> enable_msvc.bat

> build_msvc.bat
> build\zoomer

> # Or with one command
> build_msvc.bat -run
```

### MSYS2 (mingw-w64-x86_64)

```console
> # Download and setup OpenGL dependencies
> pacman -S mingw-w64-x86_64-glfw mingw-w64-x86_64-glew

> # Build
> cc src/zoomer.c -o zoomer -lglfw3 -lglew32 -lopengl32 -lGdi32
> ./zoomer.exe
```

## Controls

| Key | Action |
|-----|--------|
| `Escape` / `Q` | Quit |
| `0` | Reset zoom |
| `Scroll wheel` | Zoom +/- |
| `+` / `-` | Zoom in/out |
| `Mouse button` | Pan |
| `WASD` / `Arrows` | Move around |
| `F` | Toggle flashlight |
| `Ctrl` + `Scroll wheel` | Change size of flashlight |
