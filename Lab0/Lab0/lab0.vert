/*
 * random comment here
 * makes syntax highlight appaer
 * colors like springs sprouts
 */

#version 150

in  vec3  in_Position;
in  vec3  in_Normal;
in  vec2  in_TexCoord;

uniform mat4 projectionMatrix;
uniform mat4 worldViewMatrix;
uniform mat4 viewMatrix;
uniform mat3 normalMat;
uniform vec3 lightPos;
uniform float u_time;
const int mode = 2;

out float shade;

out vec4 forFragColor;

out vec2 frag_texcoord;


const vec3 diffuseColor = vec3(0.5, 0.0, 0.0);
const vec3 specColor = vec3(1.0, 1.0, 1.0);
// const vec3 lightPos = vec3(1.0, 1.0, 1.0);

void main(void)
{
	shade = (mat3(viewMatrix)*in_Normal).y; // Fake shading
	// float newY = in_Position.y;
	// if(newY > 0){
	// 	newY +=  (sin(u_time*0.1)+1)*0.5;
	// }
	// vec3 newPosition = vec3(in_Position.x, newY, in_Position.z);
	vec3 newPosition = in_Position;

	gl_Position=projectionMatrix*viewMatrix*vec4(newPosition, 1.0);

	// all following gemetric computations are performed in the
	// camera coordinate system (aka eye coordinates)
	vec3 normal = normalMat * in_Normal;
	vec4 vertPos4 = worldViewMatrix * vec4(newPosition, 1.0);
	vec3 vertPos = vec3(vertPos4) / vertPos4.w;
	vec3 lightDir = normalize(lightPos - vertPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	vec3 viewDir = normalize(-vertPos);

	float lambertian = max(dot(lightDir,normal), 0.0);
	float specular = 0.0;

	if(lambertian > 0.0) {
		float specAngle = max(dot(reflectDir, viewDir), 0.0);
		specular = pow(specAngle, 4.0);

		// the exponent controls the shininess (try mode 2)
		if(mode == 2)  specular = pow(specAngle, 16.0);

		// according to the rendering equation we would need to multiply
// 		// with the the "lambertian", but this has little visual effect
// 		if(mode == 3) specular *= lambertian;
// 		// switch to mode 4 to turn off the specular component
// 		if(mode == 4) specular *= 0.0;
	}
	vec3 temp = specular*specColor;
	vec3 hans = lambertian*diffuseColor;
	vec3 greta = hans+temp;
	forFragColor = vec4(greta, 1.0);

	frag_texcoord=in_TexCoord;

}
