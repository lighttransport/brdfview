#ifndef SHADER_H_160518
#define SHADER_H_160518

#include <algorithm>
#include <string>
#include <iostream>
#include <cmath>

#define GLM_FORCE_RADIANS 
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

class BaseShader {
public:
    BaseShader() {};
    virtual float sample(const glm::vec3& light_dir, const glm::vec3& out_dir,
                         const glm::vec3& normal) = 0;
};


class SpecularShader : public BaseShader {
public:
    SpecularShader() : spec_factor(5.f) {}
    virtual float sample(const glm::vec3& light_dir, const glm::vec3& out_dir,
                         const glm::vec3& normal);
    float spec_factor;
};


class KajiyaKayShader : public BaseShader {
public:
    KajiyaKayShader() : tangent(0, 0, 1), spec_factor(25.f),
                        kd(0.3f), ks(0.3f) {}
    virtual float sample(const glm::vec3& light_dir, const glm::vec3& out_dir,
                         const glm::vec3& normal);
    glm::vec3 tangent;
    float spec_factor;
    float kd, ks;
};


inline float degToRad(float x){ return x * glm::pi<float>() / 180.f; }
struct AFMarschnerHairParams {
    AFMarschnerHairParams() {
        intensityR = 5;
        longitudinalShiftR = degToRad(-7.5);
        longitudinalWidthR = degToRad(7.5);
        intensityTT = 0.5;
        longitudinalShiftTT = -longitudinalShiftR / 2.0;
        longitudinalWidthTT = longitudinalWidthR / 2.0;
        azimuthalWidthTT = 3;
        intensityTRT = 0.5;
        longitudinalShiftTRT = -3.0 * longitudinalShiftR / 2.0;
        longitudinalWidthTRT = 2.0 * longitudinalWidthR;
        intensityG = 1;
        azimuthalShiftG = degToRad(30);
        azimuthalWidthG = degToRad(10);
        attenuationFromRoot = 1.0;
        eta = 1.55;

        thickness = 0.2f;
        sigma_a = 0.2f;
    }
    ~AFMarschnerHairParams() {}

    float intensityR;
    float longitudinalShiftR;
    float longitudinalWidthR;

    float intensityTT;
    float longitudinalShiftTT;
    float longitudinalWidthTT;
    float azimuthalWidthTT;

    float intensityTRT;
    float longitudinalShiftTRT;
    float longitudinalWidthTRT;

    float intensityG;
    float azimuthalShiftG;
    float azimuthalWidthG;

    float attenuationFromRoot;

    float eta;
    float sigma_a;
    float thickness;
};

class AFMarschnerShader : public BaseShader {
public:
    AFMarschnerShader() : tangent(0, 0, 1){
        this->hp = new AFMarschnerHairParams();
    }
    ~AFMarschnerShader() { delete this->hp; }
    virtual float sample(const glm::vec3& light_dir, const glm::vec3& out_dir,
                         const glm::vec3& normal);
    glm::vec3 tangent;
    AFMarschnerHairParams *hp;
};

#endif
