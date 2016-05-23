#include "./camera.h"

void Camera::reshapeScreen(int width, int height) {
    this->width = width;
    this->height = height;
}
void Camera::move(float dx, float dy, float dz) {
    dx *= 0.05;  // scaling
    dy *= 0.05;
    dz *= 0.05;

    glm::vec3 dir_l = (this->to - this->from);
    glm::vec3 right_l = cross(dir_l, this->up);
    glm::vec3 up_l = cross(right_l, dir_l);

    dir_l   *= dx / glm::length(dir_l);
    right_l *= dy / glm::length(right_l);
    up_l    *= dz / glm::length(up_l);

    this->from += dir_l + right_l + up_l;
    this->to += right_l + up_l;
}
void Camera::rotateOrbit(float dtheta, float dphi) {
    dtheta *= 0.005; // scaling
    dphi *= 0.005;

    glm::vec3 dir = this->to - this->from;
    float dir_norm = glm::length(dir);
    dir *= 1.0 / dir_norm;  // normalize

    float theta = atan2f(dir.x, dir.z);
    float phi   = asinf(dir.y);
    theta += dtheta;
    phi   += dphi;
    // Check phi range
    const float HALF_PIE = M_PI * 0.5f - std::numeric_limits<float>::epsilon();
    if (HALF_PIE < phi) phi = HALF_PIE;
    else if (phi < -HALF_PIE) phi = -HALF_PIE;

    dir.x = cosf(phi)*sinf(theta);
    dir.y = sinf(phi);
    dir.z = cosf(phi)*cosf(theta);

    dir *= dir_norm;//reverce normalize
    this->from = this->to - dir;
}
glm::mat4 Camera::getProjectionMatrix() {
    float aspect = 1.f;
    if (width > 0 && height > 0) {
        aspect = (float)width / (float)height;
    }
    return glm::perspective(fov, aspect, 0.001f, 10000.0f);
}
glm::mat4 Camera::getViewMatrix() {
    return glm::lookAt(from, to, up);
}


