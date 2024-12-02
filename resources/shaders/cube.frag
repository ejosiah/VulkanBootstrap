#version 460

layout(location = 0) in struct {
    vec3 position;
    vec3 normal;
} fs_in;

layout(location = 0) out vec4 fragCoord;

void main() {
    fragCoord = vec4(abs(fs_in.normal), 1);
}