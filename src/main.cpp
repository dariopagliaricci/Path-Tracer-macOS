#include <stdio.h>
#include <stdlib.h>
#include "time.h"

#include "vector.h"
#include "material.h"
#include "objects.h"
#include "camera.h"
#include "scene.h"
#include "renderer.h"

#include <string>
#include <cstring>
#include <unordered_map>

//Helper: fetch value for flags supporting both "--key=value" ad "---key value"
static const char* getFlagValue(int argc, char** argv, const char* key) {
    const size_t keyLen = std::strlen(key);
    for (int i = 1; i < argc; ++i) {
        const char* a = argv[i];
        // form: --key=value
        if (std::strncmp(a, key, keyLen) == 0 && a[keyLen] == '=') {
            return a + keyLen + 1;
        }
        // form: --key value
        if (std::strcmp(a, key) == 0 && i + 1 < argc) {
            return argv[i + 1];
        }
    }
    return nullptr;
}

// Simple "x,y,z" parser with comma or space separators.
// Accepts forms like: "1,2,3", "1 2 3", "1.0, -2.5, 3" (spaces allowed)
static bool parseVec3(const char* s, Vec& out) {
    if (!s) return false;
    // Create a small copy we can mutate to normalize separators
    std::string tmp(s);
    for (char& c : tmp) {
        if (c == ',') c = ' ';
    }
    // Now we expect three numbers separated by spaces (possibly multiple)
    const char* p = tmp.c_str();
    char* end = nullptr;
    double x = std::strtod(p, &end);
    if (end == p) return false;
    p = end;
    double y = std::strtod(p, &end);
    if (end == p) return false;
    p = end;
    double z = std::strtod(p, &end);
    if (end == p) return false;
    out = Vec(x, y, z);
    return true;
}

// static bool wantLucy(int argc, char** argv) {
//     for (int i = 1; i < argc; ++i) {
//         if (std::strcmp(argv[i], "--lucy") == 0) return true;
//         if (std::strncmp(argv[i], "--scene=", 8) == 0 && std::strcmp(argv[i]+8, "lucy") == 0) return true;
//     }
//     return false;
// }


