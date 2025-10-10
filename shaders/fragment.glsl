#version 330 core
in vec2 TexCoords;
in vec4 ParticleColor;
out vec4 FragColor;

void main()
{
    // 创建圆形粒子
    float distance = length(TexCoords - vec2(0.5, 0.5));
    if (distance > 0.5) {
        discard;
    }
    
    // 可选：添加一些渐变效果
    float alpha = 1.0 - distance * 2.0;
    FragColor = vec4(ParticleColor.rgb, alpha * ParticleColor.a);
}