#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "brain.hpp"


class HunterBrain : public Brain {
public:
    HunterBrain(const std::vector<float>& limitList) : Brain(limitList, glm::vec3(1.0f, 0.5f, 0.5f), "hunter") {}
    ~HunterBrain() {}
	int getType(void) const { 
		std::lock_guard<std::mutex> l(mutex_);
		return 3; 
	}

	void behave(Boid& boid, const std::vector<Boid>& bListOld) const {
		glm::vec3 position = boid.getPos();

		//get id of the target, if id is the hunter's id, hunter has no target
		unsigned int targetId = boid.getTarget();
		
		//Check that boid is within the world's bounds
		if(!checkIfOut(position)){

			
			if(targetId == boid.getId()){	//hunter has no target
				
				targetId = findPrey(boid, bListOld); //search for target
				boid.setTarget(targetId);	//if no targets around, targetId remains the same as hunter's id
				if(targetId == boid.getId()){	//hunter has no target
					std::pair<glm::vec3, bool> obstacle = findObstacles(boid, bListOld);
					glm::vec3 direction = glm::vec3(0.0f, 0.0f, 0.0f);
					glm::vec3 wander_dir = wander(boid);
					if(obstacle.second){	//nearest obstacle is a wall
						direction += glm::normalize(obstacle.first + wander_dir) * boid.getMaxForce(); //hunter wanders while avoiding the wall
					} else {
						direction += glm::normalize(obstacle.first + wander_dir) * boid.getMaxForce(); //hunter wanders, avoiding other obstacles
					}
					boid.seek(direction); 
				} 
				else {
					pursuit(boid, boid.boidOf(targetId, bListOld), bListOld); //hunter has target and pursues it
				}
			} 
			else {
				pursuit(boid, boid.boidOf(targetId, bListOld), bListOld); //hunter has target and pursues it
			}
		} else {
			boid.seek(-findNearestWall(position).first); //boid is not within world's bounds, go back inside
		}
	}
private:

	//findPrey tries to find a new target for the hunter, returns target's Id.
	unsigned int findPrey(Boid& boid, const std::vector<Boid>& boids) const {

		//tells how far away hunter can see
		glm::vec3 sight;

		if (boid.getVelocity() != glm::vec3(0.0f, 0.0f, 0.0f)) {
			sight = glm::normalize(boid.getVelocity())*float(2);
		}
		else {
			sight = glm::vec3(0.0f, 0.0f, 0.0f);
		}
		glm::vec3 boidLocation = boid.getPos();

		//vector of boids that are near the hunter
		std::vector<Boid> potentialObstacles = nearbyBoids(boidLocation + sight * float(0.5), boids, 15);

		if (potentialObstacles.size() > 0) {
			for (auto it = potentialObstacles.begin(); it < potentialObstacles.end(); it++) {
				if (it->getId() == boid.getId() || !(it->isAlive()) || it->getBrain()->getType() == 3) {
					it = potentialObstacles.erase(it);	//deletes boid from vector if the boid is the hunter itself, the boid is already dead
														//or the boid is another hunter
				}
			}
		}
		if (potentialObstacles.size() == 0) { //if there are no possible targets nearby, the function returns the hunter's own id
			return boid.getId();
		}


		glm::vec3 endLoc = boidLocation + boid.getVelocity();

		//the coordinates of the end point of the hunter's sight vector
		glm::vec3 checkLoc = boid.getPos() + sight;
		//vector of boids that are in hunter's line of sight
		std::vector<Boid> losList;
		float sightSize = 1.5;
		for (float i = 0; i < 10; i += 1.2) {
			float x = checkLoc.x;
			float y = checkLoc.y;
			float z = checkLoc.z;
			for (auto it = potentialObstacles.begin(); it < potentialObstacles.end(); it++) {
				if (it->getBrain()->getType() != 3) {
					if (glm::length(it->getVelocity()) <= 1) {
						glm::vec3 potObs = it->getPos();
						//Checks if the position falls into the region of the currenct boid.
						if ((x > potObs.x - sightSize && x < potObs.x + sightSize) && (y > potObs.y - sightSize && y < potObs.y + sightSize) && (z > potObs.z - sightSize && z < potObs.z + sightSize)) {
							losList.push_back(*it);
						}
					}
					else {
						glm::vec3 potObs = it->getPos() + it->getVelocity();
						if ((endLoc.x > potObs.x - sightSize && endLoc.x < potObs.x + sightSize) && (endLoc.y > potObs.y - sightSize && endLoc.y < potObs.y + sightSize) && (endLoc.z > potObs.z - sightSize && endLoc.z < potObs.z + sightSize)) {
							losList.push_back(*it);
						}
					}
				}
				else {
					return boid.getId();
				}
			}
			checkLoc = boidLocation + sight * i;
		}
		if (losList.size() > 0) {
			float dist = glm::length(losList[0].getPos() - boidLocation);
			float dist2;
			Boid closest = losList[0];
			for (auto it : losList) {
				dist2 = glm::length(it.getPos() - boidLocation);
				if (dist2 < dist) {
					closest = it;
					dist = dist2;
				}
			}
			return closest.getId();
		}
		return boid.getId();
	}

