#version 460

layout(set = 0, binding = 1) uniform sampler2D colorTex;

layout(location = 0) in struct {
    vec3 position;
    vec3 normal;
    vec2 uv;
} fs_in;

layout(location = 0) out vec4 fragCoord;

void main() {
     vec3 color = abs(fs_in.normal);
//    vec3 color = texture(colorTex, fs_in.uv).rgb;
//    vec3 color = vec3(fs_in.uv, 0);
    fragCoord = vec4(color, 1);
}