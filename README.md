
# Cyclops Studio

Cyclops Studio is a high-performance, hardware-accelerated 2D animation and drawing engine written in C++23. It is designed to test the limits of low-latency rendering and batched geometry processing for professional-grade digital art tools.

The project is architected as a modular game engine (`Cyclops Engine`) decoupled from the editor application (`Cyclops App`).

## Status & Roadmap

**Current Phase: V-B (Optimization & Core Refactor) - COMPLETED**
**Next Phase: VI (Batch Rendering Architecture)**

| System | Status | Implementation Details |
| :--- | :--- | :--- |
| **Core Architecture** | ✅ Stable | Singleton Application, Delta Time Loop, Platform Detection |
| **Memory Management** | ✅ Stable | Strict RAII (`std::unique_ptr`, `std::shared_ptr`), No raw `new`/`delete` |
| **Event System** | ✅ Stable | Blocking Event Bus, Template-based Dispatcher, Window/Input Polling |
| **Layer Stack** | ✅ Stable | `std::vector` stack with forward update / reverse event propagation |
| **UI / Editor** | ✅ Stable | Dear ImGui (Docking Branch), Viewport Framebuffers, Custom Panels |
| **Profiling** | ✅ Stable | Macro-based instrumentation emitting Chrome Tracing JSON |
| **Brush Engine** | ⚠️ Alpha | Basic Splatting, Linear Interpolation (LERP), Independent Shader State |
| **Renderer 2D** | 🚧 Planned | Batch Rendering, Texture Atlasing, Draw Call Optimization |
| **Animation** | 🚧 Planned | Timeline, Keyframing, Layer Blending |

## Technical Architecture

### 1. The Engine Core
The engine utilizes a **LayerStack** architecture.
* **Updates:** Propagate from the bottom (Overlay) to the top (Game Layer).
* **Events:** Propagate from the top (Overlay) to the bottom. This allows UI layers to intercept and "consume" mouse clicks before they reach the game world (`e.Handled = true`).

### 2. Memory Model
The codebase strictly adheres to **RAII (Resource Acquisition Is Initialization)**.
* **Ownership:** Resources (Framebuffers, Shaders) are owned via `std::unique_ptr`.
* **Sharing:** Shared resources (Layers) use `std::shared_ptr`.
* **Raw Pointers:** Used *only* for non-owning observation/access (e.g., passing a Framebuffer to the Brush Engine).

### 3. Rendering Pipeline (Current)
* **API:** OpenGL 4.6 (Core Profile).
* **Context:** Managed via GLFW and GLAD.
* **Brush Engine:** Currently uses "Splatting" (drawing textured quads at interpolated positions).
* **Compositing:** The main canvas is a Framebuffer Object (FBO) rendered as a texture inside an ImGui window.

## Build Instructions

### Prerequisites
* **OS:** Windows 10/11 (x64)
* **Compiler:** MSVC (Visual Studio 2022) with C++23 support.
* **Build System:** CMake (Integrated into VS) or Console.

### Setup
This repository uses Git Submodules for dependencies (GLFW, GLM, ImGui, GLAD).

```bash
# 1. Clone recursively to pull dependencies
git clone --recursive [https://github.com/YOUR_USERNAME/CyclopsStudio.git](https://github.com/YOUR_USERNAME/CyclopsStudio.git)

# 2. If you cloned without --recursive, run:
git submodule update --init --recursive

```

### Compiling

1. Open the `CyclopsStudio` folder in **Visual Studio 2022**.
2. VS will detect `CMakeLists.txt` and generate the cache.
3. Select `x64-Debug` or `x64-Release`.
4. Build -> Build All (`Ctrl+Shift+B`).

## Instrumentation & Profiling

Cyclops includes a custom internal profiler located in `Engine/src/Debug/Instrumentor.h`.
To profile a session:

1. Run the application.
2. Perform actions (draw, resize windows).
3. Close the application.
4. A file named `CyclopsProfile.json` will be generated in the build directory.
5. Open `chrome://tracing` or [Perfetto](https://ui.perfetto.dev/) and load the JSON file.

## Dependencies

* [GLFW](https://www.glfw.org/) - Windowing and Input
* [Glad](https://glad.dav1d.de/) - OpenGL Loader
* [Dear ImGui](https://github.com/ocornut/imgui) - Immediate Mode GUI (Docking Branch)
* [GLM](https://github.com/g-truc/glm) - Mathematics

---

*Maintainer: [Souradeep Banerjee](https://github.com/Souradeep1101)*