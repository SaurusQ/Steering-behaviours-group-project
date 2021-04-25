#include "brain.hpp"
#include "boid.hpp"
//Initialize random number generator
std::mt19937 seed(time(0));
std::uniform_real_distribution<> dist(-5, 5);

//returns a vector to a random direction
glm::vec3 Brain::wander(Boid& boid) const {
	glm::vec3 newDir = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 randComp = glm::vec3(dist(seed), dist(seed), dist(seed));
	if(randComp != glm::vec3(0.0f, 0.0f, 0.0f)){
		randComp = glm::normalize(randComp) * max_avoid_force;
	} else {
		randComp += 0.3f;
	}
	newDir += glm::normalize(boid.getVelocity() * float(1.2) + randComp) * float(0.5);

  return newDir;
}

const glm::vec3 Brain::getColor(void) const {
	std::lock_guard<std::mutex> l(mutex_);
	return color_; 
}

const std::string Brain::getName(void) const {
	std::lock_guard<std::mutex> l(mutex_);
	return name_; 
}

//returns vector of boids that are near the given boid
std::vector<Boid> Brain::nearbyBoids(glm::vec3 boid, const std::vector<Boid>& boids, float scanDistance) const {
	std::vector<Boid> obstacleList;
	for (auto it = boids.begin(); it < boids.end(); it++) {
		float dist = glm::length(boid - it->getPos());
		if(dist < scanDistance){
			obstacleList.push_back(*it);
		}
	}
	return obstacleList;
}

//Returns a vector from the boid's position to the nearest wall, and the distance from the wall (negative if outside)
std::pair<glm::vec3, float> Brain::findNearestWall(glm::vec3& boid) const {
	glm::vec3 boidPos = boid;
	//Lazy if clause mess to find nearest wall.
	if (abs(boidPos.x) > abs(boidPos.y)) {
		if (abs(boidPos.x) > abs(boidPos.z)) {
			if (boidPos.x < 0) { boidPos.x = limitList_[0]; return std::make_pair(boidPos, boid.x - boidPos.x); }
			else { boidPos.x = limitList_[1]; return std::make_pair(boid - boidPos, boidPos.x - boid.x); }
		}
		else {
			if (boidPos.z < 0) { boidPos.z = limitList_[4]; return std::make_pair(boidPos, boid.z - boidPos.z); }
			else { boidPos.z = limitList_[5]; return std::make_pair(boid - boidPos, boidPos.z - boid.z); }
		}
	}
	else {
		if (abs(boidPos.y) > abs(boidPos.z)) {
			if (boidPos.y < 0) { boidPos.y = limitList_[2]; return std::make_pair(boidPos, boid.y - boidPos.y); }
			else { boidPos.y = limitList_[3]; return std::make_pair(boid - boidPos, boidPos.y - boid.y); }
		}
		else {
			if (boidPos.z < 0) { boidPos.z = limitList_[4]; return std::make_pair(boidPos, boid.z - boidPos.z); }
			else { boidPos.z = limitList_[5]; return std::make_pair(boid - boidPos, boidPos.z - boid.z); }
		}
	}
}

//returns false if boid is within world's bounds
bool Brain::checkIfOut(glm::vec3 position) const {
	if (abs(position.x) > limitList_[1] || (abs(position.y) > limitList_[3]) || (abs(position.z) > limitList_[5])) {
		return true;
	}
	else {
		return false;
	}
}

//Returns pair containing vector towards the nearest obstacle and boolean value indicating whether obstacle is a wall
std::pair<glm::vec3, bool> Brain::findObstacles(Boid& boid, const std::vector<Boid>& boids) const {
	//tell how far the boid can see to scan obstacles
	glm::vec3 sight;
	
	if(boid.getVelocity() != glm::vec3(0.0f, 0.0f, 0.0f)){
		sight = glm::normalize(boid.getVelocity())*float(1.2);
	} else {
		sight = glm::vec3(0.0f, 0.0f, 0.0f);
	}
	glm::vec3 checkLoc = boid.getPos() + sight;
	glm::vec3 boidLocation = boid.getPos();
	//vector containing boids that are near
	std::vector<Boid> potentialObstacles = nearbyBoids(boidLocation + sight * float(0.5), boids, 10);
	if(potentialObstacles.size() > 0){
		for(int i = 0; i < potentialObstacles.size(); i++){
			if(potentialObstacles[i].getPos() == boidLocation){ //removes obstacle from vector if it is the boid is itsself
				potentialObstacles.erase(potentialObstacles.begin() + i);
				
			}
		}
	}


	glm::vec3 endLoc = boidLocation + boid.getVelocity();
	float sightSize = 0.5;
	std::vector<glm::vec3> losList;
	for(float i = 0; i < 14; i+=1){
		std::pair<glm::vec3, float> wallnDist2 = findNearestWall(checkLoc);
		if (wallnDist2.second < 0) {
			float len = wallnDist2.second;
			if(len < 0){
				return std::make_pair(glm::normalize(wallnDist2.first) * max_avoid_force * float(5) / (len), true);	
			} else {
				return std::make_pair(glm::normalize(-wallnDist2.first) * max_avoid_force * float(5) / (len), true);	
			}
		}
		float x = checkLoc.x;
		float y = checkLoc.y;
		float z = checkLoc.z;
		for(int j = 0; j < potentialObstacles.size(); j++){
			if(glm::length(potentialObstacles[j].getVelocity()) <= 1){
				glm::vec3 potObs = potentialObstacles[j].getPos();
				//Checks if the position falls into the region of the currenct boid.
				if ((x > potObs.x - sightSize && x < potObs.x + sightSize) && (y > potObs.y - sightSize && y < potObs.y + sightSize) && (z > potObs.z - sightSize && z < potObs.z + sightSize)) {
					losList.push_back(potObs - checkLoc);
				}
			} else {
				glm::vec3 potObs = potentialObstacles[j].getPos() + potentialObstacles[j].getVelocity();
				if ((endLoc.x > potObs.x - sightSize && endLoc.x < potObs.x + sightSize) && (endLoc.y > potObs.y - sightSize && endLoc.y < potObs.y + sightSize) && (endLoc.z > potObs.z - sightSize && endLoc.z < potObs.z + sightSize)) {
					losList.push_back(potObs - endLoc);
				}
			}
		}
		checkLoc = boidLocation + sight * i;
	}
	
	if (losList.size() > 0) {
		float dist = glm::length(losList[0]);
		float dist2;
		glm::vec3 shortest = losList[0];
		for (auto i : losList) {
			dist2 = glm::length(i);
			if (dist2 < dist) {
				shortest = i;
				dist = dist2;
			}
		}
		if(shortest != glm::vec3(0.0f, 0.0f, 0.0f)){
			float len = glm::length(boid.getVelocity() - shortest);
			return std::make_pair(glm::normalize(boid.getVelocity() - shortest) * max_avoid_force * float(5) / (len), false);
		}
	}
	return std::make_pair(glm::vec3(0.0f, 0.0f, 0.0f), false);
}
