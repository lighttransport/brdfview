#include "glsl_window.h"

// === Window ===
bool GLSLWindow::glfw_inited = false;
bool GLSLWindow::glew_inited = false;

bool GLSLWindow::init(const std::string& title, bool user_input, 
                      int vsync_interval) {
    if (!GLSLWindow::glfw_inited) {
        std::cout << "* Initialize glfw" << std::endl;
        // Initialize GLFW
        if (!glfwInit()) {
            std::cout << " >> Failed to initialize GLFW" << std::endl;
            return false;
        }
        GLSLWindow::glfw_inited = true;

//         // Select OpenGL 3.3 Core Profile
//         glfwWindowHint(GLFW_SAMPLES, 4);
//         glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//         glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//         glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }


    // Open a window and create its OpenGL context
    this->window = glfwCreateWindow(this->width, this->height, title.c_str(),
                                    NULL, NULL);
    if (this->window == NULL) {
        std::cout << "Failed to open GLFW window. " << std::endl;
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(this->window);
    checkGlError(201);

    // Set vsync interval
    glfwSwapInterval(vsync_interval);

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(this->window, GLFW_STICKY_KEYS, GL_TRUE);

    // Bind window and this class
    glfwSetWindowUserPointer(this->window, this);

    // Callbacks
    glfwSetWindowSizeCallback(this->window, GLSLWindow::reshapeFunc);
    if (user_input) {
        glfwSetKeyCallback(this->window, GLSLWindow::keyboardFunc);
        glfwSetMouseButtonCallback(this->window, GLSLWindow::clickFunc);
        glfwSetCursorPosCallback(this->window, GLSLWindow::motionFunc);
    }

    if (!GLSLWindow::glew_inited) {
        std::cout << "* Initialize glew" << std::endl;
        // Initialize GLEW
        // glewExperimental = true; // for core profile
        if (glewInit() != GLEW_OK) {
            std::cout << "Failed to initialize GLEW." << std::endl;
            return false;
        }
        GLSLWindow::glew_inited = true;
    }

    // Background color
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);
    // Cull triangles which normal is not towards the camera
//     glEnable(GL_CULL_FACE);

    // Initialize window variables
    this->mouse_moved = false;
    this->pre_mouse_x = 0;
    this->pre_mouse_y = 0;
    this->mouse_left_pussing = false;

    return true;
}

bool GLSLWindow::shouldClose() {
    return (glfwWindowShouldClose(this->window) == GL_TRUE);
}

void GLSLWindow::active() {
    glfwMakeContextCurrent(this->window);
    checkGlError(202);
}

void GLSLWindow::update() {
    // Swap screen buffers
    glfwSwapBuffers(this->window);
    checkGlError(203);
    // Poll callbacks
    glfwPollEvents();
    checkGlError(204);
}

void GLSLWindow::exit() {
    glfwTerminate();
}

// GLFW Callbacks
void GLSLWindow::reshapeFunc(GLFWwindow *window, int width, int height){
    GLSLWindow* that = static_cast<GLSLWindow*>(glfwGetWindowUserPointer(window));

    that->width = width;
    that->height = height;
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    if (that->camera) {
        that->camera->reshapeScreen(width, height);
    }
}

void GLSLWindow::keyboardFunc(GLFWwindow *window, int key,
                            int scancode,int action, int mods) {
    GLSLWindow* that = static_cast<GLSLWindow*>(glfwGetWindowUserPointer(window));
    if (!that->use_input) return;

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        // Move camera
        float mv_x = 0.f, mv_y = 0.f, mv_z = 0.f;
        if (key == GLFW_KEY_K) mv_x += 1.f;
        else if (key == GLFW_KEY_J) mv_x += -1.f;
        else if (key == GLFW_KEY_L) mv_y += 1.f;
        else if (key == GLFW_KEY_H) mv_y += -1.f;
        else if (key == GLFW_KEY_P) mv_z += 1.f;
        else if (key == GLFW_KEY_N) mv_z += -1.f;

        if (that->camera) {
            that->camera->move(mv_x, mv_y, mv_z);
        }

        // Close window
        if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }
    }
}

void GLSLWindow::clickFunc(GLFWwindow* window, int button, int action, int mods){
    GLSLWindow* that = static_cast<GLSLWindow*>(glfwGetWindowUserPointer(window));
    if (!that->use_input) return;

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            that->mouse_left_pussing = true;
        } else if (action == GLFW_RELEASE) {
            that->mouse_left_pussing = false;
        }
    }
}

void GLSLWindow::motionFunc(GLFWwindow* window, double mouse_x, double mouse_y) {
    GLSLWindow* that = static_cast<GLSLWindow*>(glfwGetWindowUserPointer(window));
    if (!that->use_input) return;

    if (that->mouse_left_pussing && that->mouse_moved) {
        // Camera position rotation
        if (that->camera) {
            that->camera->rotateOrbit(that->pre_mouse_x - mouse_x,
                                      that->pre_mouse_y - mouse_y);
        }
    }

    // Update mouse point
    that->pre_mouse_x = mouse_x;
    that->pre_mouse_y = mouse_y;
    that->mouse_moved = true;
}


