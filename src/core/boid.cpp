#include "boid.hpp"

unsigned int Boid::s_nextId_ = 0;

Boid::Boid(glm::vec3 pos, float mass, glm::vec3 velocity = glm::vec3(0, 1.0f, 0), World* world = nullptr, const Brain* pBrain = nullptr) :
	pos_(pos), mass_(mass), velocity_(velocity),world_(world) , pBrain_(pBrain), id_(s_nextId_) {
	targetId_ = id_;
	s_nextId_++;
}

//Steers boid towards given direction and updates its position and velocity
void Boid::seek(glm::vec3 direction) {
	glm::vec3 current_velocity = this->getVelocity();
	glm::vec3 desired_velocity = direction;
	glm::vec3 current_position = this->getPos();
	
	glm::vec3 new_pos = current_position + current_velocity;
	glm::vec3 diff = new_pos - current_position;
	
	this->setPos(new_pos);

	glm::vec3 steering = desired_velocity - current_velocity;

	//Ensures that the magnitude of the steering force is smaller than or equal to the max force
	float maxForce = this->getMaxForce();
	if (glm::length(steering) > maxForce) {
		steering = glm::normalize(steering) * maxForce;
	}
	//different speeds for boids with different masses
	steering = steering / this->getMass();

	glm::vec3 new_velocity = current_velocity + steering;
	if (glm::length(new_velocity) > this->boidMaxSpeed()) {
		new_velocity = glm::normalize(new_velocity) * this->boidMaxSpeed();
	}

	this->setVelocity(new_velocity);
}

void Boid::update(const std::vector<Boid>& boidList) {
	pBrain_->behave(*this, boidList);
}

const glm::vec3 Boid::getColor(void) const {
	std::lock_guard<std::mutex> l(mutex_);
	return pBrain_->getColor();
}
const glm::vec3 Boid::getPos(void) const {
	std::lock_guard<std::mutex> l(mutex_);
	return pos_;
}
bool Boid::setPos(glm::vec3 pos) {

	std::lock_guard<std::mutex> l(mutex_);
	pos_ = pos;
	return true;
}

const glm::vec3 Boid::getVelocity(void) const {
	std::lock_guard<std::mutex> l(mutex_);
	return velocity_;
}
void Boid::setVelocity(glm::vec3 velocity) {
	std::lock_guard<std::mutex> l(mutex_);
	velocity_ = velocity;
}
const glm::mat3 Boid::getOrientation(void) const {
	std::lock_guard<std::mutex> l(mutex_);
	return orientation_;
}

unsigned int Boid::getId(void) const {
	std::lock_guard<std::mutex> l(mutex_);
	return id_;
}

const   float Boid::boidMaxSpeed(void) const {
	std::lock_guard<std::mutex> l(mutex_);
	return world_->getMaxSpeed();
}
bool Boid::isAlive(void) {
	std::lock_guard<std::mutex> l(mutex_);
	return alive_;
}
void Boid::kill(void) {
	std::lock_guard<std::mutex> l(mutex_);
	alive_ = false;
}
float Boid::getMaxForce(void) const {
	std::lock_guard<std::mutex> l(mutex_);
	return max_force;
}
void Boid::setMaxForce(float force) {
	std::lock_guard<std::mutex> l(mutex_);
	max_force = force;
}
const Brain* Boid::getBrain(void) const { 
	std::lock_guard<std::mutex> l(mutex_);
	return pBrain_;
}

//Return position vector of boid with given ID, or own position if not found.
const Boid Boid::boidOf(unsigned int id, const std::vector<Boid> boids) {
	for(auto it = boids.begin(); it < boids.end(); it++) {
		if(it->getId() == id) {
			return *it;
		}
	}
	return *this;
}

void Boid::setTarget(unsigned int targ_id) { 
	std::lock_guard<std::mutex> l(mutex_);
	targetId_ = targ_id;
}

unsigned int Boid::getTarget(void) { 
	std::lock_guard<std::mutex> l(mutex_);
	return targetId_; 
}

float Boid::getMass(void) {
	std::lock_guard<std::mutex> l(mutex_);
	return mass_;
}

//Copy assigment
Boid& Boid::operator= (const Boid& other) {
	//std::cout << "= Boid&" << std::endl;
	std::lock(mutex_, other.mutex_);
	std::unique_lock<std::mutex> lock_this(this->mutex_, std::adopt_lock);
	std::unique_lock<std::mutex> lock_other(other.mutex_, std::adopt_lock);
	mass_ = other.mass_;
	pos_ = other.pos_;
	velocity_ = other.velocity_;
	orientation_ = other.orientation_;
	max_force = other.max_force;
	world_ = other.world_;
	pBrain_ = other.pBrain_;
	alive_ = other.alive_;
	id_ = other.id_;
	targetId_ = other.targetId_;
	return *this;
}

//Copy initialization
Boid::Boid(const Boid& other) {
	std::lock_guard<std::mutex> lock_other(other.mutex_);
	//std::cout << "Boid&" << std::endl;
	mass_ = other.mass_;
	pos_ = other.pos_;
	velocity_ = other.velocity_;
	orientation_ = other.orientation_;
	max_force = other.max_force;
	world_ = other.world_;
	pBrain_ = other.pBrain_;
	alive_ = other.alive_;
	id_ = other.id_;
	targetId_ = other.targetId_;
}

Boid& Boid::operator=(Boid&& other) {
	if (this != &other) {
		std::unique_lock<std::mutex> self_lock(mutex_, std::defer_lock);
		std::unique_lock<std::mutex> other_lock(other.mutex_, std::defer_lock);
		std::lock(self_lock, other_lock);
		
		mass_ = std::move(other.mass_);
		pos_ = std::move(other.pos_);
		velocity_ = std::move(other.velocity_);
		orientation_ = std::move(other.orientation_);
		max_force = std::move(other.max_force);
		world_ = std::move(other.world_);
		pBrain_ = std::move(other.pBrain_);
		alive_ = std::move(other.alive_);
		id_ = std::move(other.id_);
		targetId_ = std::move(other.targetId_);

	}
	return *this;
}

Boid::Boid(Boid&& other) {
	std::lock_guard<std::mutex> lock(other.mutex_);
	
	mass_ = std::move(other.mass_);
	pos_ = std::move(other.pos_);
	velocity_ = std::move(other.velocity_);
	orientation_ = std::move(other.orientation_);
	max_force = std::move(other.max_force);
	world_ = std::move(other.world_);
	pBrain_ = std::move(other.pBrain_);
	alive_ = std::move(other.alive_);
	id_ = std::move(other.id_);
	targetId_ = std::move(other.targetId_);

	mass_ = 0;
	pos_ = glm::vec3(0.0f);
	velocity_ = glm::vec3(0.0f);
	orientation_ = glm::mat3(0.0f);
	max_force = 0;
	world_ = 0;
	pBrain_ = nullptr;
	alive_ = 0;
	id_ = 0;
	targetId_ = 0;
}
