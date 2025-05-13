#version 330

//out vec4 finalColor;

uniform sampler2D worldMap;

in vec2 fragTextureCoords;

void main()
{
//	gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
	gl_FragColor = texture(worldMap, fragTextureCoords);

//	finalColor = vec4(1.0, 0.0, 0.0, 1.0);
//	finalColor = texture(worldMap, positionTexture);
}


