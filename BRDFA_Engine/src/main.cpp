#pragma once
#include "brdfa_engine.hpp"

#include <iostream>
#include <chrono>


int main() {
    {
        uint16_t height = 600;

        // Configuration struct of the engine...
        brdfa::BRDFAEngineConfiguration conf;
        conf.height = height;
        conf.width = static_cast<uint16_t>(height / (9.0 / 16.0));

        // Starting the engine.
        brdfa::BRDFA_Engine analyzerEngine(conf);
        bool successStart = analyzerEngine.init();

        auto startTime = std::chrono::high_resolution_clock::now();
        uint32_t fps = 0;
        while (successStart && analyzerEngine.updateAndRender()) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
            if (time >= 1.0) {
                startTime = currentTime;
                std::cout << fps << std::endl;
                fps = 0;
            }
            fps++;
        }
    }
    return 0;
}
