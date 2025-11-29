struct MaterialUniforms {
    vec4 diffuseColorTex;           // xyz: diffuse color, w: has diffuse texture
    vec4 specularColorTex;          // xyz: specular color, w: has specular texture
    vec4 ambientColorEmissionTex;   // xyz: ambient color, w: has emission texture
    float shininess;
    int hasNormalTex;
};
uniform sampler2D MaterialDiffuseTex;
uniform sampler2D MaterialSpecularTex;
uniform sampler2D MaterialEmissionTex;
uniform sampler2D MaterialNormalTex;

uniform MaterialUniforms materialData;
