#version 330 core
out vec4 FragColor;

struct PointLight
{
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear; 
    float quadratic;
    int source; 
};

in vec3 Normal;
in vec3 FragPos;
uniform PointLight pointLight;
uniform vec3 viewPos;

uniform PointLight pointLights[10];
uniform int numPointLights;
uniform vec4 objectColor; // base color
uniform float shine; // Shineness
uniform int glow; //Glowing

void main()
{
    //Totaling light sources
    vec3 totalLight = vec3(0.0);
    vec3 n = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    for (int i = 0; i < numPointLights; i++)
    {
        if(pointLights[i].source == 0) continue;
        if(pointLights[i].source == 1);
        //Ambient light
        vec3 ambient = pointLights[i].ambient * objectColor.rgb;

        // Diffuse light
        vec3 lightDir = normalize(pointLights[i].position - FragPos);
        float diff = max(dot(n, lightDir), 0.0);
        vec3 diffuse = pointLights[i].diffuse * diff * objectColor.rgb;

        //Specular
        vec3 reflectDir = reflect(-lightDir, n);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
        vec3 specular = pointLights[i].specular*spec;

        float distance = length(pointLights[i].position - FragPos);
        float attenuation = 1.0/(pointLights[i].constant + pointLights[i].linear*distance + pointLights[i].quadratic*(distance*distance));


        totalLight += (ambient+diffuse+specular)*attenuation;;
    }

    //Adding glow
    vec3 emissive = vec3(0.0);
    if(glow == 1)
    {
        //Fresnel-style rim
        float NdV = max(dot(n, viewDir), 0.0);
        float rim = pow(1.0 - NdV, 2.0); // brighter at the edge
        vec3 core = objectColor.rgb * 3.0;

        emissive = core * (0.6 + 0.4 * rim);
    }

    vec3 finalRGB = totalLight + emissive;
    FragColor = vec4(finalRGB, objectColor.a);
}
