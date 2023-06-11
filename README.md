# Focal Engine

Focal Engine is a personal project of a 3D rendering engine, showcasing an array of custom shaders, materials, and rendering techniques.

![3D scene in the Focal Engine](https://github.com/Azzinoth/FocalEngine/blob/media/1.png)
![3D scene in the Focal Engine](https://github.com/Azzinoth/FocalEngine/blob/media/2.png)
![3D scene in the Focal Engine](https://github.com/Azzinoth/FocalEngine/blob/media/3.png)
![3D scene in the Focal Engine](https://github.com/Azzinoth/FocalEngine/blob/media/4.png)
![3D scene in the Focal Engine](https://github.com/Azzinoth/FocalEngine/blob/media/5.png)

## Features

- Physically-based rendering
- High dynamic range (HDR) rendering internally
- Gamma correction
- Deferred shading
- Screen-Space Ambient Occlusion (SSAO)
- Cascaded soft shadows
- Fast Approximate Anti-Aliasing (FXAA)
- Sky with atmospheric scattering
- Compressed textures support with multi-threaded loading

## Terrain

- Chunked terrain with tessellation for LOD
- Frustum culling of sub chunks
- Up to 8 terrain layer materials
- Each layer can define foliage spawn rules

## Performance

All instanced entities will have a pass with multiple computer shaders that perform LOD level calculations, Frustum and Occlusion culling entirely on the GPU.

## Building the Project for Visual Studio (Windows)

```bash
# Initialize a new Git repository
git init

# Add the remote repository
git remote add origin https://github.com/Azzinoth/FocalEngine

# Pull the contents of the remote repository
git pull origin master

# Initialize and update submodules
git submodule update --init --recursive

# Generate the build files using CMake
# Will work in Windows PowerShell
cmake CMakeLists.txt
```

## Scene Editor
This repository only contains the engine itself to maintain modularity. For the full experience, it is recommended to use the [Focal Engine Editor](https://github.com/Azzinoth/FocalEngineEditor).

## Third Party Licenses

This project uses the following third-party libraries:

1) **GLM**: This library is licensed under a permissive open-source license, similar to the MIT license. The full license text can be found at [GLM's GitHub repository](https://github.com/g-truc/glm/blob/master/copying.txt).

2) **jsoncpp**: This library is licensed under the MIT License. The full license text can be found at [jsoncpp's GitHub repository](https://github.com/open-source-parsers/jsoncpp/blob/master/LICENSE).

3) **lodepng**: This library is licensed under the zlib License. The full license text can be found at [lodepng's GitHub repository](https://github.com/lvandeve/lodepng/blob/master/LICENSE).

4) **stb**: This library is licensed under the MIT License. The full license text can be found at [stb's GitHub repository](https://github.com/nothings/stb/blob/master/LICENSE).
