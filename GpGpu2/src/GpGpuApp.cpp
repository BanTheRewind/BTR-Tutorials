#include "cinder/app/AppBasic.h"
#include "cinder/Arcball.h"
#include "cinder/Camera.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Vbo.h"
#include "cinder/params/Params.h"

#include "MeshHelper.h"

class GpGpuApp : public ci::app::AppBasic
{
public:
	void						draw();
	void						mouseDown( ci::app::MouseEvent event );
	void						mouseUp( ci::app::MouseEvent event );
	void						mouseDrag( ci::app::MouseEvent event );
	void						mouseWheel( ci::app::MouseEvent event );
	void						prepareSettings( ci::app::AppBasic::Settings* settings );
	void						resize();
	void						setup();
	void						update();
private:
	ci::Vec2i					mSize;
	ci::Vec2i					mSizePrev;
	
	ci::Arcball					mArcball;
	ci::CameraPersp				mCamera;
	ci::Vec3f					mEyePoint;
	
	float						mLightAttenuationConstant;
	float						mLightAttenuationLinear;
	float						mLightAttenuationQuadratic;
	
	ci::ColorAf					mLightAmbient;
	ci::ColorAf					mLightDiffuse;
	ci::Vec3f					mLightPosition;
	ci::ColorAf					mLightSpecular;
	float						mLightShine;
	
	float						mMaterialAmbient;
	float						mMaterialDiffuse;
	float						mMaterialEmissive;
	float						mMaterialSpecular;
	
	ci::Vec2f					mMouse;
	ci::Vec2f					mMouseVelocity;
	bool						mMouseDown;
	
	ci::gl::Fbo					mFbo[ 2 ];
	size_t						mFboIndex;
	
	void						drawInstanced( const ci::gl::VboMeshRef& vbo, size_t count );
	ci::gl::VboMeshRef			mMesh;
	
	ci::gl::GlslProgRef			mGlslProgDraw;
	ci::gl::GlslProgRef			mGlslProgGpGpu0;
	ci::gl::GlslProgRef			mGlslProgGpGpu1;
	
	float						mBrushSize;
	ci::gl::TextureRef			mTextureBrush;
	
	float						mFrameRate;
	bool						mFullScreen;
	bool						mFullScreenPrev;
	ci::params::InterfaceGlRef	mParams;
};

/////////////////////////////

#include "cinder/ImageIo.h"
#include "Resources.h"

using namespace ci;
using namespace ci::app;
using namespace std;

// Color attachments to hold scene data
static const GLenum kColorAttachments[ 3 ] = {
	GL_COLOR_ATTACHMENT0,	// Position
	GL_COLOR_ATTACHMENT1,	// Velocity
	GL_COLOR_ATTACHMENT2	// Acceleration
};

