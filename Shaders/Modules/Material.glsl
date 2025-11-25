struct Material {
    vec3 diffuseColor;
    float shininess;
    vec3 specularColor;
    int hasDiffuseTex;
    vec3 ambientColor;
    int hasSpecularTex;
    int hasEmissionTex;
    int hasNormalTex;
};
sampler2D diffuseTex;
sampler2D specularTex;
sampler2D emissionTex;
sampler2D normalTex;