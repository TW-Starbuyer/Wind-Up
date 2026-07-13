![Wind-Up Engine](src/assets/images/wind-up-engine.png)

### WARNING: Currently in pre-release, not ready for full application development. License TBD, not currently available for application licensing.

Wind-Up is a 3D game engine currently in early development with a focus on radical modularity, experimental game development, and broad adoptability. The engine is currently an independent project by @TW-Starbuyer.

##### Current Modules:
- 🎮 Devices
- 👾 ECS (Flecs)
- 🏰 Levels
- 📈 Profiler
- 🎥 Renderer (SDL GPU)
- 💾 Resources
- 💻 Threading
- 🕰️ Time
- ⌨️ User I/O (SDL3)
- 🗔 Windowing (SDL3)

##### Current Features:
The engine currently features an SDL3-GPU renderer, FLECS entity-component system, and an ImGUI based UI among others.

##### Developer Roadmap:
☐ Lua scripting (probably via Sol3 library)
☐ Interactive runtime level editor
☐ Jolt physics library integration
☐ Expanding multithreading implementation across engine
☐ First demo game release on itch.io
☐ Develop new custom Vulkan renderer module.

##### Assets
Secret Area-52 Room - https://skfb.ly/oLtSy

##### Building
Wind-Up uses the CMake build system for portability. The engine has been tested on x86 CPUs thus far. CMake fetch is used for non-packaged dependencies so it might take a little while at initial build-time. Just clone the repository and build the "Wind-Up" target.
