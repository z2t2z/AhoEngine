#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

layout(std140, binding = 1) uniform GeneralUBO {
	mat4 u_View;
	mat4 u_Projection;
	vec4 u_ViewPosition;
	mat4 u_LightViewMatrix; // ortho * view

	vec4 u_LightPosition[4];
	vec4 u_LightColor[4];
	ivec4 info;
};

out vec3 v_Position;
out vec4 v_PositionLightSpace;
out vec2 v_TexCoords;
out vec3 v_ViewPosition;
out vec3 v_LightPosition[4];
out vec3 v_LightColor[4];
out vec3 v_Normal;

out vec3 LightPos;
out vec3 LightColor;

uniform mat4 u_Model;

void main() {
	gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 1.0f);
	v_Position = vec3(u_Model * vec4(a_Position, 1.0f));
	v_TexCoords = a_TexCoords;

	mat3 normalMatrix = transpose(inverse(mat3(u_Model)));
	vec3 T = normalize(normalMatrix * a_Tangent);
	vec3 N = normalize(normalMatrix * a_Normal);
	T = normalize(T - dot(N, T) * N); 
	vec3 B = cross(N, T);

	// mat3 TBN = transpose(mat3(T, B, N));
	mat3 TBN = mat3(1.0f);
	v_Normal = a_Normal; 
	for (int i = 0; i < 1; i++) {
		// vec3 lightPos = vec3(u_LightPosition[i].r, u_LightPosition[i].g, u_LightPosition[i].b);
		// v_LightPosition[i] = TBN * lightPos;
		v_LightPosition[0] = vec3(u_LightPosition[0]);
		v_LightColor[0] = vec3(u_LightColor[0]); 
		LightPos = vec3(u_LightPosition[0]);
		LightColor = vec3(u_LightColor[0]);
		// v_LightColor[i] = vec3(u_LightColor[i].r, u_LightColor[i].g, u_LightColor[i].b);
	}
	v_PositionLightSpace = u_LightViewMatrix * vec4(v_Position, 1.0f);
	v_ViewPosition = TBN * vec3(u_ViewPosition);
	v_Position = TBN * v_Position;
}  

 
#type fragment
#version 460 core 

layout(location = 0) out vec4 EntityColor;
layout(location = 1) out vec4 color;

in vec3 v_Position;
in vec4 v_PositionLightSpace;
in vec2 v_TexCoords;
in vec3 v_ViewPosition;
in vec3 v_LightPosition[4];
in vec3 v_LightColor[4];
in vec3 v_Normal;

in vec3 LightPos;
in vec3 LightColor;


uniform uint u_EntityID;
uniform sampler2D u_Diffuse;
uniform sampler2D u_Normal;
uniform sampler2D u_DepthMap;

// Blinn-Phong
void main() {
	// Ambient
	float ambientStrength = 0.01f;
	// vec3 Albedo = texture(u_Diffuse, v_TexCoords).rgb;
	vec3 Albedo = v_LightColor[0];
	Albedo = pow(Albedo, vec3(2.2f)); // To linear space
	Albedo = v_LightColor[0];
	// Albedo = vec3(0.1f, 0.1f, 0.1f);
	vec3 ambient = ambientStrength * Albedo;
 
	// Diffuse
	// vec3 normal = texture(u_Normal, v_TexCoords).rgb;
	// normal = normalize(normal * 2.0f - 1.0f);
	vec3 normal = v_Normal;
	vec3 lightPos = vec3(0.0f, 1.840f, 0.0f);
	lightPos = v_LightPosition[0];
	// lightPos = LightPos; 
	vec3 lightDir = normalize(lightPos - v_Position);
	float diff = max(dot(normal, lightDir), 0.0f);
	vec3 diffuse = diff * Albedo; 
 
	// Specular
	float specularStrength = 1.0f;
	vec3 viewDir = normalize(v_ViewPosition - v_Position);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0f), 32);
	vec3 specular = specularStrength * spec * Albedo; 
	float specStrength = 0.2f; 
	specular = specStrength * specular;   
	// Combination
	vec3 result = (ambient + diffuse + specular);
	result = pow(result, vec3(1.0f / 2.2f));
	color = vec4(result, 1.0f);    

	EntityColor = vec4(
		float(u_EntityID & 0xFF) / 255.0f,              // R  
		float((u_EntityID >> 8) & 0xFF) / 255.0f,       // G !
		float((u_EntityID >> 16) & 0xFF) / 255.0f,      // B 
		float((u_EntityID >> 24) & 0xFF) / 255.0f       // A
	);
}