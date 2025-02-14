#version 120
uniform sampler2D diffuseTexture;
uniform float d3d_SceneLuminance = 1.0;

const float exposure = 6.0;

void main(void)
{
   vec3 diffuseColor = texture2D(diffuseTexture, gl_TexCoord[0].st).rgb;
   
   float luminance = ( diffuseColor.r * 0.299f + diffuseColor.g * 0.587f + diffuseColor.b * 0.114f );

   float brightness = ( 1.0f - exp(-luminance * exposure) );
   float scale = d3d_SceneLuminance * ( brightness / (luminance + 0.001f) );
  
  
   vec3 finalColor = scale * diffuseColor;
   //finalColor += 2.5 * pow(finalColor, 8.5);

   
   gl_FragColor = vec4(finalColor,  1.0);
}

