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
layout(std140, binding = 0) uniform UBO {
	mat4 u_View;
	mat4 u_Projection;
	vec4 u_ViewPosition;
	mat4 u_LightViewMatrix; // ortho * view
};

in vec2 v_TexCoords;

uniform sampler2D u_gPosition; 
uniform sampler2D u_gNormal;
uniform sampler2D u_gAlbedo;
uniform sampler2D u_Depth;
uniform float u_RayLength = 100.0f; 
uniform float u_Width = 1280.0f;
uniform float u_Height = 720.0f;

const int MAX_ITERATIONS = 128;
const float stepSiz = 0.04f;
const float thickNess = 0.1f;
   
float LinearDepth() {
    return 0.0f;
} 
 
vec3 ToNDC(vec3 pos, out float w) {
    vec4 p = vec4(pos, 1.0f);
    vec4 res = u_Projection * p;
    w = res.w;
    res /= w;
    return vec3(res);
}

vec2 ToUVSpace(vec3 pos) {
    vec3 ndc = ToNDC(pos);
    ndc = ndc * 0.5f + 0.5f;
    return ndc.xy;
}

float SquaredDis(vec3 lhs, vec3 rhs) {
    lhs -= rhs;
    return dot(lhs, rhs);
}

vec3 RayMarching() {
    vec3 fragPos = texture(u_gPosition, v_TexCoords).xyz;
	vec3 normal = texture(u_gNormal, v_TexCoords).rgb; 
    
    // view space
    vec3 beginPos = fragPos;  
    vec3 viewDir = fragPos;
    vec3 rayDir = normalize(reflect(viewDir, normal));
    vec3 endPos = startPos + rayDir * u_RayLength;

    // Clip space
    vec4 H0 = u_Projection * vec4(beginPos, 1.0f);
    vec4 H1 = u_Projection * vec4(endPos, 1.0f);
    float k0 = 1.0f / H0.w;
    float k1 = 1.0f / H1.w;

    // Wagaranai
    vec3 Q0 = startPos * k0;
    vec3 Q1 = endPos * k1;

    // Perspective division from Clip space to NDC, [-1, 1]
    vec2 P0 = H0.xy * k0;
    vec2 P1 = H1.xy * k1;

    // NDC
    float w0, w1;
    vec3 V0 = ToNDC(beginPos, w0);
    vec3 V1 = ToNDC(endPos, w1);
    float k0 = 1.0f / w0;
    float k1 = 1.0f / w1;

    // UV space
    vec2 H0 = vec2(V0 * 0.5f + vec3(0.5f));
    vec2 H1 = vec2(V1 * 0.5f + vec3(0.5f));
    float dis = SquaredDis(H0, H1);
    float pixelSiz = 1.0f / min(u_Width, u_Height);
    if (dis < pixelSiz * pixelSiz) {
        H1 += vec2(pixelSiz); // cover at least one pixel, avoid degenerated line
    }

    // perspective correction
    vec3 Q0 = V0 * k0;
    vec3 Q1 = V0 * k1;
    vec2 P0 = H0 * k0; 
    vec2 P1 = H1 * k1;

    vec2 delta = P1 - P0;
    bool permute = false;
    if (abs(delta.x) < abs(delta.y)) { 
        // This is a more-vertical line
        permute = true; delta = delta.yx; P0 = P0.yx; P1 = P1.yx; 
    }    
    float stepDir = sign(delta.x);
    float invdx = stepDir / delta.x;




}
  
void main() { 
    // Calculations are inside view space
	vec3 fragPos = texture(u_gPosition, v_TexCoords).xyz;
	vec3 normal = texture(u_gNormal, v_TexCoords).rgb; 
    vec3 viewDir = fragPos;
    vec3 reflectDir = normalize(reflect(viewDir, normal));
    out_color = vec4(0.0f, 0.0f, 0.0f, 1.0f); 
    for (int i = 0; i < MAX_ITERATIONS; i++) {  
        vec3 nxtPos = fragPos + reflectDir * stepSiz * i; 
        vec3 ndc = ToNDC(nxtPos);
        vec2 uv = ndc.xy * 0.5f + 0.5f;
        float sampleDepth = textureLod(u_gPosition, uv, 0).z;
        if (sampleDepth > nxtPos.z && sampleDepth < nxtPos.z + thickNess) {
            out_color = vec4(texture(u_gAlbedo, uv).rgb, 1.0f);
            break;
        } 
    }
}