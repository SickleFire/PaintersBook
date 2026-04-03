#version 330 core
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
    bool useTexture;
}; 

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
#define MAX_LIGHTS 8
uniform Light lights[MAX_LIGHTS];
uniform int numLights;
uniform Material material;
uniform bool useBanding;
uniform int bandLevels;

out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;  
in vec3 FragPos;  
  
uniform vec3 viewPos;
uniform vec3 lightPos; 
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_roughness1;

vec4 diffuseColor;

vec3 CalcLight(Light light, vec3 norm, vec3 viewDir, vec3 diffuseRGB, vec3 specularRGB, float roughness){
    // ambient
    vec3 ambient  = light.ambient * material.ambient;

    // diffuse 
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse  = light.diffuse * (diff * vec3(diffuseRGB));

    // specular
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), (roughness));
    vec3 specular = light.specular * (spec * vec3(specularRGB));   

    return ambient + diffuse + specular;
}

void main()
{
    if (material.useTexture){
        diffuseColor = texture(texture_diffuse1, TexCoords);
    }else {
        diffuseColor = vec4(material.diffuse, 1.0);
    }
    vec4 specularColor = texture(texture_specular1, TexCoords);
    if (!material.useTexture) {
        specularColor = vec4(material.specular, 1.0);
    }

    float roughnessColor = texture(texture_roughness1, TexCoords).r;
    if (!material.useTexture) {
        roughnessColor = material.shininess;
    }
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = vec3(0.0);
    if (useBanding){
        float levels = bandLevels; // number of bands
        for (int i = 0; i < numLights; i++)
        {
            vec3 lightDir = normalize(lights[i].position - FragPos);
            float intensity = max(dot(norm, lightDir), 0.0);
            float band = floor(intensity * levels) / levels;
            result += diffuseColor.rgb * band;
        }
    }else {
        for (int i = 0; i < numLights; i++){
            result += CalcLight(lights[i], norm, viewDir, vec3(diffuseColor), vec3(specularColor), roughnessColor);
        }
    }

    FragColor = vec4(result, diffuseColor.a);
} 

