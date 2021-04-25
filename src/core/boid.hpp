#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <iostream>
#include "brain.hpp"
#include "world.hpp"

#include <time.h>
#include <random>
#include <mutex>

class World;

class Boid {
public:
	//Thread safe class
	// Call brain, and adjust velocity/pos/base
	Boid(glm::vec3 pos, float mass, glm::vec3 velocity, World* world, const Brain* pBrain);
	~Boid() {};

	//Update calls brain with *this
	void seek(glm::vec3 direction);
	void update(const std::vector<Boid>& boidListOld);

	//Setters and getters
	const glm::vec3	    	getColor(void) const;
	const glm::vec3  		getPos(void) const;
	bool			        setPos(glm::vec3 pos);
	const glm::vec3 	  	getVelocity(void) const;
	void					setVelocity(glm::vec3 velocity);
	const glm::mat3     	getOrientation(void) const;
	unsigned int 			getId(void) const;
	const float				boidMaxSpeed(void) const;
	bool          			isAlive(void);
	void          			kill(void);
	float					getMaxForce(void) const;
	void					setMaxForce(float force);
	const Brain*			getBrain(void) const;
	const Boid				boidOf(unsigned int id, const std::vector<Boid> boids);
	void 					setTarget(unsigned int targ_id);
	unsigned int			getTarget(void);
	float 					getMass(void);
	
	Boid&	operator=(const Boid& b);
	bool 	operator==(const Boid& other) { if(this->getId() == other.getId()) return true; else return false; }
	bool	operator!=(const Boid& other) {return !(*this==other);}

	//Print the position of given boid to terminal (For debugging purposes)
	void printPos() { std::cout << "X:" << pos_.x << ", Y: " << pos_.y << ", Z: " << pos_.z << std::endl; };
	
	Boid(const Boid& another);
	Boid& operator=(Boid&& other);
	Boid(Boid&& other);

private:
	static unsigned int s_nextId_;

	float mass_;
	glm::vec3 pos_;
	glm::vec3 velocity_;
	glm::mat3 orientation_;
	float max_force = 3;
	bool alive_ = true;
	World* world_;
	Brain const* pBrain_;
	unsigned int id_;
	unsigned int targetId_;
	mutable std::mutex mutex_;
};
