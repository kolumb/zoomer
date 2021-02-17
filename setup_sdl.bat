@echo off
curl -fsSL -o SDL2-devel-2.0.12-VC.zip https://www.libsdl.org/release/SDL2-devel-2.0.12-VC.zip
tar -xf SDL2-devel-2.0.12-VC.zip
move SDL2-2.0.12 SDL2
del SDL2-devel-2.0.12-VC.zip