#ifndef GL_CAMERA_160412
#define GL_CAMERA_160412

#include <limits>

#define GLM_FORCE_RADIANS 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "gl_window.h"

class Camera : public GLCamera {
public:
    Camera() : from(1, 1.5, 3), to(0, 0, 0), up(0, 1, 0), fov(45) {}
    void reshapeScreen(int width, int height);
    void move(float dx, float dy, float dz);
    void rotateOrbit(float dtheta, float dphi);
    glm::mat4 getProjectionMatrix();
    glm::mat4 getViewMatrix();
private:
    int width, height;
    glm::vec3 from, to, up;
    float fov;
};

#endif
