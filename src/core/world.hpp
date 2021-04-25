#pragma once

#include "boid.hpp"
#include "brain.hpp"
#include <vector>
#include <mutex>
#include <map>

class Boid;

class World{
public:
    //Initialize bounds with sideLen
    World(float sideLen, float maxSpeed = 5);

    void generateBoids(unsigned int num, const Brain* pBrain);  //Add boids to the world with specified brain
    bool addBoid(Boid boid);                                    //Add boid to world
    void removeBoids(void);                                     //Remove all boids from world
    void removeType(unsigned int type); 
    void addBrain(Brain* brain);                                
    void update(void);                                          //Calculate movement for all boids
    void removeDead(void);                                      //Remove eaten boids

    void stop();                                                //Tell other thread to stop running
    bool isRunning();

    //Getters and setters
    void setLimits(float sideLen);
    void setMaxspeed(float speed);
    const float& getMaxSpeed(void) const;
    const std::vector<Boid>& getBoids(void) const;
    void returnBoids(void) const;
    const std::vector<float>&getLimits(void) const;
    std::vector<Brain*>getBrains(void);

private:
    std::vector<Boid> boids_;           //Store all boids
    std::vector<float> worldLimits_;    //Max coordinates allowed inside the world
    std::vector<Brain*> brains_;        //Storage for brains to boids
    std::vector<glm::vec3> hunters_;
    std::vector<Boid> targets_;
    float maxSpeed_;                    //Boids max speed
    bool run_ = true;                   //Run world while true
    mutable std::mutex mutex_;          //Mutex for changing the boid vectro
    mutable std::mutex guiMutex_;       //Mutex to synch gui communication
};