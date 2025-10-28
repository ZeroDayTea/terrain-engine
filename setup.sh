#!/bin/bash

echo -e "Setting up dependencies for Terrain Engine..."

mkdir -p include

if [ ! -d "include/glfw" ]; then
    cd include
    git clone --depth 1 https://github.com/glfw/glfw.git glfw-3.4
    cd ..
else
    echo "GLFW already downloaded"
fi

if [ ! -d "include/glm"  ]; then
    cd include
    git clone --depth 1 https://github.com/g-truc/glm.git glm-temp
    mv glm-temp/glm .
    rm -rf glm-temp
    cd ..
else
    echo "GLM already downloaded"
fi

if [ ! -d "include/glad" ]; then
    cd include
    mkdir glad
    unzip glad.zip -d glad
    cd ..
else
    echo "Glad already downloaded"
fi

echo -e "Dependencies setup complete"
echo "You can now build the project by running:"
echo "mkdir build"
echo "cd build"
echo "cmake .."
echo "make"
