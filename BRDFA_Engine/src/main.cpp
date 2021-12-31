#pragma once

#include <imgui/imgui.h>

#include "brdfa_engine.hpp"

#include <iostream>
#include <chrono>


#define WINDOW_HEIGHT 600

int main() {
    // Configuration struct of the engine...
    brdfa::BRDFAEngineConfiguration conf;
    conf.height = WINDOW_HEIGHT;
    conf.width = static_cast<uint16_t>(WINDOW_HEIGHT / (9.0 / 16.0));

    // Starting the engine.
    brdfa::BRDFA_Engine analyzerEngine(conf);
    analyzerEngine.init();

    auto startTime = std::chrono::high_resolution_clock::now();
    uint32_t fps = 0;
    while (!analyzerEngine.isClosed()) {
        analyzerEngine.updateAndRender();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        if (time >= 1.0) {
            startTime = currentTime;
            std::cout << fps << std::endl;
            fps = 0;
        }
        fps++;
    }

    if (!analyzerEngine.isClosed()) {
        analyzerEngine.close();
    }

    return 0;
}
