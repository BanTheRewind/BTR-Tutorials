#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"

#include "FMOD.hpp"

/*
 * This application demonstrates how to set up and 
 * configure FMOD DSP for both effects and instruments. *
 */

class FmodDspApp : public ci::app::AppNative
{
public:
	void						draw();
	void						prepareSettings( ci::app::AppNative::Settings* settings );
	void						setup();
	void						update();
private:
	FMOD::Channel*				mChannelSound;
	FMOD::Channel*				mChannelSynth;
	FMOD::Sound*				mSound;
	FMOD::System*				mSystem;
	
	float						mSoundLevel;
	float						mSoundSpeed;
	
	FMOD::DSP*					mDspChorus;
	FMOD::DSP*					mDspCompressor;
	FMOD::DSP*					mDspDistortion;
	FMOD::DSP*					mDspEcho;
	FMOD::DSP*					mDspFlange;
	FMOD::DSP*					mDspHighPass;
	FMOD::DSP*					mDspLowPass;
	FMOD::DSP*					mDspNormalize;
	FMOD::DSP*					mDspOscillator;
	FMOD::DSP*					mDspParamEq;
	FMOD::DSP*					mDspPitchShift;
	FMOD::DSP*					mDspReverb;
	FMOD::DSP*					mDspTremolo;
	
	float						mChorusDepth;
	float						mChorusDryMix;
	float						mChorusDelay;
	bool						mChorusEnabled;
	bool						mChorusEnabledPrev;
	float						mChorusRate;
	float						mChorusWetMix1;
	float						mChorusWetMix2;
	float						mChorusWetMix3;
	
	int32_t						mCompressorAttack;
	bool						mCompressorEnabled;
	bool						mCompressorEnabledPrev;
	float						mCompressorGainMakeup;
	int32_t						mCompressorRelease;
	float						mCompressorThreshold;
	
	float						mDistortionLevel;
	bool						mDistortionEnabled;
	bool						mDistortionEnabledPrev;
	
	float						mEchoDecayRatio;
	int32_t						mEchoDelay;
	float						mEchoDryMix;
	bool						mEchoEnabled;
	bool						mEchoEnabledPrev;
	float						mEchoWetMix;
	
	float						mFlangeDepth;
	float						mFlangeDryMix;
	bool						mFlangeEnabled;
	bool						mFlangeEnabledPrev;
	float						mFlangeRate;
	float						mFlangeWetMix;
	
	float						mHighPassCutoff;
	bool						mHighPassEnabled;
	bool						mHighPassEnabledPrev;
	float						mHighPassResonance;
	
	float						mLowPassCutoff;
	bool						mLowPassEnabled;
	bool						mLowPassEnabledPrev;
	float						mLowPassResonance;
	
	bool						mNormalizeEnabled;
	bool						mNormalizeEnabledPrev;
	float						mNormalizeFadeTime;
	float						mNormalizeMaxAmp;
	float						mNormalizeThreshold;
	
	bool						mOscillatorEnabled;
	bool						mOscillatorEnabledPrev;
	float						mOscillatorLevel;
	float						mOscillatorRate;
	int32_t						mOscillatorType;
	int32_t						mOscillatorTypePrev;
	std::vector<std::string>	mOscillatorTypes;
	
	float						mParamEqBandwidth;
	float						mParamEqCenter;
	bool						mParamEqEnabled;
	bool						mParamEqEnabledPrev;
	float						mParamEqGain;
	
	bool						mPitchShiftEnabled;
	bool						mPitchShiftEnabledPrev;
	float						mPitchShiftPitch;
	bool						mPitchShiftSoundLock;
	
	float						mReverbDecayHfRatio;
	float						mReverbDecayTime;
	float						mReverbDensity;
	float						mReverbDiffusion;
	float						mReverbDryLevel;
	bool						mReverbEnabled;
	bool						mReverbEnabledPrev;
	float						mReverbHfReference;
	float						mReverbLfReference;
	float						mReverbReflectionsDelay;
	float						mReverbReflectionsLevel;
	float						mReverbReverbDelay;
	float						mReverbReverbLevel;
	float						mReverbRoom;
	float						mReverbRoomHf;
	float						mReverbRoomLf;
	
	float						mTremoloDepth;
	float						mTremoloDuty;
	bool						mTremoloEnabled;
	bool						mTremoloEnabledPrev;
	float						mTremoloFrequency;
	float						mTremoloPhase;
	float						mTremoloShape;
	float						mTremoloSkew;
	float						mTremoloSpread;
	float						mTremoloSquare;
	
