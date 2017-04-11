#version 330 core

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

out vec4 color;

in vec3 FragPos;
in vec3 Normal;

// lighting information
uniform Material u_material;
uniform Light u_light;
uniform vec3 u_viewPos;

void main()
{
    // Ambient
    vec3 ambient = u_material.ambient * u_light.ambient;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(u_light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = (diff * u_material.diffuse) * u_light.diffuse;

    // Specular
    vec3 viewDir = normalize(u_viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_material.shininess);
    vec3 specular = (u_material.specular * spec) * u_light.specular;

    vec3 result = (specular + diffuse + ambient);
    color = vec4(result, 1.0f);
}
