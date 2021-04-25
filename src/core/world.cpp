#include "world.hpp"
#include <random>
#include <vector>
#include <algorithm>

extern float worldSize;

World::World(float sideLen, float maxSpeed) {
    worldLimits_ = {-sideLen/2, sideLen/2, -sideLen/2, sideLen/2, -sideLen/2, sideLen/2};
    maxSpeed_ = maxSpeed;
    boids_;
    boids_.reserve(7000);
};

void World::generateBoids(unsigned int num, const Brain* pBrain) {
    //Generally the same, but if we want non-cuboid bounds the implementation supports that
    std::random_device random;
    std::mt19937 seed(random());
    std::uniform_int_distribution<> xBounds(worldLimits_[0], worldLimits_[1]); 
    std::uniform_int_distribution<> yBounds(worldLimits_[2], worldLimits_[3]);
    std::uniform_int_distribution<> zBounds(worldLimits_[4], worldLimits_[5]);
    for (int i = 1; i <= num; i++) {
        glm::vec3 randPos = glm::vec3(xBounds(seed), yBounds(seed), zBounds(seed));
        Boid temp = Boid(randPos, 30.0f, glm::vec3(0.0f), this, pBrain);
        addBoid(temp);
        if(pBrain->getType() == 3){
            hunters_.push_back(randPos);
            targets_.push_back(temp);
        }
    }
}

bool World::addBoid(Boid boid) {
    std::lock_guard<std::mutex> l(mutex_);
    //Add checks if required
    boids_.push_back(boid);
    return true;
}

void World::removeBoids() {
    std::lock_guard<std::mutex> l(mutex_);
    boids_.clear();
}
void World::removeType(unsigned int type) {
    std::lock_guard<std::mutex> l(mutex_);
    boids_.erase(std::remove_if(boids_.begin(), boids_.end(), [type](Boid boid){return boid.getBrain()->getType() == type;}), boids_.end());
}

void World::addBrain(Brain* brain) {
    std::lock_guard<std::mutex> l(mutex_);
    brains_.push_back(brain);
}

void World::update(void) {
    for(auto it = boids_.begin(); it < boids_.end(); it++) {
        it->update(boids_);
    }
    this->removeDead();
}

void World::removeDead(void) {
    std::lock_guard<std::mutex> l(mutex_);
    //Remove read boids
    for(auto it = boids_.begin(); it < boids_.end(); it++) {
        if(!it->isAlive()) {
            auto deadId = it->getId();
            for(auto it2 = boids_.begin(); it2 < boids_.end(); it2++) {
                if(deadId = it2->getTarget()) {
                    it2->setTarget(it2->getId());
                }
            }
            it = boids_.erase(it);
        }
    }
}

void World::stop() {
    run_ = false;
}

bool World::isRunning() {
    return run_;
}

void World::setLimits(float sideLen) {
    std::lock_guard<std::mutex> l(mutex_);
    worldLimits_ = {-sideLen/2, sideLen/2, -sideLen/2, sideLen/2, -sideLen/2, sideLen/2};
    worldSize = sideLen;
    for(Brain* brain : brains_) {
        brain->setLimits(worldLimits_);
    }
}

void World::setMaxspeed(float speed) {
    //Call only from core / GUI threads
    std::lock_guard<std::mutex> l(guiMutex_);
    maxSpeed_ = speed;
}
const float& World::getMaxSpeed(void) const {
    //Call only from core / GUI threads
    std::lock_guard<std::mutex> l(guiMutex_);
    return maxSpeed_;
}
const std::vector<Boid>& World::getBoids(void) const { 
    mutex_.lock();
    return boids_;
}

void World::returnBoids(void) const {
    mutex_.unlock();
}

const std::vector<float>& World::getLimits(void) const {
    std::lock_guard<std::mutex> l(mutex_);
    return worldLimits_;
}

std::vector<Brain*> World::getBrains(void) {
    std::lock_guard<std::mutex> l(mutex_);
    return brains_;
}
