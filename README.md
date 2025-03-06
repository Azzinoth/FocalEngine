# Focal Engine

![build](https://github.com/Azzinoth/FocalEngine/actions/workflows/Build.yml/badge.svg?branch=master)

Focal Engine is an open-source, modular real-time 3D rendering and game engine designed for building interactive 3D applications, games, and scientific visualizations. It excels at handling complex geospatial data and GPU-driven rendering, empowering developers and researchers to create sophisticated graphical solutions efficiently.

⚠️ **Currently in Alpha**: Expect breaking changes and ongoing development.

![3D scene in the Focal Engine](https://github.com/Azzinoth/FocalEngine/blob/media/1.png)
![3D scene in the Focal Engine](https://github.com/Azzinoth/FocalEngine/blob/media/2.png)
![3D scene in the Focal Engine](https://github.com/Azzinoth/FocalEngine/blob/media/4.png)
![3D scene in the Focal Engine](https://github.com/Azzinoth/FocalEngine/blob/media/6.png)

## Table of Contents
- [Focal Engine Ecosystem](#focal-engine-ecosystem)
- [Projects Using Focal Engine](#projects-using-focal-engine)
- [Features](#features)
  - [Graphical Features](#graphical-features)
  - [Point Cloud Handling](#advanced-point-cloud-handling)
  - [Terrain](#terrain)
  - [Performance](#performance)
  - [VR Support](#vr-support)
  - [Entity Component System (ECS)](#entity-component-system-ecs)
    - [Prefab System](#prefab-system)
  - [C++ Scripting](#c-scripting)
- [Editor](#editor-overview)
- [Build and Setup](#build-system)
- [Testing](#testing)
- [Example Application](#simple-application-example)
- [Third-Party Licenses](#third-party-licenses)

## Focal Engine Ecosystem

The Focal Engine project consists of four modular components that work together to provide a complete development environment:

[Basic Application Module](https://github.com/Azzinoth/FEBasicApplication) - A foundation layer for OpenGL and ImGui applications that provides essential utilities including time measurement, thread pooling, logging, TCP networking, and profiling capabilities.

[Visual Node System](https://github.com/Azzinoth/VisualNodeSystem) - A framework for creating visual node-based interfaces with features like zoom, reroute nodes, group comments, and JSON serialization, ideal for material editors and visual scripting.

Focal Engine (this repository) - The engine with all core functionality.

[Focal Engine Editor](https://github.com/Azzinoth/FocalEngineEditor) - A comprehensive editor for the engine.

This modularity makes it easier to include just the engine in applications that don't need the editor's complexity. It also simplifies the implementation of export functionality in the editor, allowing users to compile their projects into standalone executable applications with all necessary resources.

## Projects Using Focal Engine

[HabiCAT 3D](https://github.com/Azzinoth/HabiCAT3D) - An open-source software that implements novel algorithms for generating multi-scale complexity metrics maps(like rugosity, fractal dimension, vector dispersion and others) for complex 3D habitat models.

## Features

### Graphical Features

- Physically-based rendering
- High dynamic range (HDR) rendering internally
- Gamma correction
- Deferred shading
- Screen-Space Ambient Occlusion (SSAO)
- Cascaded soft shadows
- Sky with atmospheric scattering
- Compressed textures support with multi-threaded loading
- Fast Approximate Anti-Aliasing (FXAA)
- Dynamic render scale adjustment
- Camera temporal jitter and partial motion vector calculations, needed for future TAA and third-party upscalers
- Each camera has its own unique rendering pipeline with customizable settings

### Advanced Point Cloud Handling

The Focal Engine provides specialized capabilities for working with large-scale point cloud data:

- LAS/LAZ File Format Support

  ⚠️ Work in progress ⚠️

- High-Performance Rendering: Support for real-time visualization of massive point clouds (hundreds of millions to billions of points)  
  ⚠️ Work in progress ⚠️

- GPU-Accelerated Editing: Tools for manipulating and editing huge point clouds directly in GPU memory  
  ⚠️ Work in progress ⚠️

### Terrain

- Chunked terrain with tessellation for LOD
- Frustum culling of sub chunks
- Up to 8 terrain layer materials
- Each layer can define foliage spawn rules

### VR Support

The Focal Engine leverages OpenXR integration to provide support for a wide range of VR headsets and controllers. The engine has a virtual UI system specifically designed for comfortable and intuitive interaction in VR space.
  
![3D scene in the Focal Engine](https://github.com/Azzinoth/FocalEngine/blob/media/8.png)

### Entity Component System (ECS)

Focal Engine employs EnTT ECS for optimized performance and flexible architecture. Easily compose entities from modular components, enabling scalable and maintainable code.

#### Prefab System

Easily define reusable entities or groups of entities as Prefabs. Prefabs encapsulate entities and their component configurations as scenes, allowing you to instantiate complex object setups multiple times efficiently. Prefabs simplify asset management and accelerate level design.

### C++ Scripting: 

Attach custom scripts directly to entities for rapid prototyping, modular gameplay logic, and efficient runtime performance.

## Performance

Focal Engine features a GPU-driven rendering pipeline (⚠️ Work in progress ⚠️: currently not working with all entity components) that includes GPU Frustum Culling and GPU Occlusion Culling. The latter utilizes a custom Hierarchical Z-Buffer (HZB). Both culling techniques are implemented using compute shaders. These advanced optimization techniques enable the Focal Engine to render a substantial number of objects per scene (up to millions, depending on object types).

### Profiling

Engine submodule includes a profiling system that provides detailed analysis of CPU utilization across threads. This system is  designed to handle complex, highly multithreaded workloads, allowing to identify and address performance bottlenecks.

![3D scene in the Focal Engine](https://github.com/Azzinoth/FocalEngine/blob/media/7.png)

## Editor Overview

The Focal Engine Editor provides project management capabilities through its project browser window:

![Project Browser](https://github.com/Azzinoth/FocalEngineEditor/blob/media/Project%20Browser.png)

The editor interface includes several key windows:

1. **Scene Entities**: Displays entities in a hierarchical graph structure.
2. **Scene/Scenes**: Supports multiple scene windows simultaneously. Prefabs are handled as simplified scenes for modular design.
3. **Inspector**: Provides list of components of the selected entity for viewing and editing.
4. **Content Browser**: Serves as an explorer for the project's virtual file system with structured access to all resources.
5. **Editor Cameras**: Provides settings for editor-specific cameras. While game cameras (entities with camera components) are only functional in game mode, editor cameras allow navigation through scenes even when no game camera is present.
6. **Log Window**: Displays filtered categories of warnings, errors, and informational messages for debugging.

![Log window](https://github.com/Azzinoth/FocalEngineEditor/blob/media/Log%20window.png)

Standard editor layout:

![Editor](https://github.com/Azzinoth/FocalEngineEditor/blob/media/Editor.png)

The Material Editor employs the [Visual Node System](https://github.com/Azzinoth/VisualNodeSystem) to enhance editing intuitiveness.

![Material Editor](https://github.com/Azzinoth/FocalEngineEditor/blob/media/Material%20editor.png)

Terrain editing is facilitated with different brushes (sculpt, smooth, paint layers). Each terrain can support up to eight material layers, with each layer capable of enforcing the type of foliage that can spawn on it.

![Terrain](https://github.com/Azzinoth/FocalEngineEditor/blob/media/Terrain.png)

## Build System

⚠️ Work in progress ⚠️

The Focal Engine Editor includes functionality to compile projects into standalone executable (.exe) files, along with packaged resource files containing all necessary scripts, assets, and resources. This allows for easy distribution of completed applications without requiring the editor or engine development environment.


## Shader debugging

Users can edit shaders on-the-fly in the built-in shader editor and debug compilation errors:

![Shader Compilation errors](https://github.com/Azzinoth/FocalEngineEditor/blob/media/Shader%20Compilation%20errors.png)

⚠️ Work in progress ⚠️

In addition, the editor supports real-time retrieval of shader variable values from the GPU. (Please note that not all variables are supported and further testing is needed for this feature.)

![Shader values read back](https://github.com/Azzinoth/FocalEngineEditor/blob/media/Shader%20values%20read%20back.png)

## Testing

During the development of the engine and its tools, inadequate testing was identified as a source of setbacks. To address this, a multi-layered testing approach has been implemented:

1. **Continuous Integration** - GitHub Actions run on every push as a sanity check to ensure the engine compiles successfully.

2. **Unit Testing** - Using Google Test framework to verify individual components. Currently limited to scene graph testing, with plans to expand coverage to more subsystems over time.

3. **Visual Testing Platform** - Due to the graphical nature of the engine, unit tests and GitHub Actions are insufficient for comprehensive testing. A specialized testing platform is being developed that will evaluate the engine's visual output and UI functionality.

This comprehensive testing strategy will enable more confident feature iteration and establish a proper regression testing pipeline.

## Building the Project for Visual Studio (Windows) as .lib

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

## Simple Application Example
An example of a simple application that integrates the Engine: [Example](https://github.com/Azzinoth/FocalEngineAppExample).

## Third Party Licenses

This project uses the following third-party libraries:

1) **GLM**: This library is licensed under a permissive open-source license, similar to the MIT license. The full license text can be found at [GLM's GitHub repository](https://github.com/g-truc/glm/blob/master/copying.txt).

2) **jsoncpp**: This library is licensed under the MIT License. The full license text can be found at [jsoncpp's GitHub repository](https://github.com/open-source-parsers/jsoncpp/blob/master/LICENSE).

3) **lodepng**: This library is licensed under the zlib License. The full license text can be found at [lodepng's GitHub repository](https://github.com/lvandeve/lodepng/blob/master/LICENSE).

4) **stb**: This library is licensed under the MIT License. The full license text can be found at [stb's GitHub repository](https://github.com/nothings/stb/blob/master/LICENSE).

5) **OpenXR**: This library is licensed under Apache 2.0 License. The full license text can be found at [OpenXR's GitHub repository](https://github.com/KhronosGroup/OpenXR-SDK-Source/blob/main/LICENSE).

6) **googletest**: This library is under the BSD-3-Clause license. The full license text can be found at [googletest's GitHub repository](https://github.com/google/googletest?tab=BSD-3-Clause-1-ov-file).

7) **EnTT**: This library is under the MIT License. The full license text can be found at [EnTT's GitHub repository](https://github.com/skypjack/entt?tab=MIT-1-ov-file).