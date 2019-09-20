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
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBitangent;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragTexCoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragWorldPos;

void main() {
    mat4 invtransmodel =  transpose(inverse(myTransformVals.model));
    gl_Position = myTransformVals.mvp * inPosition;
    fragColor = inColor;
    fragTexCoord = inTexcoord;

    fragNormal = normalize((invtransmodel * inNormal).xyz);
    fragWorldPos = vec3(myTransformVals.model * inPosition);
}