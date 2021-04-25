
#include "camera.hpp"

namespace Cam
{
    //Camera class
    Camera::Camera(Type type, glm::vec3 pos, float fov, float lookSens, float walkSens)
        : type_(type)
        , cameraPos_(pos)
        , fov_(fov)
        , lookSens_(lookSens)
        , walkSens_(walkSens)
    {}

    void Camera::processMouseScroll(double xoffset, double yoffset)
    {
        if(fov_ >= MIN_FOV && fov_ <= MAX_FOV)
            fov_ -= yoffset;
        if(fov_ <= MIN_FOV)
            fov_ = MIN_FOV;
        if(fov_ >= MAX_FOV)
            fov_ = MAX_FOV;
    }

    //FPS class
    FPS::FPS(glm::vec3 pos, glm::vec3 up,
                float yaw, float pitch)
        : Camera(Type::T_FPS, pos)
        , cameraUp_(up)
        , cameraFront_(glm::vec3(1.0f))
        , yaw_(yaw)
        , pitch_(pitch)//TODO limit values to MAX/MIN
    {}

    void FPS::processMouseMovement(double xoffset, double yoffset)
    {
        xoffset *= lookSens_;
        yoffset *= lookSens_;

        yaw_ += xoffset;
        pitch_ += yoffset;

        //Make sure that when pitch is out of bounds, screen doesn't get flipped
        if (pitch_ > 89.0f)
            pitch_ = 89.0f;
        if (pitch_ < -89.0f)
            pitch_ = -89.0f;

        glm::vec3 front;
        front.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
        front.y = sin(glm::radians(pitch_));
        front.z = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
        cameraFront_ = glm::normalize(front);
    }
    
    void FPS::processKeyboard(Key key)
    {
        float cameraSpeed = walkSens_ * deltaTime_;
        switch(key)
        {
            case Key::UP:
                cameraPos_ += cameraSpeed * cameraFront_;
                break;
            case Key::DOWN:
                cameraPos_ -= cameraSpeed * cameraFront_;
                break;
            case Key::LEFT:
                cameraPos_ -= glm::normalize(glm::cross(cameraFront_, cameraUp_)) * cameraSpeed;
                break;
            case Key::RIGHT:
                cameraPos_ += glm::normalize(glm::cross(cameraFront_, cameraUp_)) * cameraSpeed;
                break;
            case Key::DROP:
                cameraPos_ -= cameraSpeed * glm::vec3(0.0f, 1.0f, 0.0f);
                break;
            case Key::RISE:
                cameraPos_ += cameraSpeed * glm::vec3(0.0f, 1.0f, 0.0f);
                break;
        }
    }

    glm::mat4 FPS::getViewMatrix() const
    {
        return glm::lookAt(cameraPos_, cameraPos_ + cameraFront_, cameraUp_);
    }
}