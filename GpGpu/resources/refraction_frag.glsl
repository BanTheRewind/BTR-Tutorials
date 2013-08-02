uniform vec2		pixel;		 // Size of a pixel in [0,0]-[1,0]
uniform sampler2D	texBuffer;   // Data texture
uniform sampler2D	texRefract;  // Refraction texture (the image to be warped)

varying vec2		uv;			 // Texture coordinate

void main( void )
{
	// Calculate refraction
	vec2 above		= texture2D( texBuffer, uv + vec2( 0.0, -pixel.y ) ).rg;
	float x			= above.g - texture2D( texBuffer, uv + vec2( pixel.x, 0.0 ) ).g;
	float y			= above.r - texture2D( texBuffer, uv + vec2( 0.0, pixel.y ) ).r;

	// Sample the texture from the target position
	gl_FragColor	= texture2D( texRefract, uv + vec2( x, y ) );
}
