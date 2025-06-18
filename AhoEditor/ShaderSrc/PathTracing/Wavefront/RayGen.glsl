#type compute
#version 460


layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main() {
	ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
	ivec2 screen_size = textureSize(u_Scene, 0);
	if (coords.x >= screen_size.x || coords.y >= screen_size.y)
		return;

    vec4 L = texelFetch(u_Scene, coords, 0).rgba;

    imageStore(outputImage, coords, L);
    return;
}