# 🧩 Aho Engine

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Language](https://img.shields.io/badge/language-C++-blue.svg)](https://isocpp.org/)
[![Platform](https://img.shields.io/badge/platform-Windows-blue.svg)]()
[![Renderer](https://img.shields.io/badge/renderer-OpenGL_4.5-green.svg)]()

---

## 🧠 Background

**Aho Engine** is a developmental outcome of my master's studies.  
It serves as a platform to integrate the knowledge I have learned, while also allowing me to enhance my programming skills in **C++** and **graphics programming**.

Currently, the engine is in its early stages, with only some basic features implemented.  
In the future, I hope to continuously maintain and update the engine, implementing interesting algorithms along the way — from rendering techniques to animation systems.

---

## ✨ Current Features

### 🎮 Rendering
Implements a range of **modern real-time rendering techniques**:
* **Core Rendering Features**
  * Texture Mapping
  * Deferred Shading Pipeline
  * Physically Based Rendering (**PBR**)
  * Image-Based Lighting (**IBL**)
  * Atmospheric & Sky Rendering
* **Screen-Space Techniques**
  * Screen Space Reflections (**SSR**)
  * Screen Space Ambient Occlusion (**SSAO**)
* **Lighting & Shadows**
  * Percentage Closer Soft Shadows (**PCSS**)
  * Ray-Traced Shadows
  * Dynamic Diffuse Global Illumination (**DDGI**)
* **Advanced Rendering**
  * Path Tracing via **Compute Shader** in OpenGL
* **Post Processing**
  * Fast approximate anti-aliasing (FXAA)

### 🧩 Component System
- Built on an **Entity Component System (ECS)** architecture  
  → Easy to extend, maintain, and manage scene entities.

### 💃 Animation
- Uses **Assimp** as the model/animation parser.  
- Supports multiple formats (`.fbx`, `.dae`, etc.).  
- Includes **Inverse Kinematics (IK)** with **FABRIK** and **CCD** solvers.

### 🧰 Debug & Tools
- Shader hot-reloading system.
- Frame time statistics for each render pipeline.
- Built-in tools for **visualizing/editing skeletons**.  
- **Hot-reload filewatcher** for editing shaders without restarting.  
- **Outline highlighting** when selecting objects.  
- Basic **Inspector** panel for transform modification.

---

## 🖼️ Showcases

### 🧭 Editor Layout

Has basic editor features: world grid, object picking, and transform parameters view/edit in **Inspector**.

<img width="800" alt="Editor1" src="https://github.com/user-attachments/assets/aedd37de-c4ea-4794-99ce-412ec9a3243c" />
<img width="800" alt="Editor2" src="https://github.com/user-attachments/assets/d6a9d0b2-8926-4848-b714-2f094525e770" />
<img width="800" alt="Editor3" src="https://github.com/user-attachments/assets/c249280f-ee16-420d-b90e-7454c547a754" />

---

### 🌌 Dynamic Sky & Atmospheric Rendering  
*Referred to [UE5 Sky Atmosphere](https://github.com/sebh/UnrealEngineSkyAtmosphere)*

<img width="800" src="https://github.com/user-attachments/assets/bebcea73-85e0-4e6a-8ed8-166fb6d47d00" />
<img width="800" src="https://github.com/user-attachments/assets/491e830d-01c2-48cf-9014-a21779d78759" />

---

### ⚡ Wavefront Path Tracing *(denoising pending)*

<img width="800" src="https://github.com/user-attachments/assets/23e21144-4f53-4aae-89a0-9e4692298125" />
<img width="800" src="https://github.com/user-attachments/assets/9069f32d-7294-4860-b146-2601a23dd0e2" />

**Transparent materials with different roughness:**

<img width="800" src="https://github.com/user-attachments/assets/e7aa7e61-0125-428b-a95b-d1ad01870f1f" />
<img width="800" src="https://github.com/user-attachments/assets/b63748ee-6e1c-4265-9ca3-c090199142ba" />
<img width="800" src="https://github.com/user-attachments/assets/c36300d8-adc1-482a-8822-78872688676b" />

---

### 🔮 Stochastic Screen Space Reflection (SSSR) *(denoising undone)*

<img width="800" src="https://github.com/user-attachments/assets/72935755-d74f-48fb-985c-c79b94bf25eb" />

**Reference (path-traced result):**

<img width="800" src="https://github.com/user-attachments/assets/17b74ea6-6137-40f2-bcfa-097587112308" />

---

### 🌈 Image-Based Lighting (IBL)
*Referred to multiple open-source implementations*

<img width="800" src="https://github.com/user-attachments/assets/b99b3475-9c9b-4994-a88b-aa135b29b058" />
<img width="800" src="https://github.com/user-attachments/assets/21a3654c-863f-403e-9d17-de7549a42798" />

---

### ☀️ DDGI with Ray-Traced Shadows

<img width="800" src="https://github.com/user-attachments/assets/f8ed2f10-1565-4caa-bff5-b9b9b6406d03" />
<img width="800" src="https://github.com/user-attachments/assets/879d73ab-8850-451a-ae75-66d77b75b299" />
<img width="800" src="https://github.com/user-attachments/assets/4920dbb0-808e-40e6-a61e-efdf3660234f" />

> Notice the subtle **color bleeding** on the pillar.

---

### 🦴 Skeletal Animation

#### 🩻 Bone Visualization
<img width="800" src="https://github.com/user-attachments/assets/f4f08e0f-1993-4c37-b866-3d05f0cea2ac" />

#### 🤖 Inverse Kinematics (FABRIK / CCD)
![Animation:IK](https://github.com/user-attachments/assets/d8a56853-4e1e-446d-b9ed-bd757232991b)
![Animation:With skeleton view](https://github.com/user-attachments/assets/ceafea86-a0c7-42bb-b348-9e1d8d9b2635)

---



## ⚙️ Build Instructions

```bash
# Clone recursively (include submodules)
git clone --recurse-submodules https://github.com/z2t2z/AhoEngine

# Build Assimp (inside Vendor/assimp)
cd AhoEngine/Vendor/assimp
cmake CMakeLists.txt
cmake --build .

# Generate Visual Studio project files
cd ../../
./Generate.bat

# Open .sln and build the project
```

--- 
### ⚙️ Dependencies

The project integrates several open-source libraries to enhance functionality and maintain modularity:
- **OpenGL / GLAD / GLFW** — Graphics API, context creation, and input handling  
- **ImGui** — In-engine debug UI and editor interface  
- **EnTT** — Entity Component System (ECS) framework  
- **Assimp** — Model loading (supports FBX, DAE, OBJ, etc.)  
- **GLM** — Mathematics library (vector/matrix operations)  
- **stb_image** — Image loading and texture management  
- **tinyfiledialogs / filesystem** — File management and dialogs  
- **spdlog** — Lightweight, high-performance logging library  
- **CMake** — Build configuration and dependency management

###
---

### 📚 References

The following resources and literature inspired or supported the development of this project:

- **Real-Time Rendering, 4th Edition** — Tomas Akenine-Möller et al.  
- **Physically Based Rendering: From Theory to Implementation (PBRT)** — Matt Pharr, Wenzel Jakob, Greg Humphreys  
- **UE5 Sky and Atmospheric Scattering Model** — [Sebastien Hillaire, Unreal Engine SkyAtmosphere](https://github.com/sebh/UnrealEngineSkyAtmosphere)  
- **DirectX 12 Sample Projects** — Microsoft / NVIDIA technical samples  
- **Filament Rendering Engine** — Google Open Source  
- **LearnOpenGL** — [learnopengl.com](https://learnopengl.com/) (for early rendering prototypes)  
- **NVIDIA Technical Blogs** — References for DDGI, PCSS, and stochastic rendering methods  

---
