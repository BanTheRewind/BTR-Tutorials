// This shader simply passes the vertex position 
// and texture coordinate to the fragment shader

varying vec2 uv;

void main( void )
{
	uv			= gl_MultiTexCoord0.st;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