	void pursuit(Boid& boid, const Boid& target, const std::vector<Boid>& bList) const {
		// Targets position in the next iteration
		float multiplier = 1;
		float dist = glm::length(target.getPos() - boid.getPos());
		if (dist < 10) multiplier = dist / 10;
		glm::vec3 future_pos = target.getPos() + target.getVelocity()*multiplier;
		glm::vec3 wander_dir = wander(boid);
		std::pair<glm::vec3, bool> obstacle = findObstacles(boid, bList);
		glm::vec3 adjusted_wander = glm::normalize(obstacle.first + wander_dir); //wander while avoiding obstacle
		glm::vec3 direction;
		if (obstacle.second) {	//nearest obstacle is a wall
			direction = glm::normalize(future_pos - boid.getPos() + float(3)*adjusted_wander) * boid.getMaxForce();
		}
		else {
			direction = glm::normalize(future_pos - boid.getPos() + float(0.5)*wander_dir) * boid.getMaxForce();
		}

		boid.seek(direction);
	}
};

class FlockBrain : public Brain {
public:
    FlockBrain(const std::vector<float>& limitList) : Brain(limitList, glm::vec3(0.5f, 1.0f, 0.5f), "flocker") {}
    ~FlockBrain() {}
	int getType(void) const {
		std::lock_guard<std::mutex> l(mutex_);
		return 2; 
	}

	void behave(Boid& boid, const std::vector<Boid>& bListOld) const {
		glm::vec3 position = boid.getPos();
		if(!checkIfOut(position)){ //flocks when iside the world
			flocking_wander(boid, bListOld);
		} else {
			boid.seek(-findNearestWall(position).first); //seeks back inside if outside the world's bounds
		}
	}
private:
	void flocking_wander(Boid& boid, const std::vector<Boid>& bListOld) const {
		std::pair<glm::vec3, bool> obstacle = findObstacles(boid, bListOld);
		glm::vec3 wander_dir = wander(boid);
		glm::vec3 adjusted_wander;
		
		adjusted_wander = glm::normalize(obstacle.first + wander_dir);
		
		//vector of all nearby boids
		std::vector<Boid> neighbours = nearbyBoids(boid.getPos(), bListOld, 8);
		glm::vec3 direction;
		if(neighbours.size() > 0){
			//vector of nearby boids that are also flockers
			std::vector<Boid> neighbours2;
			for(int i = 0; i < neighbours.size(); i++){
				if(neighbours[i].getBrain()->getType() == 2){
					neighbours2.push_back(neighbours[i]);
				} else if(neighbours[i].getBrain()->getType() == 3){ //nearby boid is a hunter
					float dist = glm::length(neighbours[i].getPos() - boid.getPos());
					if (dist <= float(0.5)) { //if hunter is close enough, flocker is caught and killed
						boid.kill();
					}
				}
			}
			
			glm::vec3 align = alignment(boid, neighbours2);
			glm::vec3 cohese = cohesion(boid, neighbours2);
			glm::vec3 separate = separation(boid, neighbours2);
			
			if(obstacle.second){ //nearest obstacle is a wall
				direction = float(1)*align + float(2)*cohese + float(2)*separate + float(2.7)*adjusted_wander;
			} else {
				direction = float(1)*align + float(2.5)*cohese + float(2)*separate + float(2.7)*adjusted_wander;
			}
		} else { //if no boids nearby, boid wanders
			direction = adjusted_wander;
		}
		
		direction = glm::normalize(direction);

		boid.seek(direction);
		//TODO std::cout << "w" << std::endl;
	}
	// Returns a vector towards a direction that is aligned with nearby boids
	glm::vec3 alignment(Boid& boid, std::vector<Boid>& boids) const {

		glm::vec3 align_vec = glm::vec3(0.0f, 0.0f, 0.0f);

		if (boids.size() == 0) {
			return align_vec;
		}

		for (auto i = 0; i < boids.size(); i++) {
			align_vec += boids[i].getVelocity();
		}
		if(align_vec !=  glm::vec3(0.0f, 0.0f, 0.0f)){
			align_vec = glm::normalize(align_vec);
		}

		return align_vec;
	}

