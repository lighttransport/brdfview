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


void drawMesh(Mesh &mesh, glm::vec3 color) {
    // enable light and material
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glColorMaterial(GL_FRONT, GL_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
    // color
    glColor3fv(&color[0]);

    // draw
    glBegin(GL_TRIANGLES);
    for (int tri = 0; tri < mesh.indices.size(); tri++) {
        for (int i = 0; i < 3; i++) {
            unsigned int v_idx = mesh.indices[tri][i];
            glNormal3fv(&(mesh.normals[v_idx][0]));
            glVertex3fv(&(mesh.vertices[v_idx][0]));
        }
    }
    glEnd();

    // disable light and material
    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHTING);
}

void drawLine(glm::vec3& pos0, glm::vec3& pos1, float width, glm::vec3 color) {
    // color and width
    glColor3fv(&color[0]);
    glLineWidth(width);

    // draw
    glBegin(GL_LINES);
    glVertex3fv(&(pos0[0]));
    glVertex3fv(&(pos1[0]));
    glEnd();
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

    // fps counter
    GLFpsCounter fps;

    // imgui
    ImGui_ImplGlfw_Init(window.getRawRef(), false);

    // rendering loop
    while (!window.shouldClose()) {
        fps.update();
        window.active();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // camera
        glm::mat4 mv_mat = camera.getViewMatrix() * glm::mat4(1.0);
        glm::mat4 p_mat = camera.getProjectionMatrix();
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glMultMatrixf(&mv_mat[0][0]);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glMultMatrixf(&p_mat[0][0]);
        checkGlError(101);

        // create mesh
        Mesh brdf_mesh;  // brdf mesh
        createBRDFMesh(brdf_mesh, *(shaders[shader_idx]), light_pos, 1.f, 100,
                       normal);
        Mesh ground_mesh;  // ground mesh
        createGround(ground_mesh, 1);

        // draw meshes
        drawMesh(brdf_mesh, glm::vec3(0.f, 1.f, 0.f));
        drawMesh(ground_mesh, glm::vec3(0.5f));
        checkGlError(102);

        // draw lines
        drawLine(org_pos, light_pos, 5.f, glm::vec3(1.f));
        if (shader_idx != 0) {  // tangent line
            glm::vec3 neg_tangent = -tangent;
            drawLine(tangent, neg_tangent, 10.f, glm::vec3(1.f, 0.5f, 0.2f));
        }

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
        checkGlError(103);

        // Check input type
        ImGuiIO &io = ImGui::GetIO();
        bool in_imgui = io.WantCaptureMouse || io.WantCaptureKeyboard;
        window.useInput(!in_imgui);

        // show
        window.update();
    }

    // exit
    std::cout << "* Exit" << std::endl;
    ImGui_ImplGlfw_Shutdown();

    return 0;
}
