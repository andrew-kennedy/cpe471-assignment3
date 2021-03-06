#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec3 Normal;

uniform mat3 u_nRotation;
uniform mat3 u_normal;
uniform mat4 u_rotation;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    mat3 normalTrans = mat3(transpose(inverse(model)));
    gl_Position = projection * view * u_rotation * model * vec4(position, 1.0f);
    Normal = u_nRotation * normalTrans * normal;
}

