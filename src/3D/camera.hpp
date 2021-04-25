
#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Cam
{
    #define MAX_FOV 60.0f
    #define MIN_FOV 1.0f
    #define DEF_FOV 45.0f
    #define DEF_LOOK_SENS 0.1f
    #define DEF_WALK_SENS 5.0f

    enum Type {
        T_FPS,
        T_CHASE,
        T_FOLLOW
    };

    enum Key {
        UP,
        DOWN,
        LEFT,
        RIGHT,
        DROP,
        RISE
    };

    class Camera
    {
        public:
            Camera(Type type, glm::vec3 pos, float fov = DEF_FOV,
                float lookSens = DEF_LOOK_SENS,
                float walkSens = DEF_WALK_SENS);
            //virtual ~Camera();
            //Input processing
            virtual void processMouseMovement(double xoffset, double yoffset) = 0;
            virtual void processKeyboard(Key key) = 0;
            virtual void processMouseScroll(double xoffset, double yoffset);
            //Getters
            virtual glm::mat4 getViewMatrix() const = 0;
            glm::vec3 getPos() const { return cameraPos_; }
            float getFov() const { return fov_; }
            float getDeltaTime() const { return deltaTime_; }
            float getLookSens() const { return lookSens_; }
            float getWalkSens() const { return walkSens_; }
            Type getType() const { return type_; }
            //Setters
            void setDeltaTime(float deltaTime) { deltaTime_ = deltaTime; }
            void setLookSens(float lookSens) { lookSens_ = lookSens; }
            void setWalkSens(float walkSens) { walkSens_ = walkSens; }
        protected:
            float deltaTime_ = 0;       //Time between frames
            float fov_;
            glm::vec3 cameraPos_;
            float lookSens_;
            float walkSens_;
            Type type_;
    };

    class FPS : public Camera
    {
        public:
            FPS(glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
                float yaw = -90.0f, float pitch = 0.0f);
            virtual void processMouseMovement(double xoffset, double yoffset);
            virtual void processKeyboard(Key key);
            virtual glm::mat4 getViewMatrix() const;
        private:
            glm::vec3 cameraFront_;
            glm::vec3 cameraUp_;
            float yaw_;
            float pitch_;
    };
}


#endif
