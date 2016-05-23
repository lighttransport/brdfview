#ifndef GL_FPSCOUNTER_H_150331
#define GL_FPSCOUNTER_H_150331

#include <iostream>
#include <GLFW/glfw3.h>

class GLFpsCounter {
public:
    GLFpsCounter();
    void init();
    void update();

private:
    double last_time;
    int frame_count;
};
#endif