	// Return a vector towards the mass center of nearby boids
	glm::vec3 cohesion(Boid& boid, std::vector<Boid>& boids) const {
		glm::vec3 boidLoc = boid.getPos();
		glm::vec3 cohesion_vec = glm::vec3(0.0f, 0.0f, 0.0f);
		float size = boids.size();
		if (size == 0) {
			return cohesion_vec;
		}

		for (float i = 0; i < size; i++) {
			cohesion_vec += (boids[i].getPos() - boidLoc);
		}
		if(cohesion_vec  !=  glm::vec3(0.0f, 0.0f, 0.0f)){
			cohesion_vec = glm::normalize(cohesion_vec);
		}

		return cohesion_vec;
	}

	// Returns a vector facing away from nearby boids
	glm::vec3 separation(Boid& boid, std::vector<Boid>& boids) const {
		glm::vec3 separation_vec = glm::vec3(0.0f, 0.0f, 0.0f);
		float size = boids.size();
		if (size == 0) {
			return separation_vec;
		}

		for (float i = 0; i < size; i++) {
			separation_vec -= (boids[i].getPos() - boid.getPos());
		}
		if(separation_vec != glm::vec3(0.0f, 0.0f, 0.0f)){
			separation_vec = glm::normalize(separation_vec);
		}

		return separation_vec;
	}
};

class PassiveBrain : public Brain {
public:
    PassiveBrain(const std::vector<float>& limitList) : Brain(limitList, glm::vec3(0.5f, 0.5f, 1.0f), "wanderer") {}
	~PassiveBrain() {}

	int getType(void) const { 
		std::lock_guard<std::mutex> l(mutex_);
		return 1;
	}

    void behave(Boid& boid, const std::vector<Boid>& bListOld) const {
		glm::vec3 position = boid.getPos();
		
		if(!checkIfOut(position)){ //boid is inside world's bounds
			//vector of all nearby boids
			std::vector<Boid> neighbours = nearbyBoids(boid.getPos(), bListOld, 10);
			for (int i = 0; i < neighbours.size(); i++) {
				if (neighbours[i].getBrain()->getType() == 3) { //nearby boid is a hunter
					float dist = glm::length(neighbours[i].getPos() - boid.getPos());
					if (dist <= float(0.5)) { //if hunter is close enough, wanderer is caught and killed
						boid.kill();
					}
				}

			}
			if (boid.isAlive()) {
				std::pair<glm::vec3, bool> obstacle = findObstacles(boid, bListOld);
				glm::vec3 direction = glm::vec3(0.0f, 0.0f, 0.0f);
				glm::vec3 wander_dir = wander(boid);
				if (obstacle.second) { //obstacle is a wall
					direction += glm::normalize(obstacle.first + wander_dir) * boid.getMaxForce();
				}
				else {
					direction += glm::normalize(obstacle.first + wander_dir) * boid.getMaxForce();
				}
				boid.seek(direction);
			}
			
		} else { //boid is outside world's bounds, go back inside
			boid.seek(-findNearestWall(position).first);
		}
	}
};