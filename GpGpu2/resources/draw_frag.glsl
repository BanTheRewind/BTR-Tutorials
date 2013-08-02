uniform vec3		eyePoint;
uniform vec3		lightPos;

uniform float		Ac;
uniform float		Al;
uniform float		Aq;
uniform vec4		Ax;
uniform vec4		Dx;
uniform float		Ka;
uniform float		Kd;
uniform float		Ke;
uniform float		Ks;
uniform float		n;
uniform vec4		Sx;

varying vec4		color;
varying vec3		normal;
varying vec3		position;
varying vec2		uv;

// d	= distance to light
// Ix	= result color
// Lx	= light color
// Ax	= ambient color
// Dx	= diffuse color
// Sx	= specular color
// Ka	= ambient coefficient
// Kd	= diffuse coefficient
// Ke	= emissive coefficient
// Ks	= specular coefficient
// Ac	= constant attenuation
// Al	= linear attenuation
// Aq	= quadratic attenuation
// Att	= attenuation coefficient
// n	= shine
// N	= surface normal
// L	= light vector
// R	= reflection vector
// V	= view vector

void main( void )
{
	vec4 Ix			= color;
	
	float d			= distance( lightPos, position );
	float Att		= 1.0 / ( Ac + Al * d + Aq * ( d * d ) );
	
	vec3 N			= normal.xyz;
	vec3 L			= normalize( lightPos.xyz - position.xyz );
	vec3 R			= normalize( -reflect( L, N ) );
	vec3 V			= normalize( -eyePoint );
	
	// Phong
	vec4 Lx			= Ax * ( Ka + Ke ) * Dx + Att * ( Dx * ( Kd + Ke ) * max( dot( N, L ), 0.0 ) + Sx * ( Ks + Ke ) * pow( max( dot( R, V ), 0.0 ), n ) );
	Ix				+= Lx;
	
	gl_FragColor	= Ix;
}
