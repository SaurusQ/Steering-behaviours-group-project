#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <mutex>

class Boid; //Forward declaration

class Brain {
public:
	Brain(const std::vector<float>& limitList, glm::vec3 color, std::string name) :
	color_(color), limitList_(limitList), name_(name) {}
	~Brain() {};

	const glm::vec3		getColor() const;
	virtual int			getType(void) const = 0;
	const	std::string	getName() const;
	virtual void        behave(Boid& boid, const std::vector<Boid>& bListOld) const = 0;
	
	void 				setLimits(std::vector<float> limits) {limitList_ = limits;}
protected:
	glm::vec3					wander(Boid& boid) const;
	bool 						checkIfOut(glm::vec3 checkPos) const;
	//Returns nearest wall, with it's single static coordinate.
	std::pair<glm::vec3, float> findNearestWall(glm::vec3& boid) const;
	std::pair<glm::vec3, bool>	findObstacles(Boid& boid, const std::vector<Boid>& boids) const;
	std::vector<Boid> 			nearbyBoids(glm::vec3 boid, const std::vector<Boid>& boids, float scanDistance) const;
	
	float max_avoid_force = 15;
	const glm::vec3 color_;
	std::string name_;
	std::vector<float> limitList_;
	mutable std::mutex mutex_;
};