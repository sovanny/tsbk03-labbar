#version 150

//in vec3 in_Color;
in vec3 in_Position;
in vec3 in_Normal;
in vec2 in_TexCoord;
uniform mat4 matrix;

out vec4 g_color;
const vec3 lightDir = normalize(vec3(0.3, 0.5, 1.0));

uniform mat4 rotation0;
uniform mat4 rotation1;

uniform mat4 M0;
uniform mat4 M1;

uniform vec3 pos0;
uniform vec3 pos1;



// Uppgift 3: Soft-skinning p� GPU
//
// Flytta �ver din implementation av soft skinning fr�n CPU-sidan
// till vertexshadern. Mer info finns p� hemsidan.

void main(void)
{

        //First bone
        mat4 transInv0 = mat4(1, 0, 0, -pos0.x,
                           0, 1, 0, -pos0.y,
                           0, 0, 1, -pos0.z,
                           0, 0, 0, 1);

        mat4 trans0 = mat4(1, 0, 0, pos0.x,
                           0, 1, 0, pos0.y,
                           0, 0, 1, pos0.z,
                           0, 0, 0, 1);
        mat4 Mbone0 =  trans0 * rotation0 *  inverse(trans0) ;
        //mat4 Mbone0 = rotation0;

        //Second bone
        mat4 transInv1 = mat4(1, 0, 0, -pos1.x,
                           0, 1, 0, -pos1.y,
                           0, 0, 1, -pos1.z,
                           0, 0, 0, 1);

        mat4 trans1 = mat4(1, 0, 0, pos1.x,
                           0, 1, 0, pos1.y,
                           0, 0, 1, pos1.z,
                           0, 0, 0, 1);

         mat4 Mbone1 = trans1 * rotation1 * inverse(trans1)   ;
        //mat4 Mbone1 = rotation1 * transInv1  * trans1 ;

	// transformera resultatet med ModelView- och Projection-matriserna
	vec4 vm0 = in_TexCoord.x * M0 * vec4(in_Position, 1.0);
	vec4 vm1 = in_TexCoord.y * M1   * vec4(in_Position, 1.0);
	//vec4 vm0 = in_TexCoord.x * Mbone0 * vec4(in_Position, 1.0);
	//vec4 vm1 = in_TexCoord.y * Mbone1  * vec4(in_Position, 1.0);
	//vec3 vm1 = ScalarMult(MultVec3(Mbone1,vm),g_boneWeights[row][corner].x);


	//gl_Position = matrix * (vm0 + vm1);
	gl_Position = matrix * vec4(in_Position, 1.0);

	// s�tt r�d+gr�n f�rgkanal till vertex Weights
	vec4 color = vec4(in_TexCoord.x, in_TexCoord.y, 0.0, 1.0);

	// L�gg p� en enkel ljuss�ttning p� vertexarna
	float intensity = dot(in_Normal, lightDir);
	color.xyz *= intensity;

	g_color = color;
}
