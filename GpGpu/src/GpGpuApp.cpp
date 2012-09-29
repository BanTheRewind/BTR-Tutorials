/*
* Copyright (c) 2012, Ban the Rewind
* All rights reserved.
* 
* Redistribution and use in source and binary forms, with or 
* without modification, are permitted provided that the following 
* conditions are met:
* 
* Redistributions of source code must retain the above copyright 
* notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright 
* notice, this list of conditions and the following disclaimer in 
* the documentation and/or other materials provided with the 
* distribution.
* 
* Neither the name of the Ban the Rewind nor the names of its 
* contributors may be used to endorse or promote products 
* derived from this software without specific prior written 
* permission.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* 
*/

#include "GpGpuApp.h"

#include "cinder/ImageIo.h"
#include "Resources.h"

using namespace ci;
using namespace ci::app;
using namespace std;

const Vec2i	kWindowSize	= Vec2i( 700, 700 );
const Vec2f	kPixel		= Vec2f::one() / Vec2f( kWindowSize );

void GpGpuApp::draw()
{
	///////////////////////////////////////////////////////////////
	// GPGPU pass

	// Enable textures
	gl::enable( GL_TEXTURE_2D );
	gl::color( Colorf::white() );

	// Bind the other FBO to draw onto it
	size_t pong = ( mFboIndex + 1 ) % 2;
	mFbo[ pong ].bindFramebuffer();

	// Set up the window to match the FBO
	gl::setViewport( mFbo[ mFboIndex ].getBounds() );
	gl::setMatricesWindow( mFbo[ mFboIndex ].getSize(), false );
	gl::clear();

	// Bind the texture from the FBO on which we last 
	// wrote data
	mFbo[ mFboIndex ].bindTexture();

	// Bind and configure the GPGPU shader
	mShaderGpGpu.bind();
	mShaderGpGpu.uniform( "buffer", 0 ); 
	mShaderGpGpu.uniform( "pixel", kPixel );

	// Draw a fullscreen rectangle to process data
	drawFullScreenRect();

	// End shader output
	mShaderGpGpu.unbind();

	// Unbind and disable textures
	mFbo[ mFboIndex ].unbindTexture();
	gl::disable( GL_TEXTURE_2D );

	// Draw mouse input into red channel
	if ( mMouseDown ) {
		gl::color( ColorAf( 1.0f, 0.0f, 0.0f, 1.0f ) );
		gl::drawSolidCircle( Vec2f( mMouse ), 5.0f, 32 );
		gl::color( Color::white() );
	}

	// Stop drawing to FBO
	mFbo[ pong ].unbindFramebuffer();

	// Swap FBOs
	mFboIndex = pong;

	///////////////////////////////////////////////////////////////
	// Refraction pass

	// Clear screen and set up viewport
	gl::clear( Color::black() );
	gl::setViewport( getWindowBounds() );
	gl::setMatricesWindow( getWindowSize() );
	
	// This flag draws the raw data without refraction
	if ( mShowInput ) {
		gl::draw( mFbo[ mFboIndex ].getTexture() );
	} else {

		// Bind the FBO we last rendered as a texture
		mFbo[ mFboIndex ].bindTexture( 0, 0 );

		// Bind and enable the refraction texture
		gl::enable( GL_TEXTURE_2D );
		mTexture.bind( 1 );
		
		// Bind and configure the refraction shader
		mShaderRefraction.bind();
		mShaderRefraction.uniform( "buffer", 0 );
		mShaderRefraction.uniform( "pixel", kPixel );
		mShaderRefraction.uniform( "tex", 1 );

		// Fill the screen with the shader output
		drawFullScreenRect();

		// Unbind and disable the texture
		mTexture.unbind();
		gl::disable( GL_TEXTURE_2D );

		// End shader output
		mShaderRefraction.unbind();
	}
}

