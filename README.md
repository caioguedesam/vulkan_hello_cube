# Vulkan Hello Cube

My first project using the Vulkan graphics API.

This was mostly an experiment to learn Vulkan, and as such there's no concern for code quality, readability, maintainablity, etc.

The application renders two rotating cubes at a fixed angle, with proper texture mapping (texture assets not included) and depth testing.

![result](https://i.imgur.com/9kLMCby.gif)
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