	float						mFrameRate;
	bool						mFullScreen;
	bool						mFullScreenPrev;
	ci::params::InterfaceGlRef	mParams;
};

using namespace ci;
using namespace ci::app;
using namespace std;

static const size_t kBufferLength	= 512;
static const float	kBufferLengthf	= (float)kBufferLength;

void FmodDspApp::draw()
{
	gl::clear();
	
	float waveData[ kBufferLength ];
	mSystem->getWaveData( waveData, kBufferLength, 0 );
	
	float h = (float)getWindowHeight() * 0.25f;
	float w = (float)getWindowWidth();
	float y = getWindowCenter().y;
	vector<Vec2f> vertices;
	for ( size_t i = 0; i < kBufferLength; ++i ) {
		vertices.push_back( Vec2f( w / kBufferLengthf * i, y + h * waveData[ i ] ) );
	}

	glEnableClientState( GL_VERTEX_ARRAY );
	gl::color( Color( 1.0f, 0.5f, 0.25f ) );
	glVertexPointer( 2, GL_FLOAT, 0, &vertices[ 0 ] );
	glDrawArrays( GL_LINE_STRIP, 0, vertices.size() );
	
	mParams->draw();
}

void FmodDspApp::prepareSettings( Settings* settings )
{
	settings->setFrameRate( 60.0f );
	settings->setWindowSize( 1024, 768 );
}

