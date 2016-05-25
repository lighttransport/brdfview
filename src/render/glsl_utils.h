#ifndef GLSL_UTILS_160412
#define GLSL_UTILS_160412

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

void checkGlError(int idx=-1);

// Shader Loader
GLuint loadShaders(const std::string& vs_file, const std::string& fs_file);


// Vertex Array
class GLSLVertexArray {
public:
    GLSLVertexArray() : inited(false), vertex_array_id(0), element_buf_id(0) {}
    ~GLSLVertexArray() { clear(); }

    template<class T1, class... T2>
    void set(const GLuint program_id,
             const std::vector<T1>& element, // indices
             const std::vector<T2>&... attributes);

    void draw();

    void clear();

private:
    bool inited;
    GLuint vertex_array_id;  // vertex array
    GLuint element_buf_id;  // indices buffer
    std::map<int, GLint> attribute_locs;  // other attribute buffers
    std::map<GLint, GLuint> attribute_ids;

    GLsizei element_count;
    GLenum element_type;

    template<class T>
    void setupElement(const std::vector<T>& element);

    template<class First, class... Rest>
    void setupAttribute(const GLuint idx,
                        const GLuint program_id,
                        const std::vector<First>& first,
                        const std::vector<Rest>&... rest);

    template<class T>
    void setupAttribute(const GLuint idx,
                        const GLuint program_id,
                        const std::vector<T>& attribute);
};

#include "glsl_utils_private.h"

#endif
