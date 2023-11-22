#version 330 core

in vec3 Normal;
in vec2 UV;
in vec3 FragPos;

out vec4 FragColor;

uniform sampler2D myTexture;
uniform vec3 lightPos;

void main()
{
    // ambient color
    vec3 ambient = vec3(0.2, 0.2, 0.2);

    // compute attenuation
    float kc = 1.0;
    float kl = 0.35e-4;
    float kq = 0.44e-4;
    float distance = length(lightPos - FragPos);
    float attenuation = 1.0 / (kc + kl * distance + kq * distance * distance);

    // diffuse color
    vec3 diffuse = attenuation * vec3(1.0, 1.0, 1.0);

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);

    vec4 objectColor = texture(myTexture, UV);
    FragColor = vec4(ambient, 1.0) * objectColor + vec4(diffuse, 1.0) * objectColor * diff;
}
