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

uniform Light light;  
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

void main()
{
    if (material.useTexture){
        diffuseColor = texture(texture_diffuse1, TexCoords);
    }else {
        diffuseColor = vec4(material.diffuse, 1.0);
    }
    vec4 specularColor = texture(texture_specular1, TexCoords);
    if (specularColor == vec4(0.0)) {
        specularColor = vec4(material.specular, 1.0);
    }

    float roughnessColor = texture(texture_roughness1, TexCoords).r;
    if (roughnessColor == 0.0) {
        roughnessColor = material.shininess;
    }

    // ambient
    vec3 ambient  = light.ambient * material.ambient;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse  = light.diffuse * (diff * vec3(diffuseColor));
    
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), (roughnessColor));
    vec3 specular = light.specular * (spec * vec3(specularColor));   
        
    vec3 result = ambient + diffuse + specular;
    if (useBanding){
        float intensity = max(dot(norm, lightDir), 0.0);
        float levels = bandLevels; // number of bands
        float band = floor(intensity * levels) / levels;
        result = diffuseColor.rgb * band;
    }

    FragColor = diffuseColor * vec4(result, 1.0);
} 