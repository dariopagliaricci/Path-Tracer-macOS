# Path Tracer (macOS Apple Silicon)

A small CPU Monte Carlo path tracer in C++.  
Forked from **James Griffin’s** project: https://github.com/JamesGriffin/Path-Tracer (original README excerpted here).

The original project by James Griffin implements a simple path tracer with OBJ mesh loading, KD-tree acceleration, and diffuse/specular/emission materials.  

This fork adds:

- **macOS / Apple Silicon** one-command build via CMake Presets (Homebrew **LLVM**)
- **OpenMP** multithreading enabled by default (with LLVM)
- **Thread-safe progress bar** (monotonic %, rows, ETA)
- **RNG seed fix** for `erand48` (no narrowing warnings)
- **INFINITY → finite sentinel** (no UB warnings under `-ffast-math`)
- **VS Code** IntelliSense alignment (uses `compile_commands.json`)

> All rendering logic and scene code are by James Griffin; this fork focuses on build polish and small runtime QoL improvements.

---

## Features (from the original project)

- Mesh rendering
- OBJ model import
- PNG textures (via [lodepng](https://github.com/lvandeve/lodepng))
- Simple KD-tree for meshes
- Diffuse, specular, emission materials

---

## Build (macOS, Apple Silicon & Intel)

### Recommended: Homebrew LLVM + OpenMP (no env vars)

This repo includes `CMakePresets.json`. Just:

```bash
# from project root
rm -rf build
cmake --preset release-llvm
cmake --build build -j
```
Run:
```
$ ./pathtracer <number of samples>
```
This will render the Stanford Dragon scene which is included.
Take a look at src/main.cpp to see how to create a scene and import objs.

## Sample Images
![Dabrovic Sponza](doc/example_renders/sponza.png?raw=true "Dabrovic Sponza")

Dabrovic Sponza - http://hdri.cgtechniques.com/~sponza/files/


![Stanford Dragon](doc/example_renders/dragon.png?raw=true "Stanford Dragon")

Stanford Dragon - http://graphics.stanford.edu/data/3Dscanrep


![Stanford Lucy](doc/example_renders/lucy.png?raw=true "Stanford Lucy")

Stanford Lucy  - http://graphics.stanford.edu/data/3Dscanrep

![Cornell Box](doc/example_renders/cornell.png?raw=true "Cornell Box")

Cornell Box

## Changelog (this fork)
- Add macOS-first CMake preset (Homebrew LLVM + OpenMP)
- Add progress bar (thread-safe, shows ETA)
- Fix erand48 seeding (no unsigned short narrowing)
- Replace INFINITY usage under -ffast-math with large finite sentinel
- Improve VS Code experience (compile_commands, tidy constructors)
- Provide a simple spheres-only Cornell for quick testing

## Credits
- Upstream repo: https://github.com/JamesGriffin/Path-Tracer
- Libraries: [tinyobjloader], [lodepng]

## License
- Original code © James Griffin — GNU GPL v2.0 (see LICENSE).
- This fork (build scripts and small patches) remains GPL v2.0.
