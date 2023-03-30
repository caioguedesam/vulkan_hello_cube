# Vulkan Hello Cube

My first project using the Vulkan graphics API. The intent is to render a textured cube that spins at an angle from the camera.

This project has two goals: helping me learn Vulkan and be a starting point to use when porting [Typheus](https://www.google.com) from OpenGL to Vulkan.

As of this moment, I have a hardcoded color-interpolated triangle being rendered.

![result](https://i.imgur.com/llRxrdY.png)
------
### Build instructions

This project is Windows-only. To fully build it, you must have `clang-cl` and the Vulkan SDK installed.

Building it is just a matter of executing both build scripts provided in the build folder. Execute the following commands from the project root:
```
cd build
mkdir debug
.\build_debug_shaders
.\build_debug
```
Then run from the build folder using:
```
.\debug\app
```
