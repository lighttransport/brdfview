#ifndef GLSL_UTILS_PRIVATE_160412
#define GLSL_UTILS_PRIVATE_160412


// === GL Types ===
// type
inline GLenum getDataType(const std::vector<float>& v) { return GL_FLOAT; }
inline GLenum getDataType(const std::vector<glm::vec2>& v) { return GL_FLOAT; }
inline GLenum getDataType(const std::vector<glm::vec3>& v) { return GL_FLOAT; }
inline GLenum getDataType(const std::vector<glm::vec4>& v) { return GL_FLOAT; }
inline GLenum getDataType(const std::vector<signed int>& v) { return GL_INT; }
inline GLenum getDataType(const std::vector<glm::ivec2>& v) { return GL_INT; }
inline GLenum getDataType(const std::vector<glm::ivec3>& v) { return GL_INT; }
inline GLenum getDataType(const std::vector<glm::ivec4>& v) { return GL_INT; }
inline GLenum getDataType(const std::vector<unsigned int>& v) {return GL_UNSIGNED_INT; }
inline GLenum getDataType(const std::vector<glm::uvec2>& v) { return GL_UNSIGNED_INT; }
inline GLenum getDataType(const std::vector<glm::uvec3>& v) { return GL_UNSIGNED_INT; }
inline GLenum getDataType(const std::vector<glm::uvec4>& v) { return GL_UNSIGNED_INT; }

// channels
inline GLint getDataChannels(const std::vector<float>& v) { return 1; }
inline GLint getDataChannels(const std::vector<glm::vec2>& v) { return 2; }
inline GLint getDataChannels(const std::vector<glm::vec3>& v) { return 3; }
inline GLint getDataChannels(const std::vector<glm::vec4>& v) { return 4; }
inline GLint getDataChannels(const std::vector<signed int>& v) { return 1; }
inline GLint getDataChannels(const std::vector<glm::ivec2>& v) { return 2; }
inline GLint getDataChannels(const std::vector<glm::ivec3>& v) { return 3; }
inline GLint getDataChannels(const std::vector<glm::ivec4>& v) { return 4; }
inline GLint getDataChannels(const std::vector<unsigned int>& v) { return 1; }
inline GLint getDataChannels(const std::vector<glm::uvec2>& v) { return 2; }
inline GLint getDataChannels(const std::vector<glm::uvec3>& v) { return 3; }
inline GLint getDataChannels(const std::vector<glm::uvec4>& v) { return 4; }

// total size (bytes)
template<typename T>
inline GLint getDataTotalSize(const std::vector<T>& v) {
    return v.size() * getDataChannels(v) * sizeof(T);
}


// === Vertex Array ===
template<class T1, class... T2>
void GLSLVertexArray::set(const std::vector<T1>& element,
                          const std::vector<T2>&... attributes) {
    // generate vertex array
    if (!this->inited) {
        glGenVertexArrays(1, &(this->vertex_array_id));
    }
    glBindVertexArray(this->vertex_array_id);

    // indices
    this->setupElement(element);

    // attribute
    this->setupAttribute(0, attributes...);

    this->inited = true;
}

template<class T>
void GLSLVertexArray::setupElement(const std::vector<T>& element) {
    // generate
    if (!this->inited) {
        glGenBuffers(1, &(this->element_buf_id));
        this->element_count = element.size() * getDataChannels(element);
        this->element_type = getDataType(element);
    }

    // set
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->element_buf_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, getDataTotalSize(element),
                 &element[0] , GL_DYNAMIC_DRAW);
}

template<class First, class... Rest>
void GLSLVertexArray::setupAttribute(const GLuint idx,
                                     const std::vector<First>& first,
                                     const std::vector<Rest>&... rest) {
    this->setupAttribute(idx, first);
    this->setupAttribute(idx + 1, rest...);
}

template<class T>
void GLSLVertexArray::setupAttribute(const GLuint idx,
                                     const std::vector<T>& attribute) {
    // generate or fetch
    GLuint attribute_id;
    if (!this->inited) {
        glGenBuffers(1, &attribute_id);
        this->attribute_ids[idx] = attribute_id;
    } else {
        assert(this->attribute_ids.count(idx));
        attribute_id = this->attribute_ids[idx];
    }

    // set
    glEnableVertexAttribArray(idx);
    glBindBuffer(GL_ARRAY_BUFFER, attribute_id);
    glBufferData(GL_ARRAY_BUFFER, getDataTotalSize(attribute), &(attribute[0]),
                 GL_DYNAMIC_DRAW);
    glVertexAttribPointer(idx, getDataChannels(attribute),
                          getDataType(attribute), GL_FALSE, 0, (void*)0);
}
#endif
