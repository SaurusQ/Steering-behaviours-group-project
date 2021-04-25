#include "world.hpp"
#include "brainTypes.hpp"
#include <algorithm>

unsigned int newFps;
bool fpsChanged;

bool check_number(const std::string &s) {
  return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
}

void processInput(std::string command, World &world) {
    const Brain* wanderer = new PassiveBrain(world.getLimits());
    std::vector<std::string> tokens;
    unsigned int pos = 0;
    std::string token;
    while ((pos = command.find(" ")) < 100 ) {
        token = command.substr(0, pos);
        tokens.push_back(token);
        command.erase(0, pos + 1);
    }
    tokens.push_back(command);
    std::string action = tokens[0];
    if (action == "add") {
        if(tokens.size() == 3) {
            std::string amount = tokens[1];
            int n;
            if(check_number(amount)) {
                n = std::stoi(amount);
            } else {std::cout << "Second argument must be a number" << std::endl; return;}
            std::string brainName = tokens[2];
            bool found = false;
            for (const Brain* brain : world.getBrains()) {
                if (brain->getName() == brainName) {
                    found = true;
                    world.generateBoids(n, brain);
                }
            }
            if (found == true) {
                std::cout << "Added " << n << " new boids." << std::endl;
            } else { std::cout << "Brain " << brainName << " not found." << std::endl; return; }
        } else { std::cout << "Invalid number of arguments!" << std::endl; return; }

    } else if(action == "clear") {
        if(tokens.size() == 2) {
            if(tokens[1] == "hunter") {
                world.removeType(3);
            } else if(tokens[1] == "flocker") {
                world.removeType(2);
            } else if(tokens[1] == "wanderer") {
                world.removeType(1);
            } else {std::cout << "No boid of type " << tokens[1] << " found." << std::endl;}
        } else {
            world.removeBoids();
        }
    } else if(action == "bounds") {
        if(tokens.size() == 2) {
            std::string amount = tokens[1];
            int n;
            if(check_number(amount)) {
                n = std::stoi(amount);
            } else { std::cout << "Second argument must be a number" << std::endl; return;}
            world.setLimits(n);
        } else { std::cout << "Invalid number of arguments!" << std::endl; return; }
    } else if(action == "setfps") {
        if(tokens.size() == 2) {
            std::string amount = tokens[1];
            int n;
            if(check_number(amount)) {
                n = std::stoi(amount);
                newFps = n;
                fpsChanged = true;
            } else { std::cout << "Second argument must be a number" << std::endl; return;}
        } else { std::cout << "Invalid number of arguments!" << std::endl; return; }
    } else if(action == "maxspeed") {
        if(tokens.size() == 2) {
            std::string amount = tokens[1];
            float n;
            if(check_number(amount)) {
                n = std::stoi(amount);
                world.setMaxspeed(n/100.0f);
            } else { std::cout << "Second argument must be a number" << std::endl; return;}
        } else { std::cout << "Invalid number of arguments!" << std::endl; return; }
    } else if(action == "maxforce") {
        if(tokens.size() == 2) {
            std::string amount = tokens[1];
            float n;
            if(check_number(amount)) {
                n = std::stoi(amount);
                for(Boid boid : world.getBoids()) {
                    boid.setMaxForce(n/100.0f);
                }
                world.returnBoids();
            } else { std::cout << "Second argument must be a number" << std::endl; return;}
        } else { std::cout << "Invalid number of arguments!" << std::endl; return; }
    }
}
