#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords;

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
// transformations
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 u_rotation;
uniform mat3 u_normal;
uniform mat3 u_nRotation;

// application params
uniform vec2 u_resolution;
uniform float u_time;

// lighting information
uniform vec3 u_viewPos;
uniform Light u_light;
uniform Material u_material;

out vec3 LightingColor;

void main()
{
    mat3 normalTrans = mat3(transpose(inverse(model)));
	gl_Position = projection * view * u_rotation * model * vec4(position, 1.0);

    // Gourad shading
    // --------------
    vec3 Position = vec3(model * vec4(position, 1.0));
    vec3 Normal = u_nRotation * normalTrans * normal;

    // Ambient
    vec3 ambient = u_material.ambient * u_light.ambient;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(u_light.position - Position);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = (u_material.diffuse * diff) * u_light.diffuse;

    // Specular
    vec3 viewDir = normalize(u_viewPos - Position);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_material.shininess);
    vec3 specular = (u_material.specular * spec) * u_light.specular;

    LightingColor = ambient + diffuse + specular;
}
