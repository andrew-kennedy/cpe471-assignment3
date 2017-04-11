#version 330 core
in vec3 LightingColor;

out vec4 color;

void main()
{
    color = vec4(LightingColor, 1.0f);
}
