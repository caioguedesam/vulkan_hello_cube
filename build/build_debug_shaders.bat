@echo off
setlocal enabledelayedexpansion

glslc -O0 -g ../resources/shaders/first_triangle.vert -o debug/first_triangle_vs.spv
glslc -O0 -g ../resources/shaders/first_triangle.frag -o debug/first_triangle_ps.spv

endlocal