void GpGpuApp::draw()
{
	// We're going to draw new data onto the "ping" FBO,
	// using the "pong" FBO's textures as input
	size_t pong = ( mFboIndex + 1 ) % 2;
	
	// Set up OpenGL for data
	gl::disableDepthRead();
	gl::disableDepthWrite();
	gl::setViewport( mFbo[ mFboIndex ].getBounds() );
	gl::color( ColorAf::white() );
	
	// Draw any new input onto the acceleration texture
	mFbo[ pong ].bindFramebuffer();
	glDrawBuffer( GL_COLOR_ATTACHMENT2 );
	if ( mMouseDown ) {
		Vec2f fboSize	= Vec2f( mFbo[ mFboIndex ].getSize() );
		Vec2f winSize	= Vec2f( app::getWindowSize() );
		
		gl::setMatricesWindow( fboSize, true );
		
		Vec2f brushSize	= Vec2f::one() * mBrushSize * fboSize;
		Vec2f pos		= ( mMouse / winSize );
		pos.y			= 1.0f - pos.y;
		pos				*= fboSize;
		
		mGlslProgGpGpu0->bind();
		mGlslProgGpGpu0->uniform( "color", ColorAf( mMouseVelocity.x, 0.0f, 1.0f - mMouseVelocity.y, 1.0f ) );
		mGlslProgGpGpu0->uniform( "tex", 0 );
		
		gl::enable( GL_TEXTURE_2D );
		mTextureBrush->bind();
		gl::drawSolidRect( Rectf( pos - brushSize, pos + brushSize ) );
		mTextureBrush->unbind();
		gl::disable( GL_TEXTURE_2D );
		
		mGlslProgGpGpu0->unbind();
	}
	mFbo[ pong ].unbindFramebuffer();
	
	// Now let's do an update pass in GLSL
	mFbo[ mFboIndex ].bindFramebuffer();
	gl::setMatricesWindow( mFbo[ mFboIndex ].getSize(), false );
	
	// Bind the "pong" textures to use as input data
	for ( int32_t i = 0; i < 3; ++i ) {
		mFbo[ pong ].bindTexture( i, i );
	}
	
	// Set up shader to read data textures
	mGlslProgGpGpu1->bind();
	mGlslProgGpGpu1->uniform( "offsets",		0 );
	mGlslProgGpGpu1->uniform( "velocities",		1 );
	mGlslProgGpGpu1->uniform( "acceleration",	2 );
	
	// Draw a rect to process data
	glDrawBuffers( 3, kColorAttachments );
	gl::drawSolidRect( mFbo[ pong ].getBounds() );
	
	// Unbind everything
	mFbo[ pong ].unbindTexture();
	mGlslProgGpGpu1->unbind();
	mFbo[ mFboIndex ].unbindFramebuffer();
	
	// Swap FBOs
	mFboIndex = pong;
	
	/////////////////////////////////
	
	// Make sure we have data to work with before we draw geometry
	if ( mFbo[ mFboIndex ] &&
		mFbo[ mFboIndex ].getTexture( 0 ) &&
		mFbo[ mFboIndex ].getTexture( 1 ) ) {
		
		// Set up window for 3D drawing
		gl::clear( Colorf( 0.5f, 0.45f, 0.4f ) );
		gl::setViewport( getWindowBounds() );
		gl::setMatrices( mCamera );
		gl::enableDepthRead();
		gl::enableDepthWrite();
		gl::multModelView( mArcball.getQuat() );
		gl::color( ColorAf::black() );
		
		// Set up shader to render scene
		mGlslProgDraw->bind();
		mGlslProgDraw->uniform( "Ax",			mLightAmbient );
		mGlslProgDraw->uniform( "Ac",			mLightAttenuationConstant );
		mGlslProgDraw->uniform( "Al",			mLightAttenuationLinear );
		mGlslProgDraw->uniform( "Aq",			mLightAttenuationQuadratic );
		mGlslProgDraw->uniform( "Dx",			mLightDiffuse );
		mGlslProgDraw->uniform( "eyePoint",		mEyePoint );
		mGlslProgDraw->uniform( "Ka",			mMaterialAmbient );
		mGlslProgDraw->uniform( "Kd",			mMaterialDiffuse );
		mGlslProgDraw->uniform( "Ke",			mMaterialEmissive );
		mGlslProgDraw->uniform( "Ks",			mMaterialSpecular );
		mGlslProgDraw->uniform( "lightPos",		mLightPosition );
		mGlslProgDraw->uniform( "n",			mLightShine );
		mGlslProgDraw->uniform( "offsets",		0 );
		mGlslProgDraw->uniform( "projection",	mCamera.getProjectionMatrix() );
		mGlslProgDraw->uniform( "size",			Vec2f( mSize ) );
		mGlslProgDraw->uniform( "Sx",			mLightSpecular );
		mGlslProgDraw->uniform( "t",			(float)getElapsedSeconds() );
		
		// Bind textures to use as input data
		for ( int32_t i = 0; i <= 2; ++i ) {
			mFbo[ mFboIndex ].bindTexture( i, i );
		}
		
		// Draw instanced
		drawInstanced( mMesh, mSize.x * mSize.y );
		
		// Finished drawing
		mFbo[ mFboIndex ].unbindTexture();
		mGlslProgDraw->unbind();
		
		// Draw textures so we can see what's going on under the hood
		gl::setMatricesWindow( getWindowSize() );
		gl::disableDepthRead();
		gl::disableDepthWrite();
		gl::color( ColorAf::white() );
		gl::pushMatrices();
		
		float x = 20.0f;
		float y = 440.0f;
		
		float width = 64.0f;
		Area srcArea( Vec2i::zero(), mSize );
		Rectf destRect( x, y, x + width, y + width );
		
		gl::draw( mFbo[ 0 ].getTexture( 0 ), srcArea, destRect );
		destRect.x1 += width;
		destRect.x2 += width;
		gl::draw( mFbo[ 1 ].getTexture( 0 ), srcArea, destRect );
		
		destRect.y1 += width;
		destRect.y2 += width;
		destRect.x1 = x;
		destRect.x2 = x + width;
		gl::draw( mFbo[ 0 ].getTexture( 1 ), srcArea, destRect );
		destRect.x1 += width;
		destRect.x2 += width;
		gl::draw( mFbo[ 1 ].getTexture( 1 ), srcArea, destRect );
		
		destRect.y1 += width;
		destRect.y2 += width;
		destRect.x1 = x;
		destRect.x2 = x + width;
		gl::draw( mFbo[ 0 ].getTexture( 2 ), srcArea, destRect );
		destRect.x1 += width;
		destRect.x2 += width;
		gl::draw( mFbo[ 1 ].getTexture( 2 ), srcArea, destRect );
		gl::popMatrices();
	}
	
	// Draw parameters
	if ( getElapsedFrames() > 1 ) { // This condition prevents a memory leak
		mParams->draw();
	}
}