void FmodDspApp::setup()
{
	///////////////////////////////////////////////////////////////////////////
	// Define all properties
	
	mFrameRate				= 0.0f;
	mFullScreen				= isFullScreen();
	mFullScreenPrev			= mFullScreen;
	
	mChorusDelay			= 40.0f;
	mChorusDepth			= 0.03f;
	mChorusDryMix			= 0.5f;
	mChorusEnabled			= false;
	mChorusEnabledPrev		= mChorusEnabled;
	mChorusRate				= 0.8f;
	mChorusWetMix1			= 0.5f;
	mChorusWetMix2			= 0.5f;
	mChorusWetMix3			= 0.5f;
	
	mCompressorEnabled		= false;
	mCompressorEnabledPrev	= mCompressorEnabled;
	mCompressorAttack		= 50;
	mCompressorGainMakeup	= 0.0f;
	mCompressorRelease		= 50;
	mCompressorThreshold	= 0.0f;
	
	mDistortionLevel		= 0.5f;
	mDistortionEnabled		= false;
	mDistortionEnabledPrev	= mDistortionEnabled;
	
	mEchoDecayRatio			= 0.0f;
	mEchoDelay				= 500;
	mEchoDryMix				= 1.0f;
	mEchoEnabled			= false;
	mEchoEnabledPrev		= mEchoEnabled;
	mEchoWetMix				= 1.0f;
	
	mFlangeDepth			= 1.0f;
	mFlangeDryMix			= 0.45f;
	mFlangeEnabled			= false;
	mFlangeEnabledPrev		= mFlangeEnabled;
	mFlangeRate				= 0.1f;
	mFlangeWetMix			= 0.55f;
	
	mHighPassCutoff			= 5000.0f;
	mHighPassEnabled		= false;
	mHighPassEnabledPrev	= mHighPassEnabled;
	mHighPassResonance		= 1.0f;
	
	mLowPassCutoff			= 5000.0f;
	mLowPassEnabled			= false;
	mLowPassEnabledPrev		= mLowPassEnabled;
	mLowPassResonance		= 1.0f;
	
	mNormalizeEnabled		= false;
	mNormalizeEnabledPrev	= mNormalizeEnabled;
	mNormalizeFadeTime		= 5000.0f;
	mNormalizeMaxAmp		= 20.0f;
	mNormalizeThreshold		= 0.1f;
	
	mOscillatorEnabled		= false;
	mOscillatorEnabledPrev	= mOscillatorEnabled;
	mOscillatorLevel		= 1.0f;
	mOscillatorRate			= 220.0f;
	mOscillatorType			= 0;
	mOscillatorTypePrev		= mOscillatorType;
	mOscillatorTypes.push_back( "Sine" );
	mOscillatorTypes.push_back( "Square" );
	mOscillatorTypes.push_back( "Saw up" );
	mOscillatorTypes.push_back( "Saw down" );
	mOscillatorTypes.push_back( "Triangle" );
	mOscillatorTypes.push_back( "Noise" );
	
	mParamEqBandwidth		= 1.0f;
	mParamEqCenter			= 8000.0f;
	mParamEqEnabled			= false;
	mParamEqEnabledPrev		= mParamEqEnabled;
	mParamEqGain			= 1.0f;
	
	mPitchShiftEnabled		= false;
	mPitchShiftEnabledPrev	= mPitchShiftEnabled;
	mPitchShiftPitch		= 1.0f;
	mPitchShiftSoundLock	= false;
	
	mReverbDecayHfRatio		= 0.5f;
	mReverbDecayTime		= 1.0f;
	mReverbDensity			= 100.0f;
	mReverbDiffusion		= 100.0f;
	mReverbDryLevel			= 0.0f;
	mReverbEnabled			= false;
	mReverbEnabledPrev		= mReverbEnabled;
	mReverbHfReference		= 5000.0f;
	mReverbLfReference		= 250.0f;
	mReverbReflectionsDelay	= 0.02f;
	mReverbReflectionsLevel	= -10000.0f;
	mReverbReverbDelay		= 0.04f;
	mReverbReverbLevel		= 0.0f;
	mReverbRoom				= -10000.0f;
	mReverbRoomHf			= 0.0f;
	mReverbRoomLf			= 0.0f;
	
	mSoundLevel				= 1.0f;
	mSoundSpeed				= 1.0f;
	
	mTremoloDepth			= 0.0f;
	mTremoloDuty			= 0.5f;
	mTremoloEnabled			= false;
	mTremoloEnabledPrev		= true;
	mTremoloFrequency		= 4.0f;
	mTremoloPhase			= 0.0f;
	mTremoloShape			= 0.0f;
	mTremoloSkew			= 0.0f;
	mTremoloSpread			= 0.0f;
	mTremoloSquare			= 0.0f;
	
	///////////////////////////////////////////////////////////////////////////
	// FMOD
	
	// Basic FMOD system initialization
	FMOD::System_Create( &mSystem );
	mSystem->init( 32, FMOD_INIT_NORMAL, 0 );
	
	// Initialize our DSPs
	mSystem->createDSPByType( FMOD_DSP_TYPE_CHORUS,		&mDspChorus );
	mSystem->createDSPByType( FMOD_DSP_TYPE_COMPRESSOR,	&mDspCompressor );
	mSystem->createDSPByType( FMOD_DSP_TYPE_DISTORTION,	&mDspDistortion );
	mSystem->createDSPByType( FMOD_DSP_TYPE_ECHO,		&mDspEcho );
	mSystem->createDSPByType( FMOD_DSP_TYPE_FLANGE,		&mDspFlange );
	mSystem->createDSPByType( FMOD_DSP_TYPE_HIGHPASS,	&mDspHighPass );
	mSystem->createDSPByType( FMOD_DSP_TYPE_LOWPASS,	&mDspLowPass );
	mSystem->createDSPByType( FMOD_DSP_TYPE_NORMALIZE,	&mDspNormalize );
	mSystem->createDSPByType( FMOD_DSP_TYPE_OSCILLATOR, &mDspOscillator );
	mSystem->createDSPByType( FMOD_DSP_TYPE_PARAMEQ,	&mDspParamEq );
	mSystem->createDSPByType( FMOD_DSP_TYPE_PITCHSHIFT,	&mDspPitchShift );
	mSystem->createDSPByType( FMOD_DSP_TYPE_SFXREVERB,	&mDspReverb );
	mSystem->createDSPByType( FMOD_DSP_TYPE_TREMOLO,	&mDspTremolo );
	
	// Load and play the loop
	mSystem->createSound( getAssetPath( "Blank__Kytt_-_08_-_RSPN.mp3" ).string().c_str(), FMOD_SOFTWARE, 0, &mSound );
	mSound->setMode( FMOD_LOOP_NORMAL );
	mSystem->playSound( FMOD_CHANNEL_FREE, mSound, false, &mChannelSound );
	
	// The oscillator generates sound, so we are going
	// to play it in its own channel rather than add
	// it as an effect
	mSystem->playDSP( FMOD_CHANNEL_FREE, mDspOscillator, true, &mChannelSynth );
	
	// Lower the FFT size for better clarity when locking pitch
	mDspPitchShift->setParameter( FMOD_DSP_PITCHSHIFT_FFTSIZE, 512 );
	
	///////////////////////////////////////////////////////////////////////////
	// Params
	
	mParams = params::InterfaceGl::create( "Params", Vec2i( 260, 700 ) );
	mParams->addParam( "Frame rate",				&mFrameRate,				"", true );
	mParams->addParam( "Full screen",				&mFullScreen,				"key=f" );
	mParams->addButton( "Quit",						bind( &FmodDspApp::quit, this ), "key=q" );
	
	mParams->addSeparator();
	mParams->addParam( "Sound: Level",				&mSoundLevel,				"min=0.0 max=1.0 step=0.001" );
	mParams->addParam( "Sound: Speed",				&mSoundSpeed,				"min=0.5 max=2.0 step=0.001" );
	
	mParams->addSeparator();
	mParams->addParam( "Chorus: Delay",				&mChorusDelay,				"min=0.1 max=100.0 step=0.1" );
	mParams->addParam( "Chorus: Depth",				&mChorusDepth,				"min=0.0 max=1.0 step=0.0001" );
	mParams->addParam( "Chorus: Dry mix",			&mChorusDryMix,				"min=0.0 max=1.0 step=0.001" );
	mParams->addParam( "Chorus: Enabled",			&mChorusEnabled,			"key=c" );
	mParams->addParam( "Chorus: Rate",				&mChorusRate,				"min=0.0 max=20.0 step=0.01" );
	mParams->addParam( "Chorus: Wet mix 1",			&mChorusWetMix1,			"min=0.0 max=1.0 step=0.001" );
	mParams->addParam( "Chorus: Wet mix 2",			&mChorusWetMix2,			"min=0.0 max=1.0 step=0.001" );
	mParams->addParam( "Chorus: Wet mix 3",			&mChorusWetMix3,			"min=0.0 max=1.0 step=0.001" );
	
	mParams->addSeparator();
	mParams->addParam( "Compressor: Attack",		&mCompressorAttack,			"min=10 max=200 step=1" );
	mParams->addParam( "Compressor: Enabled",		&mCompressorEnabled,		"key=m" );
	mParams->addParam( "Compressor: Gain makeup",	&mCompressorGainMakeup,		"min=0.0 max=30.0 step=0.01" );
	mParams->addParam( "Compressor: Release",		&mCompressorRelease,		"min=20 max=1000 step=1" );
	mParams->addParam( "Compressor: Threshold",		&mCompressorThreshold,		"min=-60.0 max=0.0 step=0.01" );
	
	mParams->addSeparator();
	mParams->addParam( "Distortion: Level",			&mDistortionLevel,			"min=0.0 max=1.0 step=0.001" );
	mParams->addParam( "Distortion: Enabled",		&mDistortionEnabled,		"key=d" );
	
	mParams->addSeparator();
	mParams->addParam( "Echo: Decay ratio",			&mEchoDecayRatio,			"min=0.0 max=1.0 step=0.001" );
	mParams->addParam( "Echo: Delay",				&mEchoDelay,				"min=10 max=5000 step=1" );
	mParams->addParam( "Echo: Dry mix",				&mEchoDryMix,				"min=0.0 max=1.0 step=0.001" );
	mParams->addParam( "Echo: Enabled",				&mEchoEnabled,				"key=e" );
	mParams->addParam( "Echo: Wet mix",				&mEchoWetMix,				"min=0.0 max=1.0 step=0.001" );
	
	mParams->addSeparator();
	mParams->addParam( "Flange: Depth",				&mFlangeDepth,				"min=0.01 max=1.0 step=0.001" );
	mParams->addParam( "Flange: Dry Mix",			&mFlangeDryMix,				"min=0.0 max=1.0 step=0.001" );
	mParams->addParam( "Flange: Enabled",			&mFlangeEnabled,			"key=g" );
	mParams->addParam( "Flange: Rate",				&mFlangeRate,				"min=0.0 max=20.0 step=0.01" );
	mParams->addParam( "Flange: Wet Mix",			&mFlangeWetMix,				"min=0.0 max=1.0 step=0.001" );
	
	mParams->addSeparator();
	mParams->addParam( "High pass: Cutoff",			&mHighPassCutoff,			"min=10.0 max=22000.0 step=1.0" );
	mParams->addParam( "High pass: Enabled",		&mHighPassEnabled,			"key=h" );
	mParams->addParam( "High pass: Resonance",		&mHighPassResonance,		"min=1.0 max=10.0 step=0.01" );
	
	mParams->addSeparator();
	mParams->addParam( "Low pass: Cutoff",			&mLowPassCutoff,			"min=10.0 max=22000.0 step=1.0" );
	mParams->addParam( "Low pass: Enabled",			&mLowPassEnabled,			"key=l" );
	mParams->addParam( "Low pass: Resonance",		&mLowPassResonance,			"min=1.0 max=10.0 step=0.01" );
	
	mParams->addSeparator();
	mParams->addParam( "Normalize: Enabled",		&mNormalizeEnabled,			"key=n" );
	mParams->addParam( "Normalize: Fade time",		&mNormalizeFadeTime,		"min=0.0 max=20000.0 step=1.0" );
	mParams->addParam( "Normalize: Max amp",		&mNormalizeMaxAmp,			"min=1.0 max=100000.0 step=1.0" );
	mParams->addParam( "Normalize: Threshold",		&mNormalizeThreshold,		"min=0.0 max=1.0 step=0.001" );
	
	mParams->addSeparator();
	mParams->addParam( "Oscillator: Enabled",		&mOscillatorEnabled,		"key=o" );
	mParams->addParam( "Oscillator: Level",			&mOscillatorLevel,			"min=0.0 max=1.0 step=0.001" );
	mParams->addParam( "Oscillator: Rate",			&mOscillatorRate,			"min=1.0 max=22000.0 step=1.0" );
	mParams->addParam( "Oscillator: Type",			mOscillatorTypes, &mOscillatorType );
	
	mParams->addSeparator();
	mParams->addParam( "Param EQ: Bandwidth",		&mParamEqBandwidth,			"min=0.2 max=5.0 step=0.01" );
	mParams->addParam( "Param EQ: Center",			&mParamEqCenter,			"min=20.0 max=22000.0 step=1.0" );
	mParams->addParam( "Param EQ: Enabled",			&mParamEqEnabled,			"key=p" );
	mParams->addParam( "Param EQ: Gain",			&mParamEqGain,				"min=0.05 max=3.0 step=0.01" );
	
	mParams->addSeparator();
	mParams->addParam( "Pitch shift: Enabled",		&mPitchShiftEnabled,		"key=s" );
	mParams->addParam( "Pitch shift: Pitch",		&mPitchShiftPitch,			"min=0.5 max=2.0 step=0.001" );
	mParams->addParam( "Pitch shift: Sound lock",	&mPitchShiftSoundLock );
	
	mParams->addSeparator();
	mParams->addParam( "Reverb: Decay HF ratio",	&mReverbDecayHfRatio,		"min=0.1 max=2.0 step=0.001" );
	mParams->addParam( "Reverb: Decay time",		&mReverbDecayTime,			"min=0.1 max=20.0 step=0.01" );
	mParams->addParam( "Reverb: Density",			&mReverbDensity,			"min=0.0 max=100.0 step=0.1" );
	mParams->addParam( "Reverb: Diffusion",			&mReverbDiffusion,			"min=0.0 max=100.0 step=0.1" );
	mParams->addParam( "Reverb: Dry level",			&mReverbDryLevel,			"min=0.0 max=1.0 step=0.001" );
	mParams->addParam( "Reverb: Enabled",			&mReverbEnabled,			"key=r" );
	mParams->addParam( "Reverb: HF Reference",		&mReverbHfReference,		"min=20.0 max=20000.0 step=1.0" );
	mParams->addParam( "Reverb: LF reference",		&mReverbLfReference,		"min=20.0 max=1000.0 step=1.0" );
	mParams->addParam( "Reverb: Reflections delay",	&mReverbReflectionsDelay,	"min=0.0 max=0.3 step=0.001" );
	mParams->addParam( "Reverb: Reflections level",	&mReverbReflectionsLevel,	"min=-10000.0 max=1000.0 step=1.0" );
	mParams->addParam( "Reverb: Reverb delay",		&mReverbReverbDelay,		"min=0.0 max=0.1 step=0.0001" );
	mParams->addParam( "Reverb: Reverb level",		&mReverbReverbLevel,		"min=-10000.0 max=2000.0 step=1.0" );
	mParams->addParam( "Reverb: Room",				&mReverbRoom,				"min=-10000.0 max=0.0 step=1.0" );
	mParams->addParam( "Reverb: Room HF",			&mReverbRoomHf,				"min=-10000.0 max=0.0 step=1.0" );
	mParams->addParam( "Reverb: Room LF",			&mReverbRoomLf,				"min=-10000.0 max=0.0 step=1.0" );

	mParams->addSeparator();
	mParams->addParam( "Tremolo: Depth",			&mTremoloDepth,				"min=0.0 max=1.0 step=0.001" );
	mParams->addParam( "Tremolo: Duty",				&mTremoloDuty,				"min=0.0 max=1.0 step=0.001" );
	mParams->addParam( "Tremolo: Enabled",			&mTremoloEnabled,			"key=t" );
	mParams->addParam( "Tremolo: Frequency",		&mTremoloFrequency,			"min=0.1 max=20.0 step=0.001" );
	mParams->addParam( "Tremolo: Phase",			&mTremoloPhase,				"min=0.0 max=1.0 step=0.001" );
	mParams->addParam( "Tremolo: Shape",			&mTremoloShape,				"min=0.0 max=1.0 step=0.001" );
	mParams->addParam( "Tremolo: Skew",				&mTremoloSkew,				"min=-1.0 max=1.0 step=0.001" );
	mParams->addParam( "Tremolo: Spread",			&mTremoloSpread,			"min=-1.0 max=1.0 step=0.001" );
	mParams->addParam( "Tremolo: Square",			&mTremoloSquare,			"min=0.0 max=1.0 step=0.001" );
}

