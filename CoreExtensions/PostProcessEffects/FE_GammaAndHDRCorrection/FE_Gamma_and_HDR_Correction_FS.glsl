in vec2 textureCoords;
uniform float FEExposure;
uniform float FEGamma;
@Texture@ inputTexture;

void main(void)
{
    vec3 hdrColor = texture(inputTexture, textureCoords).rgb;
  
    // Exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * FEExposure);
    // Gamma correction 
    mapped = pow(mapped, vec3(1.0 / FEGamma));
  
    gl_FragColor = vec4(mapped, 1.0);
}