void GpGpuApp::drawInstanced( const gl::VboMeshRef &vbo, size_t count )
{
	vbo->enableClientStates();
	vbo->bindAllData();
	glDrawElementsInstancedARB( vbo->getPrimitiveType(), vbo->getNumIndices(), GL_UNSIGNED_INT, (GLvoid*)( sizeof(uint32_t) * 0 ), count );
	gl::VboMesh::unbindBuffers();
	vbo->disableClientStates();
}

void GpGpuApp::mouseDown( MouseEvent event )
{
	if ( event.isControlDown() ) {
		mArcball.mouseDown( event.getPos() );
	} else {
		mMouseVelocity	= Vec2f::zero();
		mMouse			= Vec2f( event.getPos() );
		mMouseDown		= true;
	}
}

void GpGpuApp::mouseDrag( MouseEvent event )
{
	if ( event.isControlDown() ) {
		mArcball.mouseDrag( event.getPos() );
	} else {
		Vec2f pos		= Vec2f( event.getPos() );
		mMouseVelocity	= pos - mMouse;
		mMouse			= pos;
	}
}

void GpGpuApp::mouseUp( MouseEvent event )
{
	mMouseDown = false;
}

void GpGpuApp::mouseWheel( MouseEvent event )
{
	mEyePoint.z += -event.getWheelIncrement() * 5.0f;
}

void GpGpuApp::prepareSettings( Settings* settings )
{
	settings->setFrameRate( 60.0f );
	settings->setWindowSize( 1280, 720 );
}

void GpGpuApp::resize()
{
	mCamera.setAspectRatio( getWindowAspectRatio() );
}

