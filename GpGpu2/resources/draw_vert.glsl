#extension GL_ARB_draw_instanced	: enable
#extension GL_EXT_gpu_shader4		: enable

uniform sampler2D	offsets;
uniform vec2		size;
uniform float		t;

varying vec4		color;
varying vec3		normal;
varying vec3		position;
varying vec2		uv;

void main( void ) 
{
	color		= gl_Color;
	normal		= gl_NormalMatrix * gl_Normal;
	position	= gl_Vertex.xyz;
	uv			= gl_MultiTexCoord0.st;
	
	float x		= float( mod( float( gl_InstanceID ), size.x ) );
	float y		= float( floor( float( gl_InstanceID ) / size.x ) );
	vec2 loc	= vec2( x, y ) / size;
	
	vec3 offset = vec3( 0.0 );
	offset.x	= x * 2.0 - size.x;
	offset.y	= sin( loc.s * 10.0 + t ) * 10.0;
	offset.z	= y * 2.0 - size.y;

	offset		+= texture2D( offsets, loc ).rgb * 2.0 - vec3( 1.0 );
	offset.y	*= 1.5;
	float scale	= 1.0 + cos( loc.t * t * 0.333 ) * 0.5;
	
	vec4 pos	= gl_Vertex * scale + vec4( offset, 0.0 );
	pos			= gl_ModelViewProjectionMatrix * pos;
	position	= pos.xyz;
	
	color		= gl_Color;
	gl_Position = pos;
}
