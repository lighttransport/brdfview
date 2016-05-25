#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <limits>

#include <GL/glew.h>
#define GLM_FORCE_RADIANS 
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp> 
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "io/gl_fps.h"
#include "mesh.h"
#include "render/camera.h"
#include "render/glsl_utils.h"
#include "render/glsl_window.h"
#include "shader.h"


void setMeshToVA(GLuint program_id, Mesh& mesh, GLSLVertexArray& va) {
    va.set(program_id, mesh.indices, mesh.vertices, mesh.normals);
}


int main(int argc, char const* argv[]) {
    // parameters
    glm::vec3 org_pos(0, 0, 0);
    glm::vec3 light_pos(1, 1, 0);
    glm::vec3 normal(0, 1, 0);
    glm::vec3 tangent(0, 0, 1);
    // shader
    SpecularShader specular_shader;
    KajiyaKayShader kajiyakay_shader;
    AFMarschnerShader afmarschner_shader;
    // shader array
    int shader_idx = 0;
    std::vector<BaseShader*> shaders{
        &specular_shader,
        &kajiyakay_shader,
        &afmarschner_shader,
    };
    const char* shader_names[] = {
        "Specular Shader",
        "Kajiyakay Shader",
        "AF Marschner Shader",
    };

    // glsl window
    GLSLWindow window(1024, 512);
    bool gl_ret = window.init("title", true, 0);
    if (!gl_ret) return false;

    // camera
    Camera camera;
    window.setCamera(&camera);

    // shader
    GLuint program_id = loadShaders("../src/render/simple.vs",
                                    "../src/render/simple.fs");
    if (program_id == 0) return false;

    // viewer uniform
    GLuint mvp_mat_id = glGetUniformLocation(program_id, "mvp_mat");
    GLuint mv_mat_id = glGetUniformLocation(program_id, "mv_mat");
    GLuint kd_vec_id = glGetUniformLocation(program_id, "Kd");
    GLuint ka_vec_id = glGetUniformLocation(program_id, "Ka");

    GLFpsCounter fps;

    // vertex array
    GLSLVertexArray brdf_vertex_array, ground_vertex_array;

    // imgui
    ImGui_ImplGlfw_Init(window.getRawRef(), false);

    // rendering loop
    while (!window.shouldClose()) {
        fps.update();
        window.active();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // create mesh
        Mesh brdf_mesh;  // brdf mesh
        createBRDFMesh(brdf_mesh, *(shaders[shader_idx]), light_pos, 1.f, 100,
                       normal);
        Mesh ground_mesh;  // ground mesh
        createGround(ground_mesh, 1);

        // set mesh to vertex array
        setMeshToVA(program_id, brdf_mesh, brdf_vertex_array);
        setMeshToVA(program_id, ground_mesh, ground_vertex_array);

        // use glsl shader
        glUseProgram(program_id);

        // common uniforms
        glm::mat4 mv_mat = camera.getViewMatrix() * glm::mat4(1.0);
        glm::mat4 mvp_mat = camera.getProjectionMatrix() * mv_mat;
        glUniformMatrix4fv(mv_mat_id, 1, GL_FALSE, &mv_mat[0][0]);
        glUniformMatrix4fv(mvp_mat_id, 1, GL_FALSE, &mvp_mat[0][0]);

        // draw ground mesh
        glUniform3f(kd_vec_id, 0.2f, 0.2f, 0.2f);  // Gray
        glUniform3f(ka_vec_id, 0.05f, 0.05f, 0.05f);
        ground_vertex_array.draw();

        // draw brdf mesh
        glUniform3f(kd_vec_id, 0.f, 1.f, 0.f);  // Green
        glUniform3f(ka_vec_id, 0.f, 0.25f, 0.f);
        brdf_vertex_array.draw();

        // draw light line
        glUniform3f(kd_vec_id, 0.f, 0.f, 0.f);  // White
        glUniform3f(ka_vec_id, 1.f, 1.f, 1.f);
        glLineWidth(5.0);
        glBegin(GL_LINES);
        glVertex3fv(&(org_pos[0]));
        glVertex3fv(&(light_pos[0]));
        glEnd();

        // draw tangent line
        if (shader_idx != 0) {
            glUniform3f(kd_vec_id, 0.f, 0.f, 0.0f);  // Orange
            glUniform3f(ka_vec_id, 1.f, 0.5f, 0.031f);
            glLineWidth(10.0);
            glBegin(GL_LINES);
            glm::vec3 neg_tangent = -tangent;
            glVertex3fv(&neg_tangent[0]);
            glVertex3fv(&tangent[0]);
            glEnd();
        }

        // clear glsl
        glUseProgram(0);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        // imgui
        ImGui_ImplGlfw_NewFrame();
        {
            ImGui::Text("BRDF View");
            ImGui::ListBox("Shader", &shader_idx, shader_names, shaders.size(),
                           std::min((int)shaders.size(), 5));
            assert(0 <= shader_idx && shader_idx < shaders.size());
            // Light
            ImGui::DragFloat3("Light Position", &light_pos[0], 0.01f);
            // Tangent
            if (shader_idx != 0) {
                ImGui::DragFloat3("Tangent", &(tangent[0]), 0.01f);
                tangent = glm::normalize(tangent);
                if (tangent != tangent) tangent = glm::vec3(0, 0, 1); // for nan 
            }
            // shader parameters
            if (shader_idx == 0) {
                // Specular
                ImGui::DragFloat("Spec Factor", &(specular_shader.spec_factor),
                                  0.1f);
            } else if (shader_idx == 1) {
                // Kajiya Kay
                kajiyakay_shader.tangent = tangent;
                ImGui::DragFloat("Spec Factor", &(kajiyakay_shader.spec_factor),
                                  0.1f);
                ImGui::DragFloat("Kd", &(kajiyakay_shader.kd), 0.01f);
                ImGui::DragFloat("Ks", &(kajiyakay_shader.ks), 0.01f);
            } else if (shader_idx == 2) {
                // AF Marschner
                afmarschner_shader.tangent = tangent;
                ImGui::DragFloat("Intensity R",
                                 &(afmarschner_shader.hp->intensityR), 0.01f);
                ImGui::DragFloat("Intensity TT",
                                 &(afmarschner_shader.hp->intensityTT), 0.01f);
                ImGui::DragFloat("Intensity TRT",
                                 &(afmarschner_shader.hp->intensityTRT), 0.01f);
            }
        }
        ImGui::Render();


        // Check input type
        ImGuiIO &io = ImGui::GetIO();
        bool in_imgui = io.WantCaptureMouse || io.WantCaptureKeyboard;
        window.useInput(!in_imgui);

        // show
        window.update();

        // error check
        GLenum errcode = glGetError();
        if (errcode != GL_NO_ERROR) {
            const GLubyte *errstring = gluErrorString(errcode);
            std::cout << errstring << std::endl;
        }
    }

    // exit
    std::cout << "* Exit" << std::endl;
    ImGui_ImplGlfw_Shutdown();
    glDeleteProgram(program_id);

    return 0;
}
