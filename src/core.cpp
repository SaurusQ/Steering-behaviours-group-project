
#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>

#include "core/world.hpp"
#include "core/boid.hpp"
#include "core/brainTypes.hpp"

#define CORE_FPS 60

extern unsigned int newFps;
extern bool fpsChanged;
extern std::mutex g_pause;

void mainCore(World& gameWorld) {
    std::cout << "Main core running" << std::endl;

    Brain* wanderer = new PassiveBrain(gameWorld.getLimits());
    Brain* flocker = new FlockBrain(gameWorld.getLimits());
    Brain* hunter = new HunterBrain(gameWorld.getLimits());
    gameWorld.addBrain(wanderer);
    gameWorld.addBrain(flocker);
    gameWorld.addBrain(hunter);

    gameWorld.generateBoids(100, flocker);
	gameWorld.generateBoids(5, hunter);


    std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point curTime;
    std::chrono::nanoseconds deltaCore = std::chrono::nanoseconds(1000000000/CORE_FPS);
    uint64_t tick = 0;
    std::cout << "Starting core loop" << std::endl;

    while(gameWorld.isRunning())
    {
        if(fpsChanged) {
            deltaCore = std::chrono::nanoseconds(1000000000/newFps);
            fpsChanged = false;
            std::cout << "Fps was changed" << std::endl;
        }
        g_pause.lock();
        g_pause.unlock();
        curTime = std::chrono::high_resolution_clock::now();
        if(deltaCore <= std::chrono::duration_cast<std::chrono::nanoseconds>(curTime - startTime)) {

            gameWorld.update();

            //Update the start time of the next tick
            startTime += deltaCore;
            tick++;
    
            //Check that the ticks are not late
            curTime = std::chrono::high_resolution_clock::now();
            if(deltaCore <= std::chrono::duration_cast<std::chrono::nanoseconds>(curTime - startTime))
            {
                startTime = curTime - deltaCore;
            }
        }
    }
    //Clenup
    delete wanderer;
    delete flocker;
    delete hunter;
}