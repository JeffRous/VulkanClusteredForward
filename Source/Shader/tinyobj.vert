#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (set = 0, binding = 0) uniform bufferVals {
    mat4 mvp;
} myBufferVals;

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inTexcoord;
layout(location = 3) in vec4 inNormal;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = myBufferVals.mvp * inPosition;
    fragColor = inColor;
}