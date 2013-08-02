uniform vec4		color;
uniform sampler2D	tex;

varying vec2		uv;
void main ( void )
{
	gl_FragColor = vec4( vec3( 0.5 ), 1.0 ) + texture2D( tex, uv ) * color;
}
 