varying vec3 position;
varying vec3 normal;

const vec3 LIGHT_POS = vec3(0, 0, 1000);

uniform vec3 Kd;
uniform vec3 Ka;
vec3 Ks = vec3(0.01, 0.01, 0.01);
const float GLOSSINESS = 0.2;

void main() {
    vec3 light_dir = normalize(LIGHT_POS - position);
    vec3 look_dir = normalize(position - vec3(0, 0, 0));

    vec3 col = vec3(0.0, 0.0, 0.0);

    // diffuse
    float l_diffuse = dot(light_dir, normal);
    if (l_diffuse < 0.0) l_diffuse = 0.0; // for negative normal
    col += l_diffuse * Kd;

    // specular
    vec3 r = reflect(-light_dir, normal);
    float l_specular = dot(-look_dir, r);
    if (l_specular < 0.0) l_specular = 0.0; // for negative normal
    col += pow(l_specular, GLOSSINESS) * Ks;

    // ambient
    col += Ka;

    col = clamp(col, 0.0, 1.0);

    gl_FragColor = vec4(col, 1);
}