void FmodDspApp::update()
{
	mFrameRate = getAverageFps();
	
	// Toggle fullscreen mode
	if ( mFullScreenPrev != mFullScreen ) {
		setFullScreen( mFullScreen );
		mFullScreenPrev	= mFullScreen;
	}
	
	// Sample playback
	mChannelSound->setFrequency( 44100.0f * mSoundSpeed );
	mChannelSound->setVolume( mSoundLevel );
	
	// Chorus
	if ( mChorusEnabledPrev != mChorusEnabled ) {
		if ( mChorusEnabled ) {
			mSystem->addDSP( mDspChorus, 0 );
		} else {
			mDspChorus->remove();
		}
		mChorusEnabledPrev = mChorusEnabled;
	}
	if ( mChorusEnabled ) {
		mDspChorus->setParameter( FMOD_DSP_CHORUS_DELAY,	mChorusDelay );
		mDspChorus->setParameter( FMOD_DSP_CHORUS_DEPTH,	mChorusDepth );
		mDspChorus->setParameter( FMOD_DSP_CHORUS_DRYMIX,	mChorusDryMix );
		mDspChorus->setParameter( FMOD_DSP_CHORUS_RATE,		mChorusRate );
		mDspChorus->setParameter( FMOD_DSP_CHORUS_WETMIX1,	mChorusWetMix1 );
		mDspChorus->setParameter( FMOD_DSP_CHORUS_WETMIX2,	mChorusWetMix2 );
		mDspChorus->setParameter( FMOD_DSP_CHORUS_WETMIX3,	mChorusWetMix3 );
	}
	
	// Compressor
	if ( mCompressorEnabledPrev != mCompressorEnabled ) {
		if ( mCompressorEnabled ) {
			mSystem->addDSP( mDspCompressor, 0 );
		} else {
			mDspCompressor->remove();
		}
		mCompressorEnabledPrev = mCompressorEnabled;
	}
	if ( mCompressorEnabled ) {
		mDspCompressor->setParameter( FMOD_DSP_COMPRESSOR_ATTACK,		mCompressorAttack );
		mDspCompressor->setParameter( FMOD_DSP_COMPRESSOR_GAINMAKEUP,	mCompressorGainMakeup );
		mDspCompressor->setParameter( FMOD_DSP_COMPRESSOR_RELEASE,		mCompressorRelease );
		mDspCompressor->setParameter( FMOD_DSP_COMPRESSOR_THRESHOLD,	mCompressorThreshold );
	}
	
	// Distortion
	if ( mDistortionEnabledPrev != mDistortionEnabled ) {
		if ( mDistortionEnabled ) {
			mSystem->addDSP( mDspDistortion, 0 );
		} else {
			mDspDistortion->remove();
		}
		mDistortionEnabledPrev = mDistortionEnabled;
	}
	if ( mDistortionEnabled ) {
		mDspDistortion->setParameter( FMOD_DSP_DISTORTION_LEVEL, mDistortionLevel );
	}
	
	// Echo
	if ( mEchoEnabledPrev != mEchoEnabled ) {
		if ( mEchoEnabled ) {
			mSystem->addDSP( mDspEcho, 0 );
		} else {
			mDspEcho->remove();
		}
		mEchoEnabledPrev = mEchoEnabled;
	}
	if ( mEchoEnabled ) {
		mDspEcho->setParameter( FMOD_DSP_ECHO_DECAYRATIO,	mEchoDecayRatio );
		mDspEcho->setParameter( FMOD_DSP_ECHO_DELAY,		mEchoDelay );
		mDspEcho->setParameter( FMOD_DSP_ECHO_DRYMIX,		mEchoDryMix );
		mDspEcho->setParameter( FMOD_DSP_ECHO_WETMIX,		mEchoWetMix );
	}
	
	// Flange
	if ( mFlangeEnabledPrev != mFlangeEnabled ) {
		if ( mFlangeEnabled ) {
			mSystem->addDSP( mDspFlange, 0 );
		} else {
			mDspFlange->remove();
		}
		mFlangeEnabledPrev = mFlangeEnabled;
	}
	if ( mFlangeEnabled ) {
		mDspFlange->setParameter( FMOD_DSP_FLANGE_DEPTH,	mFlangeDepth );
		mDspFlange->setParameter( FMOD_DSP_FLANGE_DRYMIX,	mFlangeDryMix );
		mDspFlange->setParameter( FMOD_DSP_FLANGE_RATE,		mFlangeRate );
		mDspFlange->setParameter( FMOD_DSP_FLANGE_WETMIX,	mFlangeWetMix );
	}
	
	// High pass filter
	if ( mHighPassEnabledPrev != mHighPassEnabled ) {
		if ( mHighPassEnabled ) {
			mSystem->addDSP( mDspHighPass, 0 );
		} else {
			mDspHighPass->remove();
		}
		mHighPassEnabledPrev = mHighPassEnabled;
	}
	if ( mHighPassEnabled ) {
		mDspHighPass->setParameter( FMOD_DSP_HIGHPASS_CUTOFF,		mHighPassCutoff );
		mDspHighPass->setParameter( FMOD_DSP_HIGHPASS_RESONANCE,	mHighPassResonance );
	}
	
	// Low pass filter
	if ( mLowPassEnabledPrev != mLowPassEnabled ) {
		if ( mLowPassEnabled ) {
			mSystem->addDSP( mDspLowPass, 0 );
		} else {
			mDspLowPass->remove();
		}
		mLowPassEnabledPrev = mLowPassEnabled;
	}
	if ( mLowPassEnabled ) {
		mDspLowPass->setParameter( FMOD_DSP_LOWPASS_CUTOFF,		mLowPassCutoff );
		mDspLowPass->setParameter( FMOD_DSP_LOWPASS_RESONANCE,	mLowPassResonance );
	}
	
	// Normalizer
	if ( mNormalizeEnabledPrev != mNormalizeEnabled ) {
		if ( mNormalizeEnabled ) {
			mSystem->addDSP( mDspNormalize, 0 );
		} else {
			mDspNormalize->remove();
		}
		mNormalizeEnabledPrev = mNormalizeEnabled;
	}
	if ( mNormalizeEnabled ) {
		mDspNormalize->setParameter( FMOD_DSP_NORMALIZE_FADETIME,	mNormalizeFadeTime );
		mDspNormalize->setParameter( FMOD_DSP_NORMALIZE_MAXAMP,		mNormalizeMaxAmp );
		mDspNormalize->setParameter( FMOD_DSP_NORMALIZE_THRESHHOLD,	mNormalizeThreshold );
	}
	
	// Oscillator (synth)
	if ( mOscillatorEnabledPrev != mOscillatorEnabled ) {
		mChannelSynth->setPaused( !mOscillatorEnabled );
		mOscillatorEnabledPrev = mOscillatorEnabled;
	}
	if ( mOscillatorTypePrev != mOscillatorType ) {
		mDspOscillator->setParameter( FMOD_DSP_OSCILLATOR_TYPE,	mOscillatorType );
		mOscillatorTypePrev = mOscillatorType;
	}
	if ( mOscillatorEnabled ) {
		mChannelSynth->setVolume( mOscillatorLevel );
		mDspOscillator->setParameter( FMOD_DSP_OSCILLATOR_RATE,	mOscillatorRate );
	}
	
	// Parametric EQ
	if ( mParamEqEnabledPrev != mParamEqEnabled ) {
		if ( mParamEqEnabled ) {
			mSystem->addDSP( mDspParamEq, 0 );
		} else {
			mDspParamEq->remove();
		}
		mParamEqEnabledPrev = mParamEqEnabled;
	}
	if ( mParamEqEnabled ) {
		mDspParamEq->setParameter( FMOD_DSP_PARAMEQ_BANDWIDTH,	mParamEqBandwidth );
		mDspParamEq->setParameter( FMOD_DSP_PARAMEQ_CENTER,		mParamEqCenter );
		mDspParamEq->setParameter( FMOD_DSP_PARAMEQ_GAIN,		mParamEqGain );
	}
	
	// Pitch shift
	if ( mPitchShiftSoundLock ) {
		mPitchShiftPitch = 2.0f - mSoundSpeed;
	}
	if ( mPitchShiftEnabledPrev != mPitchShiftEnabled ) {
		if ( mPitchShiftEnabled ) {
			mSystem->addDSP( mDspPitchShift, 0 );
		} else {
			mDspPitchShift->remove();
		}
		mPitchShiftEnabledPrev = mPitchShiftEnabled;
	}
	if ( mPitchShiftEnabled ) {
		mDspPitchShift->setParameter( FMOD_DSP_PITCHSHIFT_PITCH, mPitchShiftPitch );
	}
	
	// Reverb
	if ( mReverbEnabledPrev != mReverbEnabled ) {
		if ( mReverbEnabled ) {
			mSystem->addDSP( mDspReverb, 0 );
		} else {
			mDspReverb->remove();
		}
		mReverbEnabledPrev = mReverbEnabled;
	}
	if ( mReverbEnabled ) {
		mDspReverb->setParameter( FMOD_DSP_SFXREVERB_DECAYHFRATIO,		mReverbDecayHfRatio );
		mDspReverb->setParameter( FMOD_DSP_SFXREVERB_DECAYTIME,			mReverbDecayTime );
		mDspReverb->setParameter( FMOD_DSP_SFXREVERB_DENSITY,			mReverbDensity );
		mDspReverb->setParameter( FMOD_DSP_SFXREVERB_DIFFUSION,			mReverbDiffusion );
		mDspReverb->setParameter( FMOD_DSP_SFXREVERB_DRYLEVEL,			mReverbDryLevel );
		mDspReverb->setParameter( FMOD_DSP_SFXREVERB_HFREFERENCE,		mReverbHfReference );
		mDspReverb->setParameter( FMOD_DSP_SFXREVERB_LFREFERENCE,		mReverbLfReference );
		mDspReverb->setParameter( FMOD_DSP_SFXREVERB_REFLECTIONSDELAY,	mReverbReflectionsDelay );
		mDspReverb->setParameter( FMOD_DSP_SFXREVERB_REFLECTIONSLEVEL,	mReverbReflectionsLevel );
		mDspReverb->setParameter( FMOD_DSP_SFXREVERB_REVERBDELAY,		mReverbReverbDelay );
		mDspReverb->setParameter( FMOD_DSP_SFXREVERB_REVERBLEVEL,		mReverbReverbLevel );
		mDspReverb->setParameter( FMOD_DSP_SFXREVERB_ROOM,				mReverbRoom );
		mDspReverb->setParameter( FMOD_DSP_SFXREVERB_ROOMHF,			mReverbRoomHf );
		mDspReverb->setParameter( FMOD_DSP_SFXREVERB_ROOMLF,			mReverbRoomLf );
	}
	
	// Tremolo
	if ( mTremoloEnabledPrev != mTremoloEnabled ) {
		if ( mTremoloEnabled ) {
			mSystem->addDSP( mDspTremolo, 0 );
		} else {
			mDspTremolo->remove();
		}
		mTremoloEnabledPrev = mTremoloEnabled;
	}
	if ( mTremoloEnabled ) {
		mDspTremolo->setParameter( FMOD_DSP_TREMOLO_DEPTH,		mTremoloDepth );
		mDspTremolo->setParameter( FMOD_DSP_TREMOLO_DUTY,		mTremoloDuty );
		mDspTremolo->setParameter( FMOD_DSP_TREMOLO_FREQUENCY,	mTremoloFrequency );
		mDspTremolo->setParameter( FMOD_DSP_TREMOLO_PHASE,		mTremoloPhase * (float)getElapsedSeconds() );
		mDspTremolo->setParameter( FMOD_DSP_TREMOLO_SHAPE,		mTremoloShape );
		mDspTremolo->setParameter( FMOD_DSP_TREMOLO_SKEW,		mTremoloSkew );
		mDspTremolo->setParameter( FMOD_DSP_TREMOLO_SPREAD,		mTremoloSpread );
		mDspTremolo->setParameter( FMOD_DSP_TREMOLO_SQUARE,		mTremoloSquare );
	}
}

CINDER_APP_NATIVE( FmodDspApp, RendererGl )
