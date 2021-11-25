in vec2 textureCoords;
@Texture@ firstLayers;
@Texture@ nextLayers;

layout (location = 0) out vec4 firstLayersNormalized;
layout (location = 1) out vec4 nextLayersNormalized;

void main(void)
{
	vec4 firstLayersData = texture(firstLayers, textureCoords);
	vec4 nextLayersData = texture(nextLayers, textureCoords);
	
	float layersSum = firstLayersData[0] + firstLayersData[1] +
				      firstLayersData[2] + firstLayersData[3];

	layersSum += nextLayersData[0] + nextLayersData[1] +
						  nextLayersData[2] + nextLayersData[3];

	firstLayersNormalized[0] = firstLayersData[0] / layersSum;
	firstLayersNormalized[1] = firstLayersData[1] / layersSum;
	firstLayersNormalized[2] = firstLayersData[2] / layersSum;
	firstLayersNormalized[3] = firstLayersData[3] / layersSum;

	nextLayersNormalized[0] = nextLayersData[0] / layersSum;
	nextLayersNormalized[1] = nextLayersData[1] / layersSum;
	nextLayersNormalized[2] = nextLayersData[2] / layersSum;
	nextLayersNormalized[3] = nextLayersData[3] / layersSum;
}
