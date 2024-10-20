#type compute
#version 430

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
layout(location = 0) uniform sampler2D u_NoiseTexture;

layout(std430, binding = 0) buffer DataBuffer0 {
    uint data[];
};

layout(std430, binding = 1) buffer DataBuffer1 {
	vec4 accumulateData[];
};

struct Ray {
    vec3 Origin;
    vec3 Direction;
};

struct HitInfo {
	float HitDistance;
	int ObjectIndex;
	vec3 WorldPosition;
	vec3 WorldNormal;
};

struct Sphere {
    vec3 Position;
    float Radius;
    int MaterialIndex;
};

struct Material {
	vec3 Albedo;
	float Roughness;
	//float Metallic;
	//vec3 EmissionColor;
	//float EmissionPower;
};

uniform bool u_Accumulate;
uniform int u_FrameIndex;
uniform int u_Width;
uniform int u_Height;
uniform int u_Time;
uniform vec3 u_CamPos;
uniform mat4 u_ViewInv;
uniform mat4 u_ProjInv;

uniform Sphere u_Sphere0;
uniform Sphere u_Sphere1;

uniform Material u_Mt0;
uniform Material u_Mt1;

uint ConvertToRGBA(vec4 color) {
    uint r = uint(clamp(color.r * 255.0f, 0.0f, 255.0f));
    uint g = uint(clamp(color.g * 255.0f, 0.0f, 255.0f));
    uint b = uint(clamp(color.b * 255.0f, 0.0f, 255.0f));
	uint a = uint(clamp(color.a * 255.0f, 0.0f, 255.0f));
    return (a << 24) | (b << 16) | (g << 8) | r;
}

vec4 ConvertToVec4(uint value) {
	float r = float(value & 0xFF) / 255.0;        
	float g = float((value >> 8) & 0xFF) / 255.0; 
	float b = float((value >> 16) & 0xFF) / 255.0;
	float a = float((value >> 24) & 0xFF) / 255.0;

	vec4 color = vec4(r, g, b, a);
	return color;
}

uint PCG_Hash(uint seed) {
	uint state = seed * 747796405u + 2891336453u;
	uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	return (word >> 22u) ^ word;
	//seed = (seed ^ 61) ^ (seed >> 16);
	//seed *= 9;
	//seed = seed ^ (seed >> 4);
	//seed *= 0x27d4eb2d;
	//seed = seed ^ (seed >> 15);
	//return seed;
}

float RandomFloat(uint seed) {
	float PHI = 1.61803398874989484820459;  // �� = Golden Ratio
	uint x = gl_GlobalInvocationID.x;
	uint y = gl_GlobalInvocationID.y;
	vec2 xy = vec2(x, y);
	return fract(sin(dot(xy.xy, vec2(12.9898, 78.233))) * 43758.5453 * seed);
}

vec3 GenerateRandomVec3(uint seed) {
	//float randomValue = fract(sin(seed) * 43758.5453);
	//return vec3(randomValue * 2.0 - 0.5, randomValue * 2.0 - 0.5, randomValue * 2.0 - 0.5);
	//seed = PCG_Hash(seed);
	seed = u_Time;
	float r = RandomFloat(seed);
	seed = PCG_Hash(seed);
	float g = RandomFloat(seed);
	seed = PCG_Hash(seed);
	float b = RandomFloat(seed);
	//return normalize(vec3(r, g, b));
	return vec3(r, g, b);
}

HitInfo ClosestHit(const Ray ray, float hitDistance, int objectIndex) {
	HitInfo hitInfo;
	hitInfo.HitDistance = hitDistance;
	hitInfo.ObjectIndex = objectIndex;

	Sphere closestSphere = u_Sphere0;
	if (objectIndex == 1) {
		closestSphere = u_Sphere1;
	}

	vec3 origin = ray.Origin - closestSphere.Position;
	hitInfo.WorldPosition = origin + ray.Direction * hitDistance;
	hitInfo.WorldNormal = normalize(hitInfo.WorldPosition);

	hitInfo.WorldPosition += closestSphere.Position;

	return hitInfo;
}

HitInfo Miss(const Ray ray) {
	HitInfo hitInfo;
	hitInfo.HitDistance = -1.0f;
	return hitInfo;
}


