#version 330 core
out vec4 color;

in vec3 Normal;

void main() {
  color.rgb = Normal;
  color.a = 1.0f;
}