void GpGpuApp::drawFullScreenRect()
{
	// Begin drawing
	gl::begin( GL_TRIANGLES );

	// Define quad vertices
	Area bounds = getWindowBounds();
	Vec2f vert0( (float)bounds.x1, (float)bounds.y1 );
	Vec2f vert1( (float)bounds.x2, (float)bounds.y1 );
	Vec2f vert2( (float)bounds.x1, (float)bounds.y2 );
	Vec2f vert3( (float)bounds.x2, (float)bounds.y2 );

	// Define quad texture coordinates
	Vec2f uv0( 0.0f, 0.0f );
	Vec2f uv1( 1.0f, 0.0f );
	Vec2f uv2( 0.0f, 1.0f );
	Vec2f uv3( 1.0f, 1.0f );

	// Draw quad (two triangles)
	gl::texCoord( uv0 );
	gl::vertex( vert0 );
	gl::texCoord( uv2 );
	gl::vertex( vert2 );
	gl::texCoord( uv1 );
	gl::vertex( vert1 );

	gl::texCoord( uv1 );
	gl::vertex( vert1 );
	gl::texCoord( uv2 );
	gl::vertex( vert2 );
	gl::texCoord( uv3 );
	gl::vertex( vert3 );

	// End drawing
	gl::end();
}

void GpGpuApp::keyDown( KeyEvent event )
{
	switch ( event.getCode() ) {		
		case KeyEvent::KEY_q: 
			quit();
		break;
		case KeyEvent::KEY_i:
			mShowInput = !mShowInput;
		break;
	}
}

void GpGpuApp::mouseDown( MouseEvent event )
{
	mMouseDown = true;
	mouseDrag( event );
}

void GpGpuApp::mouseDrag( MouseEvent event )
{
	mMouse = event.getPos();
}

void GpGpuApp::mouseUp( MouseEvent event )
{
	mMouseDown = false;
}

void GpGpuApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( kWindowSize.x, kWindowSize.y );
	settings->setFrameRate( 60.0f );
	settings->setResizable( false );
}

void GpGpuApp::setup()
{
	// Set flags
	mMouse		= Vec2i::zero();
	mMouseDown	= false;
	mShowInput	= false;

	// Load shaders
	try {
		mShaderGpGpu = gl::GlslProg( loadResource( RES_PASS_THRU_VERT ), loadResource( RES_GPGPU_FRAG ) );
	} catch ( gl::GlslProgCompileExc ex ) {
		console() << "Unable to compile GPGPU shader:\n" << ex.what() << "\n";
		quit();
	}
	try {
		mShaderRefraction = gl::GlslProg( loadResource( RES_PASS_THRU_VERT ), loadResource( RES_REFRACTION_FRAG ) );
	} catch ( gl::GlslProgCompileExc ex ) {
		console() << "Unable to compile refraction shader:\n" << ex.what() << "\n";
		quit();
	}

	// Load refraction texture
	{
		gl::Texture::Format format;
		format.setInternalFormat( GL_RGBA32F_ARB );
		mTexture = gl::Texture( loadImage( loadResource( RES_TEXTURE ) ) );
		mTexture.setWrap( GL_REPEAT, GL_REPEAT );
	}
	
	// Create FBO
	{
		// Set up format with 32-bit color for high resolution data
		gl::Fbo::Format format;
		format.enableColorBuffer( true );
		format.enableDepthBuffer( false );
		format.setColorInternalFormat( GL_RGBA32F_ARB );

		// Create two frame buffer objects to ping pong
		mFboIndex = 0;
		for ( size_t i = 0; i < 2; ++i ) {
			mFbo[ i ] = gl::Fbo( kWindowSize.x, kWindowSize.y, format );
			mFbo[ i ].bindFramebuffer();
			gl::setViewport( mFbo[ i ].getBounds() );
			gl::clear();
			mFbo[ i ].unbindFramebuffer();
			mFbo[ i ].getTexture().setWrap( GL_REPEAT, GL_REPEAT );
		}
	}
}

CINDER_APP_BASIC( GpGpuApp, RendererGl( RendererGl::AA_MSAA_32 ) )
