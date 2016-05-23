#include "glsl_utils.h"

namespace {

bool readShaderCode(const std::string& filename, std::string& dst_code) {
    dst_code = "";

    // Open code file
    std::ifstream code_stream(filename.c_str(), std::ios::in);
    if (!code_stream.is_open()) {
        return false;
    }

    // Read each line
    std::string line = "";
    while (std::getline(code_stream, line)) {
        dst_code += "\n" + line;
    }
    code_stream.close();

    return true;
}

GLint compileShader(int id, const std::string& code){
    // Compile Shader
    char const * code_ptr = code.c_str();
    glShaderSource(id, 1, &code_ptr , NULL);
    glCompileShader(id);

    GLint status = GL_FALSE;
    int info_log_length;

    // Check Vertex Shader
    glGetShaderiv(id, GL_COMPILE_STATUS, &status);
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &info_log_length);
    if (info_log_length > 0) {
        char mess[info_log_length + 1];
        glGetShaderInfoLog(id, info_log_length, NULL, mess);
        std::cout << mess;
    }
    return status;
}

} // namespace 

// === Shader Loader ===
GLuint loadShaders(const std::string& vs_file, const std::string& fs_file){
    // Read vertex shader file
    std::string vs_code;
    if (!readShaderCode(vs_file, vs_code)) {
        std::cout << "Can't open vertex file:" << vs_file << std::endl;
        return 0;
    }

    // Read fragment shader file
    std::string fs_code;
    if (!readShaderCode(fs_file, fs_code)) {
        std::cout << "Can't open fragment file: " << fs_file << std::endl;
        return 0;
    }

    // Create the shaders
    GLuint vs_id = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs_id = glCreateShader(GL_FRAGMENT_SHADER);


    // Compile Vertex Shader
    std::cout << "* Compiling Vertex Shader : " << vs_file << std::endl;
    if (compileShader(vs_id, vs_code) == GL_FALSE) {
        std::cout << " >> Failed to compile Vertex Shader." << std::endl;
        return 0;
    }
    // Compile Fragment Shader
    std::cout << "* Compiling Fragment Shader : " << fs_file << std::endl;
    if (compileShader(fs_id, fs_code) == GL_FALSE) {
        std::cout << " >> Failed to compile Fragment Shader." << std::endl;
        return 0;
    }


    // Link the program
    std::cout << "* Linking Program" << std::endl;
    GLuint program_id = glCreateProgram();
    glAttachShader(program_id, vs_id);
    glAttachShader(program_id, fs_id);
    glLinkProgram(program_id);

    // Check the program
    GLint status = GL_FALSE;
    int info_log_length;
    glGetProgramiv(program_id, GL_LINK_STATUS, &status);
    glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &info_log_length);
    if (info_log_length > 0) {
        char mess[info_log_length + 1];
        glGetProgramInfoLog(program_id, info_log_length, NULL, mess);
        std::cout << mess;
    }
    if (status == GL_FALSE) {
        std::cout << " >> Failed to Ling Program." << std::endl;
        return 0;
    }

    // Delete Shaders
    glDeleteShader(vs_id);
    glDeleteShader(fs_id);

    return program_id;
}


// === Vertex Array ===
void GLSLVertexArray::draw() {
    if (inited) {
        glBindVertexArray(this->vertex_array_id);
        glDrawElements(GL_TRIANGLES, this->element_count,
                       this->element_type, (void*)0);
    } else {
        std::cout << "* Vertex array is not initialized" << std::endl;
    }
}

void GLSLVertexArray::clear() {
    inited = false;
    // attributes
    std::map<int, GLuint>::iterator it = this->attribute_ids.begin();
    for(; it != this->attribute_ids.end(); it++) {
        glDeleteBuffers(1, &(it->second));
    }
    this->attribute_ids.clear();
    // element
    if (!this->element_buf_id) {
        glDeleteBuffers(1, &(this->element_buf_id));
    }
    // vertex array
    if (!this->vertex_array_id) {
        glDeleteVertexArrays(1, &(this->vertex_array_id));
    }
}

