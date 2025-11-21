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
uniform int renderMode; // 渲染模式 uniform

in vec3 FragPos;
in vec3 Normal;
in vec3 Color;

void main()
{
    // 根据不同的渲染模式显示不同的效果
    if (renderMode == 0) {
        // 顶点着色器结果 - 直接显示顶点颜色
        FragColor = vec4(Color, 1.0);
    } else if (renderMode == 1) {
        // 光栅化后结果 - 显示法线信息
        vec3 normalizedNormal = normalize(Normal);
        FragColor = vec4(normalizedNormal * 0.5 + 0.5, 1.0); // 将法线从[-1,1]映射到[0,1]
    } else if (renderMode == 2) {
        // 片段着色后结果 - 显示未光照计算的纯色
        FragColor = vec4(Color, 1.0);
    } else {
        // 最终处理结果 - 完整的光照计算
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
}