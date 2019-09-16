#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (std140, binding = 0) uniform transformVals {
    mat4 mvp;
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 proj_view;
    vec3 cam_pos;
} myTransformVals;

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inTexcoord;
layout(location = 3) in vec4 inNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragTexCoord;

void main() {
    gl_Position = myTransformVals.mvp * inPosition;
    //gl_Position.z = 0.5;
    //gl_Position.w = 1.0;
    fragColor = inColor;
    fragTexCoord = inTexcoord;
}