#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (std140, binding = 0) uniform bufferVals {
    mat4 mvp;
} myBufferVals;

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inTexcoord;
layout(location = 3) in vec4 inNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragTexCoord;

void main() {
    gl_Position = myBufferVals.mvp * inPosition;
    //gl_Position.z = 0.5;
    //gl_Position.w = 1.0;
    fragColor = inColor;
    fragTexCoord = inTexcoord;
}