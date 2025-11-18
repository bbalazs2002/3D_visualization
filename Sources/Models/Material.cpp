#include "../../Headers/include_all.h"

Material::~Material() {
    glDeleteTextures(1, &diffuseTex);
    diffuseTex = 0;
    glDeleteTextures(1, &specularTex);
    specularTex = 0;
    glDeleteTextures(1, &emissionTex);
    emissionTex = 0;
    glDeleteTextures(1, &normalTex);
    normalTex = 0;
}