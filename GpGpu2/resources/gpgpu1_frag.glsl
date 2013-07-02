uniform sampler2D	acceleration;
uniform sampler2D	offsets;
uniform sampler2D	velocities;

varying vec2		uv;

const float			dampen		= 0.98;
const float			restitution	= 0.96;
const float			speed		= 0.033;

void main ( void ) 
{
	vec3 acc			= texture2D( acceleration,	uv ).rbg * 2.0 - vec3( 1.0 );
	vec3 offset			= texture2D( offsets,		uv ).rgb * 2.0 - vec3( 1.0 );
	vec3 velocity		= texture2D( velocities,	uv ).rgb * 2.0 - vec3( 1.0 );
	
	float scale			= texture2D( offsets,		uv ).a;
	
	scale				+= length( acc ) * speed;
	velocity			+= acc * speed;
	offset				+= velocity;
	velocity			*= dampen;
	offset				*= restitution;

	gl_FragData[ 0 ] = ( vec4( offset,		scale ) + vec4( 1.0 ) ) * 0.5;
	gl_FragData[ 1 ] = ( vec4( velocity,	1.0 )	+ vec4( 1.0 ) ) * 0.5;
	gl_FragData[ 2 ] = vec4( vec3( 0.5 ),	1.0 );
}
 