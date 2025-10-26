#version 330 core	     // Minimal GL version support expected from the GPU

uniform vec3 camPos;
uniform vec3 lColor;
uniform vec3 lightPos;
uniform vec3 emission;
in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoord;
out vec4 color;	  // Shader output: the color response attached to this fragment

struct Material {
	sampler2D albedoTex;
};
uniform Material material;

void main() {
	vec3 texColor = texture(material.albedoTex, fTexCoord).rgb;
	vec3 n = normalize(fNormal);
	vec3 l = normalize(lightPos - fPosition); // light direction vector
	vec3 viewV = normalize(camPos - fPosition);
	vec3 refV = normalize(reflect(-l, n));
	vec3 ambient = lColor;
	vec3 diffuse = max(dot(n, l), 0.0) * lColor;
	vec3 specular = pow(max(dot(viewV, refV), 0.0), 32) *  lColor;
	vec3 finalColor = (ambient + diffuse) * texColor + specular + emission;
	color = vec4(finalColor, 1.0); // build an RGBA from an RGB
	//color = vec4(n, 1.0);
}
