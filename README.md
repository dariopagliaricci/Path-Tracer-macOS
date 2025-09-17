# Path Tracer (macOS Apple Silicon)

A small CPU Monte Carlo path tracer in C++.  
Forked from **James Griffin’s** project: https://github.com/JamesGriffin/Path-Tracer

This fork adds:

- **macOS / Apple Silicon** one-command build via CMake Presets (Homebrew **LLVM**)
- **OpenMP** multithreading enabled by default (with LLVM)
- **Thread-safe progress bar** (monotonic %, rows, ETA)
- **RNG seed fix** for `erand48` (no narrowing warnings)
- **INFINITY → finite sentinel** (no UB warnings under `-ffast-math`)
- **VS Code** IntelliSense alignment (uses `compile_commands.json`)
- **Specular and emissive materials** (in addition to diffuse)
- **Runtime mesh selection**: `--mesh <preset>` (e.g. `lucy`, `sculpt`) and `--obj <path>` for custom OBJs

---

## Features (from the original project)

- Mesh rendering
- OBJ model import
- PNG textures (via [lodepng](https://github.com/lvandeve/lodepng))
- Simple KD-tree for meshes
- Diffuse-only materials (original project)  

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
Dabrovic Sponza - http://hdri.cgtechniques.com/~sponza/files/

## CLI (optional, local)

By default, the camera is fixed to guarantee reproducibility in CI and comparable results:

- Position: `Vec(0, -4, 1.0)`
- Target: `Vec(0, 0, 1)`

To enable camera flags for local-only use, turn on the CMake option `EXPERIMENTAL_CAMERA_CLI`:

1) Configure the build with the option:

```
cmake -S . -B build -DEXPERIMENTAL_CAMERA_CLI=ON
cmake --build build -j
```

2) Use flags at runtime (accepts `x,y,z` or space-separated):

- `--cam-pos x,y,z`     (e.g., `--cam-pos 0,-3.5,1.0`)
- `--cam-target x,y,z`  (e.g., `--cam-target 0,0,1`)

Example:

```
./build/pathtracer --samples 256 --cam-pos 0,-3.5,1.0 --cam-target 0,0,1
```

Notes:
- If you don’t pass these flags, behavior is identical to the default.
- Don’t use these flags in CI or reference workflows; they’re intended for local iteration.

This will render the Stanford Dragon scene which is included.
Take a look at src/main.cpp to see how to create a scene and import objs.

```bash
# presets
./build/pathtracer 256 --mesh lucy
./build/pathtracer --samples 400 --mesh sculpt

# custom OBJ
./build/pathtracer --obj ./obj/lucy01.obj --samples 128

# legacy flags (still accepted for lucy)
./build/pathtracer 200 --scene=lucy
./build/pathtracer 200 --lucy
```

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
- Add specular and emissive materials
- Add runtime mesh selection via CLI

## Credits
- Upstream repo: https://github.com/JamesGriffin/Path-Tracer
- Libraries: [tinyobjloader], [lodepng]

## License
- Original code © James Griffin — GNU GPL v2.0 (see LICENSE).
- This fork (build scripts and small patches) remains GPL v2.0.
