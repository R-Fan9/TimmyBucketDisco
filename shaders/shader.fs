#version 330 core

struct Material {
    sampler2D diffuse;
};

struct Light {
    vec3 position;
    vec3 direction;

    float cutOff;

    vec3 ambient;
    vec3 diffuse;

    float constant;
    float linear;
    float quadratic;
};

#define NR_LIGHTS 3 

in vec3 FragPos;
in vec3 Normal;
in vec2 UV;

out vec4 FragColor;

uniform Material material;
uniform Light lights[NR_LIGHTS];

vec3 CalcSpotLight(Light light, vec3 normal, vec3 fragPos);

void main()
{
    vec3 norm = normalize(Normal);
    vec3 result = vec3(0.0);

    for(int i = 0; i < NR_LIGHTS; i++){
        result += CalcSpotLight(lights[i], norm, FragPos);
    }

    FragColor = vec4(result, 1.0);
}

vec3 CalcSpotLight(Light light, vec3 normal, vec3 fragPos)
{
    vec3 lightDir = normalize(light.position - fragPos);

    float theta = dot(lightDir, normalize(-light.direction));

    if(theta > light.cutOff){
        // ambient
        vec3 ambient = light.ambient * texture(material.diffuse, UV).rgb;

        // diffuse
        vec3 norm = normalize(Normal);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = light.diffuse * diff * texture(material.diffuse, UV).rgb;

        // attenuation
        float distance = length(light.position - FragPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

        diffuse *= attenuation;
        return (ambient + diffuse);
    }

    return (light.ambient * texture(material.diffuse, UV).rgb);
}