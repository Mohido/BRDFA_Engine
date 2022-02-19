#pragma once

#include <imgui/imgui.h>

#include "brdfa_engine.hpp"

#include <iostream>
#include <chrono>

#define WINDOW_HEIGHT 600

#define NO_CACHE_LOAD "--no-cache-load"
#define NCL "-ncl"

#define HOT_LOAD "--hot-load"
#define HL "-hl"

/// <summary>
/// Used to print the help menu when the -h or --help commands are passed.
/// </summary>
void printHelp() {
    printf("FLAGS:\n");
    printf("\t%s, %s\t\t\t Used to load the engine without the need to compile the BRDFs that are not cached. Only the BRDF source code will be loaded.\n",
        HOT_LOAD, HL);
    printf("\t%s, %s\t\t Used to disable cache loading. The engine will not load the data that was cached during the previous engine execution.\n",
        NO_CACHE_LOAD, NCL);

}


/// <summary>
/// The main function. 
/// </summary>
/// <param name="argc"></param>
/// <param name="argv"></param>
/// <returns></returns>
int main(int argc, char** argv) {
    /*Print the help command.*/
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printHelp();
            return 0;
        }
    }

    /*ENGIN Configuration*/
    brdfa::BRDFAEngineConfiguration conf;
    conf.height = WINDOW_HEIGHT;
    conf.width = static_cast<uint16_t>(WINDOW_HEIGHT / (9.0 / 16.0));
    conf.hot_load = true;
    conf.no_cache_load = true;
    for (int i = 0; i < argc; i++) {
        conf.hot_load = (strcmp(argv[i], "--hot-load") == 0) ? true : conf.hot_load;
        conf.no_cache_load = (strcmp(argv[i], "--no-cache-load") == 0) ? true : conf.no_cache_load;
    }

    /*Engin initialziation*/
    brdfa::BRDFA_Engine analyzerEngine(conf);
    analyzerEngine.init();
    
    /*The Engine update loop*/
    while (!analyzerEngine.isClosed()) 
        analyzerEngine.updateAndRender();

    /*The Engine termination*/
    if (!analyzerEngine.isClosed())
        analyzerEngine.close();    



    //auto startTime = std::chrono::high_resolution_clock::now();
    //uint32_t fps = 0;
    //while (!analyzerEngine.isClosed()) {
    //    analyzerEngine.updateAndRender();
    //    //auto currentTime = std::chrono::high_resolution_clock::now();
    //    //float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    //   //if (time >= 1.0) {
    //   //    startTime = currentTime;
    //   //    std::cout << fps << std::endl;
    //   //    fps = 0;
    //   //}
    //   //fps++;
    //}
    //
    //if (!analyzerEngine.isClosed()) {
    //    analyzerEngine.close();
    //}

    return 0;
}