HitInfo TraceSingleRay(Ray ray) {
	int closestSphere = -1;
	float hitDistance = 1E20;
	for (int i = 0; i < 2; i++) {
		Sphere sphere = u_Sphere0;
		if (i == 1) {
			sphere = u_Sphere1;
		}
		vec3 origin = ray.Origin - sphere.Position;

		float a = dot(ray.Direction, ray.Direction);
		float b = 2.0f * dot(origin, ray.Direction);
		float c = dot(origin, origin) - sphere.Radius * sphere.Radius;

		float discriminant = b * b - 4.0f * a * c;
		if (discriminant < 0.0f)
			continue;

		float closestT = (-b - sqrt(discriminant)) / (2.0f * a);
		if (closestT > 0.0f && closestT < hitDistance) {
			hitDistance = closestT;
			closestSphere = int(i);
		}
	}
	if (closestSphere < 0) {
		return Miss(ray);
	}
	return ClosestHit(ray, hitDistance, closestSphere);
}

Ray RayCasting(uint x, uint y) {
	Ray ray;
	ray.Origin = u_CamPos;
	// Image space to screen space
	float ssx = float(x) / float(u_Width), ssy = float(y) / float(u_Height);
	// Screen space to NDC
	vec2 ndc = vec2(ssx * 2.0f - 1.0f, ssy * 2.0f - 1.0f);
	// NDC to view space
	vec4 vs = u_ProjInv * vec4(ndc, 1.0f, 1.0f);

	vec3 rayDirection = vec3(u_ViewInv * vec4(normalize(vec3(vs) / vs.w), 0)); // World space
	ray.Direction = normalize(rayDirection);
	//vs.z = -1.0f;
	//vs.w = 0.0f;
	//// View space to world space
	//vec3 ws = vec3(u_ViewInv * vs);
	//ray.Direction = normalize(ws);
	return ray;
}

vec4 PerpixelShading(uint x, uint y) {
	Ray ray = RayCasting(x, y);
	vec3 color = vec3(0.0f);
	vec3 contribution = vec3(1.0f);
	float attenuation = 1.0f;
	int MAX_BOUNCE = 3;
	for (int i = 0; i < MAX_BOUNCE; i++) {
		HitInfo hitInfo = TraceSingleRay(ray);

		if (hitInfo.HitDistance < 0.0f) {
			vec3 skyColor = vec3(0.6f, 0.7f, 0.9f);
			color += skyColor * attenuation;
			break;
		}

		vec3 lightDir = normalize(vec3(-1, -1, -1));

		float lightIntensity = max(dot(hitInfo.WorldNormal, -lightDir), 0.0f); // == cos(angle)

		Sphere sphere = u_Sphere0;
		Material material = u_Mt0;
		if (hitInfo.ObjectIndex == 1) {
			sphere = u_Sphere1;
			material = u_Mt1;
		}

		contribution *= material.Albedo;
		vec3 sphereColor = material.Albedo;
		sphereColor *= lightIntensity;
		color += sphereColor * attenuation;
		attenuation *= 0.5f;

		ray.Origin = hitInfo.WorldPosition + hitInfo.WorldNormal * 0.0001f;
		//uint seed = (y * u_Width + x) * u_FrameIndex + i + u_Time;
		//uint seed = u_Time + i + y * u_Width + x;
		vec3 normal = normalize(hitInfo.WorldNormal);
		vec3 randomVec = texture(u_NoiseTexture, vec2(x, y) / vec2(256.0)).rgb;
		randomVec = normalize(randomVec);
		vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
		vec3 bitangent = cross(normal, tangent);
		mat3 TBN = mat3(tangent, bitangent, normal);
		vec3 perturbedNormalTangentSpace = vec3(0.0, 0.0, 1.0) + material.Roughness * texture(u_NoiseTexture, vec2(x, y)).xyz;
		vec3 perturbedNormalWorldSpace = normalize(TBN * perturbedNormalTangentSpace);
		ray.Direction = reflect(ray.Direction, perturbedNormalWorldSpace);
	}
	return vec4(color, 1.0f);
}

void main() {
    uint x = gl_GlobalInvocationID.x;
    uint y = gl_GlobalInvocationID.y;
	if (x < u_Width && y < u_Height) {
		vec4 color = PerpixelShading(x, y);
		uint index = y * u_Width + x;
		if (u_FrameIndex == 1) {
			accumulateData[index] = vec4(0.0f);
		}
		accumulateData[index] += color;
		vec4 nColor = accumulateData[index];
		if (u_FrameIndex > 1) {
			nColor /= u_FrameIndex;
		}
		nColor = clamp(nColor, vec4(0.0), vec4(1.0));
		data[index] = ConvertToRGBA(nColor);
		//0xFFFFFFFF white
	}
}



