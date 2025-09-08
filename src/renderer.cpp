#include <vector>
#include <stdio.h>
#include <iostream>

#include "renderer.h"
#include "../lib/lodepng/lodepng.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <sstream>
#include <iomanip>

// Clamp double to min/max of 0/1
inline double clamp(double x){ return x<0 ? 0 : x>1 ? 1 : x; }
// Clamp to between 0-255
inline int toInt(double x){ return int(clamp(x)*255+.5); }

Renderer::Renderer(Scene *scene, Camera *camera) {
    m_scene = scene;
    m_camera = camera;
    m_pixel_buffer = new Vec[m_camera->get_width()*m_camera->get_height()];
}

void Renderer::render(int samples) {
    int width = m_camera->get_width();
    int height = m_camera->get_height();
    double samples_recp = 1./samples;

    typedef std::chrono::steady_clock Clock;
    auto t0 = Clock::now();
    std::atomic<int> rows_done{0};
    // Main Loop
    #pragma omp parallel for schedule(dynamic, 1)       // OpenMP
    for (int y=0; y<height; y++){
        // unsigned short Xi[3]={0,0,y*y*y};               // Stores seed for erand48
        // erand48 requires three unsigned shorts (0..65535). Keep values in range and vary by row:
        unsigned short Xi[3] = {
            static_cast<unsigned short>((y * 9781u + 1u) & 0xFFFF),
            static_cast<unsigned short>((y * 6271u + 2u) & 0xFFFF),
            static_cast<unsigned short>((y *  151u + 3u) & 0xFFFF)
        };

        // fprintf(stderr, "\rRendering (%i samples): %.2f%% ",      // Prints
                // samples, (double)y/height*100);                   // progress

        for (int x=0; x<width; x++){
            Vec col = Vec();

            for (int a=0; a<samples; a++){
                Ray ray = m_camera->get_ray(x, y, a>0, Xi);
                col = col + m_scene->trace_ray(ray,0,Xi);
            }

            m_pixel_buffer[(y)*width + x] = col * samples_recp;
        }
        // ---- end per-pixel work for this row ----

    // Atomically count finished rows
    int d = rows_done.fetch_add(1, std::memory_order_relaxed) + 1;

    // Throttle prints (every 3 rows) and serialize the output
    if (d == height || (d % 3 == 0)) {
        double frac = double(d) / double(height);
        auto   now  = Clock::now();
        double elapsed = std::chrono::duration<double>(now - t0).count();
        double eta = (frac > 1e-9) ? elapsed * (1.0 - frac) / frac : 0.0;

        // Use an UNNAMED critical, guarded so IntelliSense doesn’t choke
        #ifdef _OPENMP
        #pragma omp critical
        #endif
            {
            std::fprintf(stderr,
            "\rRendering (%d samples): %6.2f%%  [%d/%d rows]  elapsed %5.1fs  ETA %5.1fs ",
            samples, frac * 100.0, d, height, elapsed, eta);
            if (d == height) std::fputc('\n', stderr);
            std::fflush(stderr);
            }
        }
    }
}

void Renderer::save_image(const char *file_path) {
    int width = m_camera->get_width();
    int height = m_camera->get_height();

    std::vector<unsigned char> pixel_buffer;

    int pixel_count = width*height;

    for (int i=0; i<pixel_count; i++) {
        pixel_buffer.push_back(toInt(m_pixel_buffer[i].x));
        pixel_buffer.push_back(toInt(m_pixel_buffer[i].y));
        pixel_buffer.push_back(toInt(m_pixel_buffer[i].z));
        pixel_buffer.push_back(255);
    }

    //Encode the image
    unsigned error = lodepng::encode(file_path, pixel_buffer, width, height);
    //if there's an error, display it
    if(error) std::cout << "encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;

    pixel_buffer.clear();
}
