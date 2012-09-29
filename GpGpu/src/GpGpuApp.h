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

#pragma once

#include "cinder/app/AppBasic.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"

/* 
 * This application demonstrates how to use the FBO
 * ping pong technique to update interactive data on 
 * the GPU with GLSL (GPGPU). The data is then used 
 * to refract a bitmap in GLSL. The motion is a version 
 * of the classic "2D water" algorithm.
*/

class GpGpuApp : public ci::app::AppBasic
{
public:
	void				draw();
	void				keyDown( ci::app::KeyEvent event );
	void				mouseDown( ci::app::MouseEvent event );
	void				mouseDrag( ci::app::MouseEvent event );
	void				mouseUp( ci::app::MouseEvent event );
	void				prepareSettings( ci::app::AppBasic::Settings *settings );
	void				setup();
private:
	// Convenience method for drawing fullscreeen rectangle
	// with texture coordinates
	void				drawFullScreenRect();

	// Frame buffer objects to ping pong
	ci::gl::Fbo			mFbo[ 2 ];
	size_t				mFboIndex;

	// Shaders
	ci::gl::GlslProg	mShaderGpGpu;
	ci::gl::GlslProg	mShaderRefraction;

	// Refraction texture
	ci::gl::Texture		mTexture;

	// Mouse
	ci::Vec2i			mMouse;
	bool				mMouseDown;

	// True renders input to screen
	bool				mShowInput;
};
