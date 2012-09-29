uniform sampler2D	buffer; // Data texture
uniform vec2		pixel;	// Size of a pixel in [0,0]-[1,0]
uniform sampler2D	tex;	// Refraction texture (the image to be warped)

varying vec2		uv;		// Texture coordinate

void main( void )
{
	// Calculate refraction
	vec3 above		= texture2D( buffer, uv + vec2( 0.0, -pixel.y ) ).rgb;
	float x			= above.g - texture2D( buffer, uv + vec2( pixel.x, 0.0 ) ).g;
	float y			= above.r - texture2D( buffer, uv + vec2( 0.0, pixel.y ) ).r;

	// Sample the texture from the target position
	gl_FragColor	= texture2D( tex, uv + vec2( x, y ) );
}
