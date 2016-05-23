#ifndef GLSL_WINDOW_160412
#define GLSL_WINDOW_160412

#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


// Interface class for GLSLWindow.
// To use mouse input, set inherited class through GLSLWindow::setCamera().
class GLSLCamera {
public:
    virtual void reshapeScreen(int width, int height) = 0;
    virtual void move(float dx, float dy, float dz) = 0;
    virtual void rotateOrbit(float dtheta, float dphi) = 0;
};


// Window class for GLSL.
class GLSLWindow {
public:
    GLSLWindow(int width, int height) : width(width), height(height),
                                        camera(NULL) {}
    ~GLSLWindow() { exit(); }

    bool init(const std::string& title="window", bool user_input=true,
              int vsync_interval=0);
    void setCamera(GLSLCamera *camera) { this->camera = camera; }
    bool shouldClose();
    void useInput(bool use) { this->use_input = use; }
    void active();
    void update();
    void exit();

    int getWidth() { return width; }
    int getHeight() { return height; }

    GLFWwindow* getRawRef() { return this->window; }

private:
    int width, height;
    bool mouse_moved;  // to prevent from using uninitialized mouse coordinates
    double pre_mouse_x, pre_mouse_y;
    bool mouse_left_pussing;

    static bool glfw_inited, glew_inited;

    GLFWwindow* window;
    GLSLCamera* camera;
    bool use_input = true;

    static void reshapeFunc(GLFWwindow *window, int width, int height);
    static void keyboardFunc(GLFWwindow *window, int key,
                             int scancode, int action, int mods);
    static void clickFunc(GLFWwindow* window, int button, int action, int mods);
    static void motionFunc(GLFWwindow* window, double mouse_x, double mouse_y);
};

#endif
