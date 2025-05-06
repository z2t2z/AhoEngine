#ifndef AREA_LIGHT_GLSL
#define AREA_LIGHT_GLSL

#include "../Random.glsl"
#include "../IntersectionTest.glsl"
#include "../DisneyPrincipled.glsl"

float RectangleAreaLightPdf(const AreaLight areaLight, vec3 litPos, vec3 lightPos) {
    float choosenPdf = 1.0 / float(u_LightCount.a);
    float w = areaLight.param.y, h = areaLight.param.z;
    float areaPdf = 1.0 / (w * h);
    float dist2 = dot(litPos - lightPos, litPos - lightPos);
    vec3 L = normalize(lightPos - litPos);
    float cosAlpha = dot(areaLight.normal.xyz, -L);
    if (cosAlpha <= 0.0) {
        return 0.0;
    }
    float lightPdf = choosenPdf * dist2 / (cosAlpha * areaPdf); 
    return lightPdf;
}

vec3 SampleRectangleAreaLight(inout State state, vec3 V, const AreaLight areaLight, float choosenPdf) {
    float u0 = rand(), u1 = rand();
    float w = areaLight.param.y, h = areaLight.param.z;
    vec4 localPos = vec4((u0 - 0.5) * w, (u1 - 0.5) * h, 0.0, 1.0);
    vec3 samplePos = (areaLight.transform * localPos).xyz;
    
    vec3 L = normalize(samplePos - state.pos);

    // Only support one-sided for now
    float cosAlpha = dot(areaLight.normal.xyz, -L);
    if (cosAlpha <= 0.0) {
        return vec3(0.0);
    }

    float cosTheta = dot(state.N, L);

    if (cosTheta <= 0.0 || !VisibilityTest(state.pos, samplePos)) {
        return vec3(0.0);
    }

    mat3 tbn = ConstructTBN(state.N);
    L = WorldToLocal(L, tbn);
    V = WorldToLocal(V, tbn);
    float bsdfPdf = 0.0;
    vec3 f = principled_eval(state, V, L, bsdfPdf);

    if (bsdfPdf <= 0.0) {
        return vec3(0.0);
    }

    float areaPdf = 1.0 / (w * h);
    float dist2 = dot(samplePos - state.pos, samplePos - state.pos);
    float lightPdf = choosenPdf * dist2 / (cosAlpha * areaPdf); // https://pbr-book.org/3ed-2018/Light_Transport_I_Surface_Reflection/Sampling_Light_Sources#Shape::Pdf
    
    float misWeight = PowerHeuristicPdf(lightPdf, bsdfPdf);
    
    if (misWeight <= 0.0) {
        return vec3(0.0);
    }

    vec3 Lemit = areaLight.color.rgb * areaLight.color.w; 
    return misWeight * Lemit * f / lightPdf; 
}

vec3 SampleDiskAreaLight(inout State state, vec3 V, const AreaLight areaLight) {
    return vec3(0.0);
}

vec3 SampleSphereAreaLight(inout State state, vec3 V, const AreaLight areaLight) {
    return vec3(0.0);
}

#endif