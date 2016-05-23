#include "mesh.h"


void updateNormals(Mesh& mesh) {
    std::vector<glm::uvec3> &indices = mesh.indices;
    std::vector<glm::vec3> &vertices = mesh.vertices;
    std::vector<glm::vec3> &normals = mesh.normals;

    // initialize normals
    normals.resize(vertices.size());
    for(int i = 0; i < normals.size(); i++) {
        normals[i] = glm::vec3(0.f, 0.f, 0.f);
    }
    // initialize normals weight
    std::vector<int> normals_weight(vertices.size());
    for (int i = 0; i < normals_weight.size(); i++) {
        normals_weight[i] = 0;
    }
    // compute normals based on the faces
    for (int tri_idx = 0; tri_idx < indices.size(); tri_idx++) {
        unsigned int v_idx0 = indices[tri_idx][0];
        unsigned int v_idx1 = indices[tri_idx][1];
        unsigned int v_idx2 = indices[tri_idx][2];
        glm::vec3 v0 = vertices[v_idx0];
        glm::vec3 v1 = vertices[v_idx1];
        glm::vec3 v2 = vertices[v_idx2];
        glm::vec3 e1 = v1 - v0;
        glm::vec3 e2 = v2 - v0;
        glm::vec3 n = glm::normalize(glm::cross(e1, e2));
        // accumulate normals
        normals[v_idx0] += n;
        normals[v_idx1] += n;
        normals[v_idx2] += n;
        // count up
        normals_weight[v_idx0]++;
        normals_weight[v_idx1]++;
        normals_weight[v_idx2]++;
    }
    // average
    for (int v_idx = 0; v_idx < normals.size(); v_idx++) {
        normals[v_idx] /= (float)normals_weight[v_idx];
    }
}


void createGround(Mesh& mesh, float scale, float eps) {
    mesh.clear();

    // y up
    // vertex
    mesh.vertices.resize(4);
    mesh.vertices[0] = glm::vec3(-scale, -eps, -scale);
    mesh.vertices[1] = glm::vec3(-scale, -eps, scale);
    mesh.vertices[2] = glm::vec3(scale, -eps, -scale);
    mesh.vertices[3] = glm::vec3(scale, -eps, scale);

    // normals
    mesh.normals.resize(4);
    for (int i = 0; i < 4; i++) {
        mesh.normals[i] = glm::vec3(0.f, 1.f, 0.f);
    }

    // indices
    mesh.indices.resize(2);
    mesh.indices[0] = glm::uvec3(0, 1, 2);  // ounterclock-wise
    mesh.indices[1] = glm::uvec3(1, 3, 2);
}


// Create intensity hemisphere
void createBRDFMesh(Mesh& mesh, BaseShader& shader, const glm::vec3& light_pos,
                    float scale, int n_phi, const glm::vec3& up_dir) {
    mesh.clear();

    glm::vec3 light_dir = glm::normalize(light_pos);

    // y direction step
    int n_theta = std::max(n_phi / 4, 1); // 2 PI -> 1/2 PI

    // rotation matrix
    glm::mat4 rot = glm::orientation(glm::vec3(0.f, 1.f, 0.f),
                                     glm::normalize(up_dir));
    if (rot != rot) {
        // maybe 180 degree rotation
        rot = glm::mat4(1.f);
        rot[1][1] *= -1;
    }

    // vertices
    mesh.vertices.resize(n_phi * n_theta + 1);
    for (int i_phi = 0; i_phi < n_phi; i_phi++) {
        float phi_rad = 2.0 * M_PI * i_phi / n_phi;

        for (int i_theta = 0; i_theta < n_theta; i_theta++) {
            float theta_rad = 0.5 * M_PI * i_theta / n_theta; // before the top

            // y up
            float x = scale * cos(theta_rad) * sin(phi_rad);
            float y = scale * sin(theta_rad);
            float z = scale * cos(theta_rad) * cos(phi_rad);
            glm::vec3 pos = glm::mat3(rot) * glm::vec3(x, y, z); // rotation
            // sampling
            float intensity = shader.sample(light_dir, pos, up_dir);
            // set
            mesh.vertices[i_phi * n_theta + i_theta] = pos * intensity;
        }
    }
    {
        // the top one
        glm::vec3 top_pos = glm::mat3(rot) * glm::vec3(0, scale, 0); // rotation
        float intensity = shader.sample(light_dir, top_pos, up_dir);
        mesh.vertices[n_phi * n_theta] = glm::vec3(top_pos) * intensity;
    }

    // register indices
    for (int i_phi = 0; i_phi < n_phi; i_phi++) {
        int i_phi2 = (i_phi + 1) % n_phi; // rotation

        for (int i_theta = 0; i_theta < n_theta; i_theta++) {
            int i_theta2 = i_theta + 1;

            if (i_theta2 != n_theta) {
                unsigned int idx0 = i_phi * n_theta + i_theta;
                unsigned int idx1 = i_phi2 * n_theta + i_theta;
                unsigned int idx2 = i_phi * n_theta + i_theta2;
                unsigned int idx3 = i_phi2 * n_theta + i_theta2;

                glm::uvec3 triangle0(idx0, idx1, idx2); // counterclock-wise
                glm::uvec3 triangle1(idx1, idx3, idx2);
                mesh.indices.push_back(triangle0);
                mesh.indices.push_back(triangle1);
            } else {
                // top
                unsigned int idx0 = i_phi * n_theta + i_theta;
                unsigned int idx1 = i_phi2 * n_theta + i_theta;
                unsigned int idx2 = n_phi * n_theta; // top

                glm::uvec3 triangle0(idx0, idx1, idx2);
                mesh.indices.push_back(triangle0);
            }
        }
    }

    // normals
    updateNormals(mesh);
}
