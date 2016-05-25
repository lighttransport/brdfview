#ifndef GL_WINDOW_160412
#define GL_WINDOW_160412

#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "gl_utils.h"


// Interface class for GLWindow.
// To use mouse input, set inherited class through GLWindow::setCamera().
class GLCamera {
public:
    virtual void reshapeScreen(int width, int height) = 0;
    virtual void move(float dx, float dy, float dz) = 0;
    virtual void rotateOrbit(float dtheta, float dphi) = 0;
};


// Window class for GL.
class GLWindow {
public:
    GLWindow(int width, int height) : width(width), height(height),
                                        camera(NULL) {}
    ~GLWindow() { exit(); }

    bool init(const std::string& title="window", bool user_input=true,
              int vsync_interval=0);
    void setCamera(GLCamera *camera) { this->camera = camera; }
    bool shouldClose();
    void active();
    void update();
    void exit();

    void useInput(bool use) { this->use_input = use; }
    bool getKeyStatus(int& key, int& scancode, int& action, int& mods,
                      bool clear=true);
    bool getCharStatus(unsigned int& key_char, bool clear=true);

    int getWidth() { return width; }
    int getHeight() { return height; }

    GLFWwindow* getRawRef() { return this->window; }

private:
    int width, height;
    bool mouse_moved;  // to prevent from using uninitialized mouse coordinates
    double pre_mouse_x, pre_mouse_y;  // mouse status
    bool mouse_left_pussing;
    bool key_pushed;
    int key, key_scancode, key_action, key_mods;  // keyboard status
    bool key_char_pushed;
    unsigned int key_char;  // keyboard character status

    static bool glfw_inited, glew_inited;

    GLFWwindow* window;
    GLCamera* camera;
    bool use_input = true;

    static void reshapeFunc(GLFWwindow *window, int width, int height);
    static void keyboardFunc(GLFWwindow *window, int key, int scancode,
                             int action, int mods);
    static void charFunc(GLFWwindow *window, unsigned int c);
    static void clickFunc(GLFWwindow* window, int button, int action, int mods);
    static void motionFunc(GLFWwindow* window, double mouse_x, double mouse_y);
};

#endif
