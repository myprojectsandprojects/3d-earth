#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

in vec3 fragMyPosition;

// Input uniform values
uniform sampler2D texture0;
uniform sampler2D worldMap;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

// NOTE: Add here your custom variables

#define     MAX_LIGHTS              4
#define     LIGHT_DIRECTIONAL       0
#define     LIGHT_POINT             1

struct Light {
    int enabled;
    int type;
    vec3 position;
    vec3 target;
    vec4 color;
};

// Input lighting values
uniform Light lights[MAX_LIGHTS];
uniform vec4 ambient;
uniform vec3 viewPos;

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(worldMap, fragTexCoord);
//    vec4 texelColor = vec4(0.0, 0.6, 0.0, 1.0);

////	vec4 texelColor = mix(vec4(0.0, 1.0, 0.0, 1.0), vec4(0.7, 0.5, 0.2, 1.0), ((fragMyPosition.y + 1.0) / 2.0));
//	float distanceFromCenter = sqrt(fragMyPosition.x * fragMyPosition.x + fragMyPosition.y * fragMyPosition.y + fragMyPosition.z * fragMyPosition.z);
////	vec4 texelColor = mix(vec4(0.0, 1.0, 0.0, 1.0), vec4(0.7, 0.5, 0.2, 1.0), clamp((distanceFromCenter - 0.9) / 0.05, 0.0, 1.0));
//	vec4 texelColor = mix(vec4(0.0, 1.0, 0.0, 1.0), vec4(0.7, 0.5, 0.2, 1.0), clamp((distanceFromCenter - 0.9) / 0.2 + 0.0, 0.0, 1.0));

    vec3 lightDot = vec3(0.0);
    vec3 normal = normalize(fragNormal);
    vec3 viewD = normalize(viewPos - fragPosition);
    vec3 specular = vec3(0.0);

    vec4 tint = colDiffuse * fragColor;

    // NOTE: Implement here your fragment shader code

    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (lights[i].enabled == 1)
        {
            vec3 light = vec3(0.0);

            if (lights[i].type == LIGHT_DIRECTIONAL)
            {
                light = -normalize(lights[i].target - lights[i].position);
            }

            if (lights[i].type == LIGHT_POINT)
            {
                light = normalize(lights[i].position - fragPosition);
            }

            float NdotL = max(dot(normal, light), 0.0);
            lightDot += lights[i].color.rgb*NdotL;

            float specCo = 0.0;
            if (NdotL > 0.0) specCo = pow(max(0.0, dot(viewD, reflect(-(light), normal))), 16.0); // 16 refers to shine
            specular += specCo;
        }
    }

    finalColor = (texelColor*((tint + vec4(specular, 1.0))*vec4(lightDot, 1.0)));
    finalColor += texelColor*(ambient/10.0)*tint;
//		finalColor = vec4(lightDot / 4.0, 1.0);
//		finalColor = vec4(lightDot, 1.0);

    // Gamma correction
    finalColor = pow(finalColor, vec4(1.0/2.2));
}
