# Cinnamon

A 2D game engine written from scratch in modern C++17 and Vulkan.

**Archived** - developed between 2022–2023.

https://github.com/user-attachments/assets/f393a9ca-364f-40c8-978d-be5b3ea5bf1c

*2,600 physics-driven boxes raining into a bowl — simulated and rendered in real time in the editor.*

## Overview

Cinnamon is a from-the-ground-up engine project. No frameworks, no engine middleware. Everything from the Vulkan renderer through the math library up to the editor was hand-written. The only third-party runtime dependencies are Box2D (physics) and Dear ImGui (editor UI).

## Features

- **Vulkan renderer** — batched 2D quad rendering with a deferred lighting pipeline (G-buffer prepass + fullscreen lighting pass), point lights, line/debug rendering
- **Editor** — scene hierarchy, entity inspector, gizmos, content browser, in-editor play/pause/stop with separate edit- and runtime-scene state
- **ECS scene model** — custom entity-component registry with YAML scene serialization
- **2D physics** — Box2D integration: rigid bodies, box colliders, restitution, collider visualization
- **[CinMath](https://github.com/CzekoladowyKocur/CinMath)** — companion SIMD-accelerated (SSE/AVX) linear algebra library, written for this engine
- **Custom tooling** — memory allocation tracking, file watching, Premake-based build
