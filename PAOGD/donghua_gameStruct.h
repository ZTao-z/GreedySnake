#ifndef GAMESTRUCT_h
#define GAMESTRUCT_h

// OTHER
#include <iostream>
#include <cstdio>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>
#include<deque>

struct Position {
	float x,y;
	float size = 0.5;
	Position(float a, float b){
		x = a;
		y = b;
	}
};

typedef struct Position Position;

enum MOVE_DIRECTION {
	UP,
	DOWN,
	LEFT,
	RIGHT
};

// 阴影着色器
const char* simpleDept_vertex = "#version 330 core\n\
								layout(location = 0) in vec3 aPos;\n\
								uniform mat4 lightSpaceMatrix;\n\
								uniform mat4 model;\n\
								void main()\n\
								{\n\
									gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);\n\
								}";
const char* simpleDept_fragment = "#version 330 core\n\
									void main()\n\
									{\n\
										 gl_FragDepth = gl_FragCoord.z;\n\
									}\0";
// 创建光照顶点着色器
const char* Phong_light_vertex = "#version 330 core\n\
									layout(location = 0) in vec3 pos;\n\
									layout (location = 1) in vec3 aNormal;\n\
									layout(location = 2) in vec2 aTexCoords;\n\
									out VS_OUT {\n\
										vec3 FragPos;\n\
										vec3 Normal;\n\
										vec2 TexCoords;\n\
										vec4 FragPosLightSpace;\n\
									} vs_out;\n\
									uniform mat4 projection;\n\
									uniform mat4 view;\n\
									uniform mat4 model;\n\
									uniform mat4 lightSpaceMatrix;\n\
									void main() {\n\
										gl_Position = projection * view * model * vec4(pos, 1.0);\n\
										vs_out.FragPos = vec3(model * vec4(pos, 1.0));\n\
										vs_out.Normal = transpose(inverse(mat3(model))) * aNormal;\n\
										vs_out.TexCoords = aTexCoords;\n\
										vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);\n\
									}\0";
// 创建光照片段着色器
const char* Phong_light_fragment = "#version 330 core\n\
									out vec4 FragColor;\n\
									in VS_OUT{\n\
										vec3 FragPos;\n\
										vec3 Normal;\n\
										vec2 TexCoords;\n\
										vec4 FragPosLightSpace;\n\
									} fs_in;\n\
									uniform vec3 openShadow;\n\
									uniform sampler2D diffuseTexture;\n\
									uniform sampler2D shadowMap;\n\
									uniform vec3 lightPos;\n\
									uniform vec3 viewPos;\n\
									float ShadowCalculation(vec4 fragPosLightSpace)\n\
									{\n\
										// perform perspective divide\n\
										vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;\n\
										// transform to [0,1] range\n\
										projCoords = projCoords * 0.5 + 0.5;\n\
										// get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)\n\
										float closestDepth = texture(shadowMap, projCoords.xy).r;\n\
										// get depth of current fragment from light's perspective\n\
										float currentDepth = projCoords.z;\n\
										// calculate bias (based on depth map resolution and slope)\n\
										vec3 normal = normalize(fs_in.Normal);\n\
										vec3 lightDir = normalize(lightPos - fs_in.FragPos);\n\
										float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);\n\
										// PCF\n\
										float shadow = 0.0; \n\
										vec2 texelSize = 1.0 / textureSize(shadowMap, 0);\n\
										for (int x = -1; x <= 1; ++x)\n\
										{\n\
											for (int y = -1; y <= 1; ++y)\n\
											{\n\
												float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;\n\
												shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;\n\
											}\n\
										}\n\
										shadow /= 9.0;\n\
										// keep the shadow at 0.0 when outside the far_plane region of the light's frustum.\n\
										if (projCoords.z > 1.0)\n\
											shadow = 0.0;\n\
										return shadow;\n\
									}\n\
									void main()\n\
									{\n\
										vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;\n\
										vec3 normal = normalize(fs_in.Normal);\n\
										vec3 lightColor = vec3(0.3);\n\
										// ambient\n\
										vec3 ambient = 0.5 * color; \n\
										// diffuse\n\
										vec3 lightDir = normalize(lightPos - fs_in.FragPos);\n\
										float diff = max(dot(lightDir, normal), 0.0);\n\
										vec3 diffuse = diff * lightColor;\n\
										// specular\n\
										vec3 viewDir = normalize(viewPos - fs_in.FragPos);\n\
										vec3 reflectDir = reflect(-lightDir, normal);\n\
										float spec = 0.0;\n\
										vec3 halfwayDir = normalize(lightDir + viewDir);\n\
										spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);\n\
										vec3 specular = spec * lightColor;\n\
										// calculate shadow\n\
										float shadow = openShadow.x > 0? ShadowCalculation(fs_in.FragPosLightSpace) : 0.0f; \n\
										shadow = min(shadow, 0.75); \n\
										vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color; \n\
										FragColor = vec4(lighting, 1.0); \n\
									};";

#endif // !HW7_h