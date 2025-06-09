#type compute
#version 460

layout(binding = 0, r32f) uniform writeonly image2D depthPyramid;
uniform sampler2D u_PrevMipDepth;
uniform int u_CurrMipLevel;
uniform uint u_MinMaxMode = 0; // 0: min, 1: max

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main() {
	ivec2 coord = ivec2(gl_GlobalInvocationID.xy);
	ivec2 siz = imageSize(depthPyramid);
	if (coord.x >= siz.x || coord.y >= siz.y)
		return;
	ivec2 uv0 = coord * 2;
	ivec2 uv1 = uv0 + ivec2(1, 0);
	ivec2 uv2 = uv0 + ivec2(0, 1);
	ivec2 uv3 = uv0 + ivec2(1, 1);

	float d = 0.0;
	if (u_CurrMipLevel == 0) {
		d = texelFetch(u_PrevMipDepth, coord, 0).r; // simply copy
	} else {
		float d0 = texelFetch(u_PrevMipDepth, uv0, u_CurrMipLevel - 1).r;
		float d1 = texelFetch(u_PrevMipDepth, uv1, u_CurrMipLevel - 1).r;
		float d2 = texelFetch(u_PrevMipDepth, uv2, u_CurrMipLevel - 1).r;
		float d3 = texelFetch(u_PrevMipDepth, uv3, u_CurrMipLevel - 1).r;
		if (u_MinMaxMode == 0) 
			d = min(d0, min(d1, min(d2, d3)));
		else 
			d = max(d0, max(d1, max(d2, d3)));
	}

    imageStore(depthPyramid, coord, vec4(d, 0, 0, 0));
}