# Cinnamon

![Status](https://img.shields.io/badge/status-archived-blue)
![C++](https://img.shields.io/badge/C%2B%2B-17-blue)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue)](LICENSE)
![Platforms](https://img.shields.io/badge/platforms-Windows%20%7C%20Linux-blue)

> [!NOTE]
> **This project is no longer being developed.** It was created
> between 2022-2023 as a from-scratch engine project with minimal
> dependencies and is preserved here in its final state.

https://github.com/user-attachments/assets/f393a9ca-364f-40c8-978d-be5b3ea5bf1c

*2,600 physics-driven boxes raining into a bowl - simulated and rendered in real time in the editor.*

---

## Overview

The core of the engine is hand-written. SIMD-accelerated math library, a Vulkan renderer with batched
2D rendering, a custom ECS with YAML scene serialization, and an editor with a working play mode.
Box2D was used for physics and Dear ImGui for the editor UI.

## Features

| Area | Highlights |
|------|-----------|
| **Vulkan renderer** | Batched 2D quad rendering, line/debug rendering, custom `.shader` format bundling vertex and fragment stages |
| **Editor** | Scene hierarchy, entity inspector, gizmos, content browser, texture import, world settings, project files, in-editor play/pause/stop with separate edit- and runtime-scene state |
| **Standalone runtime** | Separate runtime application that loads and plays serialized scenes outside the editor |
| **ECS scene model** | Custom entity-component registry, YAML scene serialization, asset manager |
| **2D physics** | Box2D integration: rigid bodies, box colliders, restitution, collider visualization |
| **Cross-platform core** | Platform abstraction layer with Windows (Win32) and Linux (Wayland) windowing, filesystem and input backends |
| **[CinMath](https://github.com/CzekoladowyKocur/CinMath)** | SIMD-accelerated (SSE/AVX) linear algebra library |
| **Application framework** | Layer stack, event system (keyboard, mouse, window), input abstraction, logging |
| **Custom tooling** | Memory allocation tracking and custom allocators, file watching, Premake-based build with Python setup scripts (including Vulkan SDK download) |

---

*It was a lot of fun to build, but it's a learning project of a 16-year-old at heart. Expect
rough edges and unconventional solutions rather than production-grade
engineering!*
