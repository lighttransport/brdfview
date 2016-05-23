#include "shader.h"

// === Specular ===
float SpecularShader::sample(const glm::vec3& light_dir,
                             const glm::vec3& out_dir,
                             const glm::vec3& normal) {
    // parameter check
    this->spec_factor = std::max(this->spec_factor, 0.f);

    // shade
    glm::vec3 r = glm::reflect(-light_dir, normal);
    float l_specular = glm::dot(out_dir, r);
    if (l_specular < 0.f) l_specular = 0.f; // for negative normal
    return glm::pow(l_specular, this->spec_factor);
}


// === KajiyaKay ===
inline float KajiyaKayDiffuse(glm::vec3 tangent, glm::vec3 light_dir) {
    glm::vec3 Ln = normalize(light_dir);
    float df = dot(tangent, Ln);
    df = 1.0 - (df * df);
    if (df < 0.0) df = 0.0;
    if (df > 0.0) df = sqrt(df);
    return df;
}

inline float KajiyaKaySpecular(glm::vec3 tangent, glm::vec3 light_dir,
                               glm::vec3 view_dir) {
    float sintl = KajiyaKayDiffuse(tangent, light_dir);
    float vt = dot(view_dir, tangent);
    float sinte = 1.0 - (vt * vt);
    if (sinte < 0.0) sinte = 0.0;
    if (sinte > 0.0) sinte = sqrt(sinte);
    glm::vec3 Ln = glm::normalize(light_dir);
    float kspec = sintl * sinte - glm::dot(Ln, tangent) * vt;
    if (kspec < 0.0) kspec = 0.0;
    return kspec;
}

float KajiyaKayShader::sample(const glm::vec3& light_dir,
                              const glm::vec3& out_dir,
                              const glm::vec3& normal) {
    // parameter check
    this->spec_factor = std::max(this->spec_factor, 0.f);
    this->kd = std::max(this->kd, 0.f);
    this->ks = std::max(this->ks, 0.f);

    // shade
    glm::vec3 T = this->tangent;
    glm::vec3 V = out_dir;
    glm::vec3 L = light_dir;
    float ss = KajiyaKaySpecular(T, L, V);
    ss = pow(ss, this->spec_factor);
    float intensity = this->kd * KajiyaKayDiffuse(T, L) + this->ks * ss;
    return intensity;
}


