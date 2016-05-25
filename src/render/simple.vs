attribute vec3 attrib0;  // model_position
attribute vec3 attrib1;  // model_normal

varying vec3 position;
varying vec3 normal;

uniform mat4 mvp_mat;
uniform mat4 mv_mat;

void main() {
    vec3 model_position = attrib0;
    vec3 model_normal = attrib1;

    gl_Position = mvp_mat * vec4(model_position, 1);
    position = gl_Position.xyz;
    normal = (mv_mat * vec4(model_normal, 0)).xyz;
}
