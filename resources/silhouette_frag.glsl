#version 330 core
out vec4 color;

in vec3 FragPos;
in vec3 Normal;

// lighting information
uniform vec3 u_viewPosition;

void main()
{
    vec3 white = vec3(1.0);
    vec3 black = vec3(0.0);

    vec3 viewDir = vec3(normalize(u_viewPosition - FragPos));

    if (dot(Normal, viewDir) <= 0.200) {
        color = vec4(black, 1.0);
    } else {
        color = vec4(white, 1.0);
    }
}