int main(int argc, char *argv[]) {

    time_t start, stop;
    time(&start);               // Start execution timer
    int samples = 4;            // Default samples per pixel

    // if (argc == 2) samples = atoi(argv[1]);

    for (int i = 1; i < argc; ++i) {
        const char* a = argv[i];

        // --samples=256
        if (std::strncmp(a, "--samples=", 10) == 0) {
            samples = std::atoi(a + 10);
            if (samples < 1) samples = 1;
            break;
        }

        // --samples 256
        if (std::strcmp(a, "--samples") == 0 && i + 1 < argc) {
            samples = std::atoi(argv[i + 1]);
            if (samples < 1) samples = 1;
            break;
        }

        // positional number (e.g., ./pathtracer 256 --scene=lucy)
        if (std::strspn(a, "0123456789") == std::strlen(a)) {
            samples = std::atoi(a);
            if (samples < 1) samples = 1;
            // stop at the first numeric unless you prefer the last one to win
            break;
        }
    }

    // Camera defaults (reproducible in CI)
    Vec camPosDefault(0, -4, 1.0);
    Vec camTargetDefault(0, 0, 1);

    Vec camPos = camPosDefault;
    Vec camTarget = camTargetDefault;

    // Optional: CLI camera overrides (opt-in at compile time)
    #ifdef EXPERIMENTAL_CAMERA_CLI
    if (const char* v = getFlagValue(argc, argv, "--cam-pos")) {
        Vec parsed;
        if (parseVec3(v, parsed)) {
            camPos = parsed;
        } else {
            std::fprintf(stderr, "Invalid --cam-pos '%s' (expected x,y,z)\n", v);
        }
    }
    if (const char* v = getFlagValue(argc, argv, "--cam-target")) {
        Vec parsed;
        if (parseVec3(v, parsed)) {
            camTarget = parsed;
        } else {
            std::fprintf(stderr, "Invalid --cam-target '%s' (expected x,y,z)\n", v);
        }
    }
    #endif

    Camera camera = Camera(camPos, camTarget, 1280, 720);     // Create camera
    Scene scene = Scene();                                              // Create scene

    // Add objects to scene
    scene.add( dynamic_cast<Object*>(new Sphere(Vec(0,0,-1000), 1000, Material())) );
    scene.add( dynamic_cast<Object*>(new Sphere(Vec(-1004,0,0), 1000, Material(DIFF, Vec(0.85,0.4,0.4)))) );
    scene.add( dynamic_cast<Object*>(new Sphere(Vec(1004,0,0), 1000, Material(DIFF, Vec(0.4,0.4,0.85)))) );
    scene.add( dynamic_cast<Object*>(new Sphere(Vec(0,1006,0), 1000, Material())) );
    scene.add( dynamic_cast<Object*>(new Sphere(Vec(0,0,110), 100, Material(EMIT, Vec(1,1,1), Vec(2.2,2.2,2.2)))) );
    // scene.add( dynamic_cast<Object*>(new Mesh(Vec(), "../obj/dragon2.obj", Material(DIFF, Vec(0.9, 0.9, 0.9)))) );
    #ifndef ASSET_DIR
    #define ASSET_DIR "../obj"
    #endif

    // std::string path = std::string(ASSET_DIR) + "/dragon2.obj";
    // scene.add(new Mesh(
    //     Vec(0,0,0),
    //     path.c_str(),
    //     Material(DIFF, Vec(0.9,0.9,0.9))
    // ));

    // const bool useLucy = wantLucy(argc, argv);
    // std::string path = std::string(ASSET_DIR) + (useLucy ? "/sculpt.obj" : "/dragon2.obj");

    // Named mesh presets
    static const std::unordered_map<std::string, std::string> kPresets = {
        {"lucy",   "/lucy01.obj"},
        {"sculpt", "/sculpt.obj"},
        {"dragon", "/dragon01.obj"},
        {"dragon2","/dragon2.obj"}
        // Add more here if needed
    };

    // ---- Dynamic mesh selection ----
    std::string path;

    // (1) Custom file via --obj
    if (const char* objVal = getFlagValue(argc, argv, "--obj")) {
        path = std::string(objVal);
        // If it’s not absolute, prepend ASSET_DIR
        if (!(path.size() && (path[0] == '/' || path[0] == '.'))) {
            path = std::string(ASSET_DIR) + "/" + path;
        }
    } else {
        // (2) Preset via --mesh
        std::string key;
        if (const char* meshVal = getFlagValue(argc, argv, "--mesh")) {
            key = meshVal;
            for (auto& c : key) c = (char)std::tolower((unsigned char)c);
        }

        if (!key.empty()) {
            auto it = kPresets.find(key);
            if (it != kPresets.end()) {
                path = std::string(ASSET_DIR) + it->second;
            } else {
                fprintf(stderr, "Unknown mesh '%s'. Falling back to dragon2.obj\n", key.c_str());
            }
        }

        // (3) Fallback
        if (path.empty()) {
            path = std::string(ASSET_DIR) + "/dragon2.obj";
        }
    }

    scene.add(dynamic_cast<Object*>(new Mesh(
        Vec(0, 0, 0),                 // adjust offset
        path.c_str(),
        Material(SPEC, Vec(1, 1, 1))
    )));


    Renderer renderer = Renderer(&scene, &camera);  // Create renderer with our scene and camera
    renderer.render(samples);                       // Render image to pixel buffer
    renderer.save_image("render.png");              // Save image

    // Print duration information
    time(&stop);
    double diff = difftime(stop, start);
    int hrs = (int)diff/3600;
    int mins = ((int)diff/60)-(hrs*60);
    int secs = (int)diff-(hrs*3600)-(mins*60);
    printf("\rRendering (%i samples): Complete!\nTime Taken: %i hrs, %i mins, %i secs\n\n", samples, hrs, mins, secs);
    return 0;
}