#ifndef MESH_H_160518
#define MESH_H_160518
#include <vector>

#define GLM_FORCE_RADIANS 
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp> 

#include "shader.h"


class Mesh {
public:
    Mesh() {};
    ~Mesh() {};
    void clear() {
        indices.clear();
        vertices.clear();
        normals.clear();
    }
    std::vector<glm::uvec3> indices;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
};

void updateNormals(Mesh& mesh);
void createGround(Mesh& mesh, float scale, float eps=0.01f);
void createBRDFMesh(Mesh& mesh, BaseShader& shader, const glm::vec3& light_pos,
                    float scale, int n_phi,
                    const glm::vec3& up_dir=glm::vec3(0.f, 1.f, 0.f));
#endif