// === AF Marschner ===
namespace {

#define dumpifbadassertionf(x) dumpifbadassertionf_((#x), (x));
void dumpifbadassertionf_(std::string s, float x) {
    if(std::isnan(x) || std::isinf(x)) {
        std::cerr << s << " = " << (x) << std::endl;
    }
}
#define dump(x) { std::cerr << #x << " = " << (x) << std::endl; }
inline float ClampUnitAbs(float in) {
    return glm::clamp(in, -1.f, 1.f);
}

inline int sign(float x) {
    return x > 0 ? 1 : x < 0 ? -1 : 0;
}

inline void binsearch(bool decreasing, int& num_of_solutions, float* h, float p,
                      float phi, float etad, float retmin, float retmax) {
    // The p*PI factor makes it impossible to find roots
    auto phiph = [&](float x){
        return 2 * p * asin(ClampUnitAbs(x / etad))
               - 2 * asin(ClampUnitAbs(x)) - phi;
    };

    if(sign(phiph(retmin)) * sign(phiph(retmax)) < 0){
        for(int i = 0; fabs(retmax - retmin) > 1e-7 && i < 28; i++){
            const float ret = (retmax + retmin) / 2;
            if(decreasing)	
                ((phiph(ret) < 0) ? retmax : retmin) = ret;
            else
                ((phiph(ret) > 0) ? retmax : retmin) = ret;
        }
        h[num_of_solutions++] = (retmax + retmin)/2;
    }
}

inline int h_(float *h, float p, float phi, float etad) {
    int num_of_solutions = 0;
    if (p == 0) {
        h[0] = sin(-phi / 2);
        return 1;
    // Newton's Method
    } else if(p == 1) {
        binsearch(true,  num_of_solutions, h, p, phi, etad, -1, 1);
        return num_of_solutions;
    // Binary search
    } else if(p == 2) {
        const float localmin = sqrt(4 - etad * etad) / sqrt(3);
        binsearch(true,  num_of_solutions, h, p, phi, etad, -1, -localmin);
        binsearch(false, num_of_solutions, h, p, phi, etad, -localmin,
                  localmin);
        binsearch(true,  num_of_solutions, h, p, phi, etad, localmin, 1);
        return num_of_solutions;		
    } else {
        return 0;
    }
}

inline float Fresnel(float eta_1, float eta_2, float gamma) {
    const float s = sin(gamma) * eta_1/eta_2;
    const float a = eta_1 * cos(gamma);
    const float b = eta_2 * sqrt(1-s*s);
    const float r = (a-b)/(a+b);
    return std::min(r * r, 1.f);
}

inline float F_(float etad, float etadd, float gamma, bool inv) {
    const float fs2 = Fresnel(!inv ? 1 : etad,  !inv ? etad  : 1, gamma);
    const float ft2 = Fresnel(!inv ? 1 : etadd, !inv ? etadd : 1, gamma);
    return 0.5 * (fs2 + ft2);
}

inline float A_(int p, float h, float gamma_i, float gamma_t, float etad,
                float etadd, float theta_d, float sigma_a) {
    if (p == 0) {
        return F_(etad, etadd, gamma_i, false);
    } else {
        const float cos_theta_d = std::max(fabs(cos(theta_d)), 1e-30);
        const float T = exp(-2 * sigma_a/cos_theta_d * (1 + cos(2 * gamma_t)));
        const float fres = F_(etad, etadd, gamma_i, false);
        const float fres_inv = F_(etad, etadd, gamma_t, true);
        dumpifbadassertionf(T)
        dumpifbadassertionf(fres)
        dumpifbadassertionf(fres_inv)
        return (1 - fres) * (1 - fres) * (p == 1 ? T : (fres_inv * T * T));
    }
}

inline float dphi_dh_inv_(int p, float h, float etad) {
    if (p == 0){
        // return sqrt(1 - h * h) / (2.f * fabs(sin(h)));  // gamma_i
        return sqrt(1 - h * h)/2;
    } else {
        const float a = sqrt(1 - h * h);
        const float b = sqrt(etad * etad - h * h);
        const float ab = a * b;
        dumpifbadassertionf(a)
        dumpifbadassertionf(b)
        return ab / (2.f * std::max(fabs(p * a - b), 1e-20));
    }
}


inline float N_p_(int p, float theta_d, float phi, float eta, float sigma_a) {
    const float etad  = sqrt(eta * eta - sin(theta_d) * sin(theta_d)) /
                        std::max(fabs(cos(theta_d)), 1e-20);
    const float etadd = eta * eta / std::max(sqrt(eta * eta -
                        sin(theta_d) * sin(theta_d)), 1e-20)
                        * fabs(cos(theta_d));
    dumpifbadassertionf(etad)
    dumpifbadassertionf(etadd)

    float h_solutions[3];
    const int num_of_solutions = h_(h_solutions, p, phi, etad);

    float N_p = 0;
    for (int i = 0; i < num_of_solutions; i++) {
        const float h = h_solutions[i];
        const float gamma_i = asin(ClampUnitAbs(h));
        const float gamma_t = asin(ClampUnitAbs(h / etad));
        const float dphi_dh_inv = dphi_dh_inv_(p, h, etad);
        const float A = A_(p, h, gamma_i, gamma_t, etad, etadd, theta_d,
                           sigma_a);
        N_p += A * dphi_dh_inv * 0.5;
        // Debug
        if(fabs(h) > 1) dump(h)
        dumpifbadassertionf(gamma_i)
        dumpifbadassertionf(gamma_t)
        dumpifbadassertionf(A)
        dumpifbadassertionf(dphi_dh_inv)
    }
    return N_p;
}

inline float gaussian(float x, float mu, float sigma) {
    const float a = 1.f / sqrt(2 * M_PI);
    return exp(-(x - mu) * (x - mu) * 0.5 / (sigma * sigma)) * a / sigma;
}

inline float rnd() {
    return ((float)rand()) / RAND_MAX;
}

// #define af_marschner_hair_shader_approx_dist
inline float AFMarschner(float phi, float theta_d, float theta_h,
                         AFMarschnerHairParams *hp) {
    const int NUM_JITTER = 1;
    const float JITTER = 0.f;
    float R, TT, TRT;

    // R
    float M_R = gaussian(theta_h, hp->longitudinalShiftR,
                         hp->longitudinalWidthR);
#ifdef af_marschner_hair_shader_approx_dist
    float N_R = cos(phi * 0.5);
#else
    float N_R = 0;
    {
        const float jitter_theta = JITTER * (1 - 2 * rnd());
        const float jitter_phi = JITTER * (1 - 2 * rnd());
        for (int i = 0; i < NUM_JITTER; i++) {
            N_R += N_p_(0, theta_d + jitter_theta, phi + jitter_phi,
                        hp->eta, hp->sigma_a);
        }
        N_R /= NUM_JITTER;
    }
#endif
    R = hp->intensityR * M_R * N_R;
    R = std::max(R, 0.f); // Prevent negative value

    // TT
    float M_TT = gaussian(theta_h, hp->longitudinalShiftTT,
                          hp->longitudinalWidthTT);
#ifdef af_marschner_hair_shader_approx_dist
    float N_TT = gaussian(M_PI, fabs(phi), hp->azimuthalWidthTT);
#else
//     float N_TT = N_p_(1, theta_d, phi, hp->eta, hp->sigma_a);
    float N_TT = 0;
    {
        const float jitter_theta = JITTER*(1-2*rnd());
        const float jitter_phi = JITTER*(1-2*rnd());
        for (int i = 0; i < NUM_JITTER; i++) {
            N_TT += N_p_(1, theta_d + jitter_theta, phi + jitter_phi,
                         hp->eta, hp->sigma_a);
        }
        N_TT /= NUM_JITTER;
    }
#endif
    TT = hp->intensityTT * M_TT * N_TT;
    TT = std::max(TT, 0.f); // Prevent negative value

    // TRT
    float M_TRT = gaussian(theta_h, hp->longitudinalShiftTRT,
                           hp->longitudinalWidthTRT);
#ifdef af_marschner_hair_shader_approx_dist
    float N_TRGNG = cos(phi * 0.5);
    float N_G = hp->intensityG * gaussian(hp->azimuthalShiftG, fabs(phi),
                                          hp->azimuthalWidthG);
    float N_TRT = N_TRGNG + N_G;
#else
//     float N_TRT = N_p_(2, theta_d, phi, hp->eta, hp->sigma_a);
    float N_TRT = 0;
    {
        const float jitter_theta = JITTER*(1-2*rnd());
        const float jitter_phi = JITTER*(1-2*rnd());
        for (int i = 0; i < NUM_JITTER; i++) {
            N_TRT += N_p_(0, theta_d + jitter_theta, phi+jitter_phi,
                          hp->eta, hp->sigma_a);
        }
        N_TRT /= NUM_JITTER;
    }
#endif

    TRT = hp->intensityTRT * M_TRT * N_TRT;
    TRT = std::max(TRT, 0.f); // Prevent negative value

    // Normalize.
    //
    // Scale factor: c.f. Mar2003 p.6, 7.
    float cosThetaD = cos(theta_d);
    // TODO: clampling scale significantly alters the scale of values for some
    //       theta_d probably the cause of dot noises
    float scale = 1.0 / std::max(cosThetaD * cosThetaD, 1e-1f);

    dumpifbadassertionf(scale);
    dumpifbadassertionf(R);
    dumpifbadassertionf(TT);
    dumpifbadassertionf(TRT);
    return (R + TT + TRT) * scale;
}

inline glm::vec3 local_spherical(const glm::vec3& v, const glm::vec3& x,
                                 const glm::vec3& y, const glm::vec3& z) {
    const float xdash = glm::dot(v, x);
    const float ydash = glm::dot(v, y);
    const float zdash = glm::dot(v, z);
    const float r = sqrt(xdash*xdash + ydash*ydash + zdash*zdash);
    return glm::vec3(atan2(ydash, xdash),
                     M_PI * 0.5 - acos(ClampUnitAbs(zdash/r)),
                     0);
}

inline void findAngles(float& phi, float& theta_d, float& theta_h,
                       float& theta_t, glm::vec3& Vn, glm::vec3& Ln,
                       glm::vec3& U, glm::vec3& V, glm::vec3& W){
    glm::vec3 omegaO = local_spherical(Vn, V, W, U);
    glm::vec3 omegaI = local_spherical(Ln, V, W, U);
      
    // phi = fmod( fabs( omegaO[0] - omegaI[0] ), 2.0 * PI );
    phi = fabs(omegaO[0] - omegaI[0]);
    if (phi > M_PI) phi -= 2.f * M_PI;
    // phi = omegaO[0] - omegaI[0];

    theta_d = fabs(omegaO[1] - omegaI[1]) * 0.5;
    theta_h = (omegaO[1] + omegaI[1]) * 0.5;
    if(theta_d > M_PI / 2) theta_d -= M_PI;
    // theta_d = PI - theta_d;
    theta_t = omegaI[1];
}

} // namespace 

float AFMarschnerShader::sample(const glm::vec3& light_dir,
                                const glm::vec3& out_dir,
                                const glm::vec3& normal) {
    glm::vec3 U = glm::normalize(this->tangent);// dPdv
    glm::vec3 V = glm::normalize(normal); // N
    glm::vec3 W = glm::normalize(glm::cross(U, V));
    glm::vec3 Vn = glm::normalize(-out_dir);
    glm::vec3 Ln = glm::normalize(light_dir);

    float phi;
    float theta_d;
    float theta_h;
    float theta_t;
    findAngles(phi, theta_d, theta_h, theta_t, Vn, Ln, U, V, W);

    return AFMarschner(phi, theta_d, theta_h, hp);
}