void GpGpuApp::setup()
{
	// Load shaders
	try {
		mGlslProgDraw = gl::GlslProg::create( loadResource( RES_GLSL_DRAW_VERT ), loadResource( RES_GLSL_DRAW_FRAG ) );
	} catch ( gl::GlslProgCompileExc ex ) {
		console() << ex.what() << "\n";
		quit();
	}
	
	try {
		mGlslProgGpGpu0 = gl::GlslProg::create( loadResource( RES_GLSL_GPGPU_VERT ), loadResource( RES_GLSL_GPGPU0_FRAG ) );
	} catch ( gl::GlslProgCompileExc ex ) {
		console() << ex.what() << "\n";
		quit();
	}
	
	try {
		mGlslProgGpGpu1 = gl::GlslProg::create( loadResource( RES_GLSL_GPGPU_VERT ), loadResource( RES_GLSL_GPGPU1_FRAG ) );
	} catch ( gl::GlslProgCompileExc ex ) {
		console() << ex.what() << "\n";
		quit();
	}
	
	// Define all properties
	mArcball					= Arcball( getWindowSize() );
	mBrushSize					= 0.1f;
	mCamera						= CameraPersp( getWindowWidth(), getWindowHeight(), 60.0f, 1.0f, 100000.0f );
	mEyePoint					= Vec3f( 0.0f, 20.0f, 256.0f );
	mFullScreen					= isFullScreen();
	mFullScreenPrev				= mFullScreen;
	mLightAmbient				= ColorAf::gray( 0.1f );
	mLightAttenuationConstant	= 0.1f;
	mLightAttenuationLinear		= 0.01f;
	mLightAttenuationQuadratic	= 0.001f;
	mLightDiffuse				= ColorAf( 0.9f, 0.3f, 0.667f );
	mLightPosition				= Vec3f( 11.38f, -1.39f, 59.74f );
	mLightSpecular				= ColorAf::white();
	mLightShine					= 1.0f;
	mMaterialAmbient			= 1.0f;
	mMaterialDiffuse			= 1.0f;
	mMaterialEmissive			= 0.0f;
	mMaterialSpecular			= 1.0f;
	mMesh						= gl::VboMesh::create( MeshHelper::createCube() );
	mMouseDown					= false;
	mMouse						= Vec2f::zero();
	mMouseVelocity				= Vec2f::zero();
	mParams						= params::InterfaceGl::create( "Params", Vec2i( 250, 400 ) );
	mSize						= Vec2i( 512, 512 );
	mSizePrev					= Vec2i::zero();
	mTextureBrush				= gl::Texture::create( loadImage( loadResource( RES_PNG_BRUSH ) ) );
	
	// Set up arcball
	mArcball.setRadius( (float)getWindowHeight() * 0.5f );
	
	// Set up parameters
	mParams->addParam( "Frame rate",		&mFrameRate,					"", true );
	mParams->addParam( "Full screen",		&mFullScreen,					"key=f" );
	mParams->addButton( "Quit",				bind( &GpGpuApp::quit, this ),	"key=q" );
	
	mParams->addSeparator( "" );
	mParams->addParam( "Brush size",		&mBrushSize,					"min=0.0 max=1.0 step=0.001" );
	mParams->addParam( "Size X",			&mSize.x,						"min=1 max=1024 step=1" );
	mParams->addParam( "Size Y",			&mSize.y,						"min=1 max=1024 step=1" );
	
	mParams->addSeparator( "" );
	mParams->addParam( "Light ambient",		&mLightAmbient );
	mParams->addParam( "Light att const",	&mLightAttenuationConstant,		"min=0.0 max=1.0 step=0.001" );
	mParams->addParam( "Light att line",	&mLightAttenuationLinear,		"min=0.0 max=1.0 step=0.0001" );
	mParams->addParam( "Light att quad",	&mLightAttenuationQuadratic,	"min=0.0 max=1.0 step=0.00001" );
	mParams->addParam( "Light diffuse",		&mLightDiffuse );
	mParams->addParam( "Light position",	&mLightPosition );
	mParams->addParam( "Light specular",	&mLightSpecular );
	mParams->addParam( "Light shine",		&mLightShine,					"min=0.0 max=100000.0 step=1.0" );
	
	mParams->addSeparator( "" );
	mParams->addParam( "Material ambient",	&mMaterialAmbient,				"min=0.0 max=1.0 step=0.001" );
	mParams->addParam( "Material diffuse",	&mMaterialDiffuse,				"min=0.0 max=1.0 step=0.001" );
	mParams->addParam( "Material emissive",	&mMaterialEmissive,				"min=0.0 max=1.0 step=0.001" );
	mParams->addParam( "Material specular",	&mMaterialSpecular,				"min=0.0 max=1.0 step=0.001" );
}

void GpGpuApp::update()
{
	mFrameRate = getAverageFps();
	
	// Toggle full screen mode
	if ( mFullScreenPrev != mFullScreen ) {
		setFullScreen( mFullScreen );
		mFullScreenPrev = mFullScreen;
	}
	
	// Reset the FBOs if our grid size changes
	if ( mSizePrev != mSize ) {
		
		// Reset FBO index
		mFboIndex = 0;
		
		// We're going to create two FBOs, each with three color attachments
		gl::Fbo::Format format;
		format.enableColorBuffer( true, 3 );
		format.setColorInternalFormat( GL_RGBA32F_ARB );
		for ( size_t i = 0; i < 2; ++i ) {
			
			// Create the FBO
			mFbo[ i ]	= gl::Fbo( mSize.x, mSize.y, format );
			Area bounds = mFbo[ i ].getBounds();
			
			// Bind the FBO so we can draw on it
			mFbo[ i ].bindFramebuffer();
			
			// Configure the view
			gl::setViewport( bounds );
			gl::setMatricesWindow ( mSize );
			
			// This lets us target the three color attachments together
			glDrawBuffers( 3, kColorAttachments );
			
			// Fill in all attachments with grey
			gl::color( ColorAf::gray( 0.5f ) );
			gl::drawSolidRect( bounds );
			
			// Unbind the FBO so we are no longer drawing to it
			mFbo[ i ].unbindFramebuffer();
		}
		
		mSizePrev = mSize;
	}
	
	// Update camera position
	mCamera.lookAt( mEyePoint, Vec3f::zero() );
	
}

CINDER_APP_BASIC( GpGpuApp, RendererGl( RendererGl::AA_MSAA_32 ) )
