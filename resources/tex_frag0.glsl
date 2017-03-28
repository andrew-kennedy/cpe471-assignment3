#version 330 core
uniform sampler2D texture_diffuse1;

in vec2 vTexCoord;
in vec3 vColor;

out vec4 Outcolor;

void main() {
  vec4 BaseColor = vec4(vColor.x, vColor.y, vColor.z, 1);
  vec4 texColor0 = texture(texture_diffuse1, vTexCoord);

  Outcolor = texColor0 * BaseColor;
}

