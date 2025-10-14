#version 330 core
out vec4 FragColor;

// 材质属性
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

// 光源结构
struct Light {
    int type;              // 0=点光源, 1=方向光, 2=聚光灯
    vec3 position;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    
    // 衰减参数
    float constant;
    float linear;
    float quadratic;
    
    // 聚光灯参数
    float cutOff;
    float outerCutOff;
};

uniform Material material;
uniform Light light;
uniform vec3 viewPos;

in vec3 FragPos;
in vec3 Normal;
in vec3 Color;

void main()
{
    // 使用传入的颜色作为材质的基本颜色
    vec3 objectColor = Color;
    
    // 环境光
    vec3 ambient = light.ambient * objectColor;
    
    // 漫反射
    vec3 norm = normalize(Normal);
    vec3 lightDir;
    
    // 根据光源类型计算光线方向
    if (light.type == 1) { // 方向光
        lightDir = normalize(-light.direction);
    } else { // 点光源或聚光灯
        lightDir = normalize(light.position - FragPos);
    }
    
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * objectColor);
    
    // 镜面反射
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);
    
    // 衰减计算（仅对点光源和聚光灯）
    float attenuation = 1.0;
    if (light.type == 0 || light.type == 2) { // 点光源或聚光灯
        float distance = length(light.position - FragPos);
        attenuation = 1.0 / (light.constant + light.linear * distance + 
                            light.quadratic * (distance * distance));
    }
    
    // 聚光灯强度（仅对聚光灯）
    float spotlightIntensity = 1.0;
    if (light.type == 2) { // 聚光灯
        float theta = dot(lightDir, normalize(-light.direction)); 
        float epsilon = light.cutOff - light.outerCutOff;
        spotlightIntensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    }
    
    // 应用衰减和聚光灯强度
    ambient  *= attenuation;
    diffuse  *= attenuation * spotlightIntensity;
    specular *= attenuation * spotlightIntensity;
    
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}