#type vertex
#version 460 core
layout(location = 0) in vec3 a_Position;

layout(std140, binding = 0) uniform CameraUBO {
	mat4 u_View;
	mat4 u_ViewInv;
	mat4 u_Projection;
	mat4 u_ProjectionInv;
	vec4 u_ViewPosition;
};

uniform mat4 u_Model;

void main() {
	gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 1.0);
}

#type fragment
#version 460 core
// layout(location = 0) out vec4 out_EntityColor;
layout(location = 0) out uint outEntity;
uniform uint u_EntityID;

void main() {
	outEntity = u_EntityID;
	// out_EntityColor = vec4(
	// 	float(u_EntityID & 0xFF) / 255.0f,              // R  
	// 	float((u_EntityID >> 8) & 0xFF) / 255.0f,       // G
	// 	float((u_EntityID >> 16) & 0xFF) / 255.0f,      // B 
	// 	float((u_EntityID >> 24) & 0xFF) / 255.0f       // A
	// );
}