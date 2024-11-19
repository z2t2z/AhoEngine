# AhoEngine

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Language](https://img.shields.io/badge/language-C++-blue.svg)](https://isocpp.org/)

## Background

Aho Engine is a developmental outcome of my master's studies. This engine serves as a platform to integrate the knowledge I have learned, while also allowing me to enhance my programming skills in C++. Currently, the engine is in its early stages, with only some basic features implemented. In the future, I hope to continuously maintain and update the engine, implementing interesting algorithms along the way.

## Current Features

- **Multiple Modern Graphics Techniques**: Implements PBR (Physically Based Rendering), SSR (Screen Space Reflections), SSAO(Screen Space Ambient Occlusion), PCSS(Percentage Closer Soft Shadow), IBL(Image-Based Lighting).
- **Component System**: Based on an ECS (Entity Component System) architecture for easy extensibility and maintenance.
- **Animation**: Uses assimp library as the parser. Supports various animation formats including .fbx and .dae, and inverse kinematics (IK) functionalities.
- **Debug Tools**: Built-in debugging tools for visualizing or editing skeletons; Hot-reload filewatcher for editing shaders without re-start the project; Object outline highlight when being selected;

## Project Structure

- **...**

## Showcases

![Animation:IK](https://github.com/user-attachments/assets/d8a56853-4e1e-446d-b9ed-bd757232991b)

![Animation:With skeleton view](https://github.com/user-attachments/assets/ceafea86-a0c7-42bb-b348-9e1d8d9b2635)

![SSR](https://github.com/user-attachments/assets/3a80dc1b-7850-46b5-bbb9-ef12caaccb92)

More images to upload;

## Features to Implement

- **Path Tracer**
- **Physics Engine**
- **Scripting System**
- ...

### How to build:

- Use ``git clone --recurse-submodules https://github.com/z2t2z/AhoEngine`` to recursively clone the whole project.
- Enter AhoEngine/Vendor/assimp folder, use cmake to build assimp, the commands are:  ``cmake CMakeLists.txt`` and ``cmake --build``.
- Click Generate.bat file in the root folder.
- Open .sln file and generate the project.
