#include <iostream>
#include "core/GUI.hpp"

std::string command = "";
void consoleCore(World& gameWorld) {
    std::cout << "Console thread running" << std::endl;
    while(gameWorld.isRunning()) {
        std::cout << "Enter a command:";
        getline(std::cin, command);
        processInput(command, gameWorld);
    }
}