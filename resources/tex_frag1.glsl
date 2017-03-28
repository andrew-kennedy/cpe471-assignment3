#version 330 core
uniform sampler2D texture_diffuse1;

in vec2 vTexCoord;
in vec3 vColor;

out vec4 Outcolor;

void main() {
  vec4 BaseColor = vec4(vColor.r, vColor.g, vColor.b, 1);
  vec4 texColor1 = texture(texture_diffuse1, vTexCoord);

  vec4 test = texColor1 * BaseColor;
  if (texColor1.r < 0.17) {
  discard;
  } else {
   Outcolor = test;
  }

}

