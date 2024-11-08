#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoords;

out vec2 v_TexCoords;

void main() {
	gl_Position = vec4(a_Position, 1.0f);
	v_TexCoords = a_TexCoords;
}

#type fragment
#version 460 core

layout(location = 0) out vec4 out_color;
layout(std140, binding = 0) uniform CameraUBO {
	mat4 u_View;
	mat4 u_ViewInv;
	mat4 u_Projection;
	mat4 u_ProjectionInv;
	vec4 u_ViewPosition;
};

in vec2 v_TexCoords;

uniform sampler2D u_gPosition; 
uniform sampler2D u_gNormal;
uniform sampler2D u_gAlbedo;
uniform sampler2D u_gDepth;

uniform int u_Width;
uniform int u_Height;
uniform int u_MipLevelMax;
uniform float u_Near = 1000.0f;
uniform float u_Far = 0.1f;
uniform float u_MaxDisance = 100.0f; 

const int MAX_ITERATIONS = 100;
const float stepSiz = 0.04f;
const float thickNess = 0.1f;
   
float LinearEyeDepth(float z) {
    // z = z * 2.0f - 1.0f;
    return (2.0f * u_Near * u_Far) / (u_Far + u_Near - z * (u_Far - u_Near));
} 

vec4 ViewSpaceToClipSpace(vec3 pos) {
    vec4 p = vec4(pos, 1.0f);
    vec4 res = u_Projection * p;
    return res;
}

vec3 ViewSpaceToNDC(vec3 pos) {
    vec4 res = ViewSpaceToClipSpace(pos);
    res /= res.w;
    return vec3(res);
}

void swap(out float lhs, out float rhs) {
    float temp = lhs;
    lhs = rhs;
    rhs = temp;
}

float DistanceSquared(vec2 lhs, vec2 rhs) {
    lhs -= rhs;
    return dot(lhs, lhs);
}

bool Inside(vec2 uv) {
    return uv.x > 0.0f && uv.y > 0.0f && uv.x < 1.0f && uv.y < 1.0f;
}

float dither[16] = {
    0.1, 0.5, 0.125, 0.625,
    0.75, 0.25, 0.875, 0.375,
    0.187, 0.687, 0.0625, 0.562,
    0.937, 0.437, 0.812, 0.312
};
// DDA
vec3 RayMarching() {
    // View space
    vec3 beginPos = texture(u_gPosition, v_TexCoords).xyz;

	vec3 normal = texture(u_gNormal, v_TexCoords).xyz;
    mat3 normalMatrix4 = transpose(inverse(mat3(u_View)));
    normal = normalMatrix4 * normal; // To view space

    normal = normalize(normal);   

    vec3 viewDir = normalize(beginPos);
    vec3 rayDir = normalize(reflect(viewDir, normal));
    vec3 endPos = beginPos + rayDir * u_MaxDisance;

    // Clip space
    vec4 H0 = ViewSpaceToClipSpace(beginPos);
    vec4 H1 = ViewSpaceToClipSpace(endPos);
    float k0 = 1.0f / H0.w;
    float k1 = 1.0f / H1.w;

    // Perspective correction
    vec3 Q0 = beginPos / H0.w;
    vec3 Q1 = endPos / H1.w;

    // NDC
    vec2 P0 = H0.xy / H0.w;
    vec2 P1 = H1.xy / H1.w;

    // Screen space
    vec2 screenSize = vec2(u_Width, u_Height);
    P0 = (P0 * 0.5 + 0.5) * screenSize;
    P1 = (P1 * 0.5 + 0.5) * screenSize;
    
    // To avoid line degeneration
    P1 += vec2((DistanceSquared(P0, P1) < 0.0001) ? 0.01 : 0.0);

    // Permute the direction for DDA
    vec2 delta = P1 - P0;
    bool permute = false;
    if (abs(delta.x) < abs(delta.y)) {
        permute = true;
        delta = delta.yx;
        P0 = P0.yx;
        P1 = P1.yx;
    }

    float stepDir = sign(delta.x);
    float invdx = stepDir / delta.x;

    // Track the derivatives of Q and k
    vec3 dQ = (Q1 - Q0) * invdx;
    float dk = (k1 - k0) * invdx;
    vec2 dP = vec2(stepDir, delta.y * invdx);

    float stride = 1.0f;
    dP *= stride; 
    dQ *= stride;
    dk *= stride;

    vec2 P = P0;
    vec3 Q = Q0;
    float k = k0;
    float endX = P1.x * stepDir;
    float prevZMaxEstimate = beginPos.z;
    float rayZMin = prevZMaxEstimate, rayZMax = prevZMaxEstimate;
    float sceneZMax = rayZMax + 100;

    vec3 result = vec3(0.0f);
    for (int i = 0; i < MAX_ITERATIONS && P.x < endX; i++) {
        P += dP;
        Q.z += dQ.z;
        k += dk;

        rayZMin = prevZMaxEstimate;
        float rayZMax = (Q.z + dQ.z * 0.5f) / (k + dk * 0.5f);
        if (rayZMin > rayZMax) {
            swap(rayZMin, rayZMax);
        }
        prevZMaxEstimate = rayZMax;

        float rayDepth = Q.z / k;
        vec2 hitPixel = permute ? P.yx : P;
        float sampleDepth = texelFetch(u_gDepth, ivec2(hitPixel), 0).r;
        if (rayDepth < sampleDepth && rayDepth + thickNess > sampleDepth) {
            result = texelFetch(u_gAlbedo, ivec2(hitPixel), 0).rgb;
            break;
        }

        // result = texelFetch(u_gAlbedo, ivec2(hitPixel), 0).rgb;

        // if (hitPixel.x < 0 || hitPixel.x > u_Width || hitPixel.y < 0 || hitPixel.y > u_Height) {
        //     break;
        // }
        // float sampleDepth = texelFetch(u_gDepth, ivec2(hitPixel), 0).r;
        // sampleDepth = sampleDepth;
        
        // bool isBehind = (rayZMin + 0.1f <= sampleDepth);
        // if (isBehind && sampleDepth <= rayZMax && sampleDepth >= rayZMin) {
        //     result = texelFetch(u_gAlbedo, ivec2(hitPixel), 0).rgb;
        //     break;
        // }
    }
    return result;
}

