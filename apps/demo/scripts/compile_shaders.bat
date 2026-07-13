@echo off
set SHADER_DIR=..\assets\shaders
set OUTPUT_DIR=..\assets\shaders

for %%f in (%SHADER_DIR%\*.vert.hlsl) do (
    echo Compiling %%f...
    glslc -fshader-stage=vert -x hlsl %%f -o %OUTPUT_DIR%\%%~nf.spv
)

for %%f in (%SHADER_DIR%\*.frag.hlsl) do (
    echo Compiling %%f...
    glslc -fshader-stage=frag -x hlsl %%f -o %OUTPUT_DIR%\%%~nf.spv
)

echo Done.
pause