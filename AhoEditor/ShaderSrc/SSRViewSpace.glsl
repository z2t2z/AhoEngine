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
//uniform float u_Linear ;
//uniform float u_Quadratic;
 
uniform sampler2D u_gPosition; 
uniform sampler2D u_gNormal;
uniform sampler2D u_gAlbedo;
uniform sampler2D u_Depth;

const int MAX_ITERATIONS = 128;
const float stepSiz = 0.04f;
const float thickNess = 0.1f;
   
float LinearDepth() {
    return 0.0f;
} 
 
vec3 ToNDC(vec3 pos) {
    vec4 p = vec4(pos, 1.0f);
    vec4 res = u_Projection * p;
    res /= res.w;
    return vec3(res);
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