vec4 HiZ() {
    // View space
    vec3 beginPos = texture(u_gPosition, v_TexCoords).xyz;
    vec3 normal = texture(u_gNormal, v_TexCoords).xyz;
    mat3 normalMatrix = transpose(inverse(mat3(u_View)));
    normal = normalMatrix * normal; // To view space

    normal = normalize(normal);
    vec3 viewDir = normalize(beginPos);
    vec3 rayDir = normalize(reflect(viewDir, normal));
    vec3 endPos = beginPos + rayDir * u_MaxDisance;

    // Clip space
    vec4 H0 = ViewSpaceToClipSpace(beginPos);
    vec4 H1 = ViewSpaceToClipSpace(endPos);
    float k0 = 1.0f / H0.w;
    float k1 = 1.0f / H1.w;

    // For interpolation
    vec3 Q0 = beginPos * k0;
    vec3 Q1 = endPos * k1;

    // NDC
    vec2 P0 = H0.xy * k0;
    vec2 P1 = H1.xy * k1;

    // Screen space
    ivec2 screenSize = textureSize(u_gDepth, 0);
    P0 = (P0 * 0.5f + 0.5f) * screenSize;
    P1 = (P1 * 0.5f + 0.5f) * screenSize;

    // To avoid line degeneration
    P1 += vec2((DistanceSquared(P0, P1) < 0.0001) ? 1.0 : 0.0);

    // Permute the direction for DDA
    vec2 delta = P1 - P0;
    bool permute = false;
    if (abs(delta.x) < abs(delta.y)) {
        permute = true;
        delta = delta.yx;
        P0 = P0.yx;
        P1 = P1.yx;
    }

    float stepDir = sign(delta.x);
    float invdx = stepDir / delta.x;

    // Track the derivatives of Q and k
    vec3 dQ = (Q1 - Q0) * invdx;
    float dk = (k1 - k0) * invdx;
    vec2 dP = vec2(stepDir, delta.y * invdx);

    float stride = 1.0f;
    dP *= stride;
    dQ *= stride;
    dk *= stride;

    vec2 P = P0;
    vec3 Q = Q0;
    float k = k0;
    float endX = P1.x * stepDir;
    float prevZMaxEstimate = beginPos.z;
    float rayZMin = prevZMaxEstimate, rayZMax = prevZMaxEstimate;
    float sceneZMax = rayZMax + 100;

    //float mipLevel = 0.0f;
    int mipLevel = 0;
    int mipLevelCount = int(log2(max(u_Width, u_Height))) + 1;

    vec3 result = vec3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < MAX_ITERATIONS && P.x < endX; i++) {
        float d = exp2(mipLevel);
        P += dP * d;
        Q.z += dQ.z * d;
        k += dk * d;

        float rayDepth = Q.z / k;
        vec2 hitPixel = permute ? P.yx : P;
        vec2 bhitPixel = hitPixel;
        hitPixel /= pow(2, mipLevel);
        float sampleDepth = texelFetch(u_gDepth, ivec2(hitPixel), mipLevel).r;

        if (rayDepth < sampleDepth) {
            if (rayDepth + thickNess > sampleDepth) {
                if (mipLevel == 0) {
                    result = texelFetch(u_gAlbedo, ivec2(bhitPixel), 0).rgb;
                    return vec4(result, 1.0f);
                    break;
                }
            }
            mipLevel = max(0, mipLevel - 1);
            P -= dP * d;
            Q.z -= dQ.z * d;
            k -= dk * d;
        }
        else {
            mipLevel = min(mipLevelCount, mipLevel + 1);
        }
    }
    return vec4(0.0f);
}
  
void main() { 
    // out_color = vec4(HiZ(), 1.0f);
    out_color = HiZ();
}
