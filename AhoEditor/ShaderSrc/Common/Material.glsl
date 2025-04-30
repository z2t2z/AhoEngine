#ifndef MATERIAL_GLSL
#define MATERIAL_GLSL

// Reference: disney principled 
#ifdef PATH_TRACING
    struct Material {
        vec3 baseColor;
        float subsurface;

        float metallic;
        float specular;
        float specTint;
        float roughness;

        float anisotropic;
        float sheen;
        float sheenTint;
        float clearcoat;

        float clearcoatGloss;
        float specTrans;
        float ior;
        float ax;

        float ay;
    };
#else
    struct Material {
        vec3 baseColor;
        float metallic;
        float roughness;
        float ao;
    };
#endif

#endif