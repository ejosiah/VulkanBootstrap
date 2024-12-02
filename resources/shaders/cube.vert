#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(set = 0, binding = 0) uniform TRANSFORM {
    mat4 geomTransform;
    mat4 view;
    mat4 projection;
};

layout(location = 0) out struct {
    vec3 position;
    vec3 normal;
} vs_out;

void main(){
    vec4 worldPos = geomTransform * vec4(position, 1);

    vs_out.position = worldPos.xyz;
    vs_out.normal = normal;

    gl_Position = projection * view * worldPos;
}