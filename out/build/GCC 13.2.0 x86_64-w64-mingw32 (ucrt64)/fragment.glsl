#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 lightDir;
uniform vec4 objectColor; // base color
void main()
{
    // Normalize normal and direction
    vec3 n = normalize(Normal);
    float diff = max(dot(n, -normalize(lightDir)), 0.0);

    //Cel Shading threshold
    float intensity = 0.0;
    if (diff > 0.9)
        intensity = 1.0;
    else if (diff > 0.5)
        intensity = 0.6;
    else if (diff > 0.1)
        intensity = 0.3;
    else
        intensity = 0.05;

    FragColor = vec4(intensity * objectColor.rgb, objectColor.a);
}
