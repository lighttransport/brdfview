#include "gl_fps.h"

using namespace std;

GLFpsCounter::GLFpsCounter() {
    init();
}

void GLFpsCounter::init() {
    last_time = glfwGetTime();
    frame_count = 0;
}

void GLFpsCounter::update() {
    double current_time = glfwGetTime();
    frame_count++;
    if (current_time - last_time >= 1.0) {
        cout << frame_count << "fps" << endl;
        frame_count = 0;
        last_time += 1.0;
    }
}
