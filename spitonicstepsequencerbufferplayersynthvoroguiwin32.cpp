/*
 * Copyright (c) 2015-2016 Stephane Poirier
 *
 * stephane.poirier@oifii.org
 *
 * Stephane Poirier
 * 3532 rue Ste-Famille, #3
 * Montreal, QC, H2X 2L1
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include "spitonicstepsequencerbufferplayersynthvoroguiwin32.h"
#include "FreeImage.h"
#include <shellapi.h> //for CommandLineToArgW()
#include <mmsystem.h> //for timeSetEvent()
#include <stdio.h> //for swprintf()
#include <assert.h>
#include "spiwavsetlib.h"


#include "portaudio.h"

#ifdef WIN32
#if PA_USE_ASIO
#include "pa_asio.h"
#endif
#endif


//#include "DspFilters/Dsp.h"

#include "Tonic.h"
//#include "RtAudio.h"

#include "StepSequencerBufferPlayerDynamicSynth.h"
using namespace Tonic;


#include <string>
#include <map>
using namespace std;

#include "c_pointset.h"
#include "c_vorogui.h"


#define SAMPLE_RATE  (44100)
#define FRAMES_PER_BUFFER (2048) //#define FRAMES_PER_BUFFER (512) //#define FRAMES_PER_BUFFER (2048) //#define FRAMES_PER_BUFFER (64) 
//#define NUM_CHANNELS    (1)
#define NUM_CHANNELS    (2)



// Static smart pointer for our Synth
/*
static Synth synth;
*/
//static ControlSwitcherTestSynth synth;
//ControlSwitcherExpSynth* global_pSynth;
//static BasicSynth synth;
//static SimpleInstrumentSynth synth;
//static StepSequencerSynth synth;
//StepSequencerExpSynth* global_pSynth;
StepSequencerBufferPlayerDynamicSynth* global_pSynth;
//StepSequencerBufferPlayerEffectExpSynth* global_pSynth;
//static EventsSynth synth;
//EventsExpSynth* global_pSynth;
//EventsExpBufferPlayerSynth* global_pSynth;
//static EventsXznnSynth synth;
//BufferPlayerExpSynth* global_pSynth;
//static ArbitraryTableLookupSynth synth;
//static BandlimitedOscillatorTestSynth synth;
//BandlimitedOscillatorExpSynth* global_pSynth;
//CompressorDuckingTestSynth* global_pSynth;
//static CompressorTestSynth synth;
//CompressorExpSynth* global_pSynth;
//static ControlSnapToScaleTestSynth synth;
//ControlSnapToScaleExpSynth* global_pSynth;
//static DelayTestSynth synth;
//DelayExpSynth* global_pSynth;
//FilteredNoiseSynth* global_pSynth; 
//FilterExpSynth* global_pSynth;
//static FMDroneSynth synth;
//FMDroneExpSynth* global_pSynth;
//static InputDemoSynth synth;
//LFNoiseTestSynth* global_pSynth;
//ReverbTestSynth* global_pSynth;
//SimpleStepSeqSynth* global_pSynth;
//SimpleStepSequencerBufferPlayerSynth* global_pSynth;
//SineSumSynth* global_pSynth;
//StereoDelayTestSynth* global_pSynth;
//static SynthsAsGeneratorsDemoSynth synth;
//static SynthsAsGeneratorsExpSynth* global_pSynth;
//XYSpeedSynth* global_pSynth;



// Select sample format. 
#if 1
#define PA_SAMPLE_TYPE  paFloat32
typedef float SAMPLE;
#define SAMPLE_SILENCE  (0.0f)
#define PRINTF_S_FORMAT "%.8f"
#elif 1
#define PA_SAMPLE_TYPE  paInt16
typedef short SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#elif 0
#define PA_SAMPLE_TYPE  paInt8
typedef char SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#else
#define PA_SAMPLE_TYPE  paUInt8
typedef unsigned char SAMPLE;
#define SAMPLE_SILENCE  (128)
#define PRINTF_S_FORMAT "%d"
#endif


std::map<string,int> global_inputdevicemap;
std::map<string,int> global_outputdevicemap;

/*
RtAudio global_dac;
RtAudio::StreamParameters global_rtParams;
*/

PaStream* global_stream;
PaStreamParameters global_inputParameters;
PaStreamParameters global_outputParameters;
PaError global_err;
string global_audioinputdevicename="";
string global_audiooutputdevicename="";
int global_inputAudioChannelSelectors[2];
int global_outputAudioChannelSelectors[2];
PaAsioStreamInfo global_asioInputInfo;
PaAsioStreamInfo global_asioOutputInfo;

FILE* pFILE= NULL;

// Global Variables:

#define MAX_LOADSTRING 100
FIBITMAP* global_dib;
HFONT global_hFont;
HWND global_hwnd=NULL;
MMRESULT global_timer=0;
//#define MAX_GLOBALTEXT	4096
//WCHAR global_text[MAX_GLOBALTEXT+1];
//int global_delay_ms=5000; //default to 5 seconds delay
float global_cutfreq_hz=1000.0f; //default to 1000.0 Hz
float global_duration_sec=180;
int global_x=100;
int global_y=200;
int global_xwidth=400;
int global_yheight=400;
BYTE global_alpha=200;
int global_fontheight=24;
int global_fontwidth=-1; //will be computed within WM_PAINT handler
int global_staticalignment = 0; //0 for left, 1 for center and 2 for right
int global_staticheight=-1; //will be computed within WM_SIZE handler
int global_staticwidth=-1; //will be computed within WM_SIZE handler 
//COLORREF global_statictextcolor=RGB(0xFF, 0xFF, 0xFF); //white
COLORREF global_statictextcolor=RGB(0xFF, 0x00, 0x00); //red
//spi, begin
int global_imageheight=-1; //will be computed within WM_SIZE handler
int global_imagewidth=-1; //will be computed within WM_SIZE handler 
//spi, end
int global_titlebardisplay=1; //0 for off, 1 for on
int global_acceleratoractive=0; //0 for off, 1 for on
int global_menubardisplay=0; //0 for off, 1 for on

DWORD global_startstamp_ms;
//FILE* global_pFILE=NULL;
string global_line;
std::ifstream global_ifstream;

#define IDC_MAIN_EDIT	100
#define IDC_MAIN_STATIC	101

HINSTANCE hInst;								// current instance
//TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
//TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
TCHAR szTitle[1024]={L"spitonicstepsequencerbufferplayersynthvoroguiwin32title"};					// The title bar text
TCHAR szWindowClass[1024]={L"spitonicstepsequencerbufferplayersynthvoroguiwin32class"};				// the main window class name

//new parameters
int global_statictextcolor_red=255;
int global_statictextcolor_green=0;
int global_statictextcolor_blue=0;
string global_begin="begin.ahk";
string global_end="end.ahk";

string global_samplesfolder = ".";
string global_samplesfilter = "*.wav";
//int global_maxsamples = 100;
int global_maxsamples = 25;
int global_numsteps = 8;
float global_initialbpm = 80.0;
string global_automode = "auto"; //or manual

POINTSET* global_pPOINTSET = NULL;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

bool global_abort=false;

/*
float CubicAmplifier( float input );
static int fuzzCallback( const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData );

//Non-linear amplifier with soft distortion curve.
float CubicAmplifier( float input )
{
    float output, temp;
    if( input < 0.0 )
    {
        temp = input + 1.0f;
        output = (temp * temp * temp) - 1.0f;
    }
    else
    {
        temp = input - 1.0f;
        output = (temp * temp * temp) + 1.0f;
    }

    return output;
}
#define FUZZ(x) CubicAmplifier(CubicAmplifier(CubicAmplifier(CubicAmplifier(x))))
*/

/*
// delayline.c 
//const int M = SAMPLE_RATE*global_delay_ms/1000; 
//static float D[M];           // initialized to zero

int M = SAMPLE_RATE*global_delay_ms/1000;
static float D[SAMPLE_RATE*60];           // initialized to zero
static long ptr=0;            // read-write offset

float delayline(float x)
{
	float y = D[ptr];          // read operation 
	D[ptr++] = x;               // write operation
	if (ptr >= M) { ptr -= M; } // wrap ptr if needed
	//    ptr %= M;                   // modulo-operator syntax
	return y;
}
*/
/*
//lowpass filter
float a0 = 0.0f;
float a1 = 0.0f;
float b1 = 0.0f;
*/
/*
void SetLPF(float fCut, float fSampling)
{
	a0 = fCut/(fSampling+fCut);
	a1 = a0;
	b1 = (fSampling-fCut)/(fSampling+fCut);
}
*/
/*
void SetLPF(float fCut, float fSampling)
{
	float w = 2.0 * fSampling;
	fCut *= 2.0f *3.14159265359;
	float Norm = 1.0/(fCut+w);
	a0 = fCut * Norm;
	a1 = a0;
	b1 = (w - fCut) * Norm;
}
*/

/*
// "1024" is the number of samples over which to fade parameter changes
Dsp::Filter* global_pDspFilter = new Dsp::SmoothedFilterDesign
    <Dsp::RBJ::Design::LowPass, 2> (1024);


Dsp::Params global_DspParams;
*/

/*
float* audioData[2];
*/
float RandomFloat(float a, float b)
{
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}

/*
int RandomInt(int lowest, int highest)
{
	int range = (highest - lowest) + 1;
	int random_integer = lowest + int(range*rand() / (RAND_MAX + 1.0));
	return random_integer;
}
*/

static int renderCallback( const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData );

static int gNumNoInputs = 0;
// This routine will be called by the PortAudio engine when audio is needed.
// It may be called at interrupt level on some machines so don't do anything
// that could mess up the system like calling malloc() or free().
//
static int renderCallback( const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData )
{
    SAMPLE *out = (SAMPLE*)outputBuffer;
    const SAMPLE *in = (const SAMPLE*)inputBuffer;
    unsigned int i;
    (void) timeInfo; // Prevent unused variable warnings.
    (void) statusFlags;
    (void) userData;

	if(global_abort==true) return paAbort;

	/*
	if( inputBuffer == NULL )
    {
        for( i=0; i<framesPerBuffer; i++ )
        {
            *out++ = 0;  // left - silent
            *out++ = 0;  // right - silent 
        }
        gNumNoInputs += 1;
    }
    else
    {
		for (i = 0; i<framesPerBuffer; i++)
		{
			*out++ = *in++;  // left - unprocessed 
			*out++ = *in++;  // right - unprocessed
		}
	}
    */

	
	//synth.fillBufferOfFloats((float*)outputBuffer, nBufferFrames, NUM_CHANNELS);
	global_pSynth->fillBufferOfFloats((float*)outputBuffer, framesPerBuffer, NUM_CHANNELS);
	
    return paContinue;
}

/*
int renderCallback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
	double streamTime, RtAudioStreamStatus status, void *userData)
{
	synth.fillBufferOfFloats((float*)outputBuffer, nBufferFrames, NUM_CHANNELS);
	return 0;
}
*/



bool SelectAudioInputDevice()
{
	const PaDeviceInfo* deviceInfo;
    int numDevices = Pa_GetDeviceCount();
    for( int i=0; i<numDevices; i++ )
    {
        deviceInfo = Pa_GetDeviceInfo( i );
		string devicenamestring = deviceInfo->name;
		global_inputdevicemap.insert(pair<string,int>(devicenamestring,i));
		if(pFILE) fprintf(pFILE,"id=%d, name=%s\n", i, devicenamestring.c_str());
	}

	int deviceid = Pa_GetDefaultInputDevice(); // default input device 
	std::map<string,int>::iterator it;
	it = global_inputdevicemap.find(global_audioinputdevicename);
	if(it!=global_inputdevicemap.end())
	{
		deviceid = (*it).second;
		//printf("%s maps to %d\n", global_audiodevicename.c_str(), deviceid);
		deviceInfo = Pa_GetDeviceInfo(deviceid);
		//assert(inputAudioChannelSelectors[0]<deviceInfo->maxInputChannels);
		//assert(inputAudioChannelSelectors[1]<deviceInfo->maxInputChannels);
	}
	else
	{
		//Pa_Terminate();
		//return -1;
		//printf("error, audio device not found, will use default\n");
		//MessageBox(win,"error, audio device not found, will use default\n",0,0);
		deviceid = Pa_GetDefaultInputDevice();
	}


	global_inputParameters.device = deviceid; 
	if (global_inputParameters.device == paNoDevice) 
	{
		//MessageBox(win,"error, no default input device.\n",0,0);
		return false;
	}
	//global_inputParameters.channelCount = 2;
	global_inputParameters.channelCount = NUM_CHANNELS;
	global_inputParameters.sampleFormat =  PA_SAMPLE_TYPE;
	global_inputParameters.suggestedLatency = Pa_GetDeviceInfo( global_inputParameters.device )->defaultLowOutputLatency;
	//inputParameters.hostApiSpecificStreamInfo = NULL;

	//Use an ASIO specific structure. WARNING - this is not portable. 
	//PaAsioStreamInfo asioInputInfo;
	global_asioInputInfo.size = sizeof(PaAsioStreamInfo);
	global_asioInputInfo.hostApiType = paASIO;
	global_asioInputInfo.version = 1;
	global_asioInputInfo.flags = paAsioUseChannelSelectors;
	global_asioInputInfo.channelSelectors = global_inputAudioChannelSelectors;
	if(deviceid==Pa_GetDefaultInputDevice())
	{
		global_inputParameters.hostApiSpecificStreamInfo = NULL;
	}
	else if(Pa_GetHostApiInfo(Pa_GetDeviceInfo(deviceid)->hostApi)->type == paASIO) 
	{
		global_inputParameters.hostApiSpecificStreamInfo = &global_asioInputInfo;
	}
	else if(Pa_GetHostApiInfo(Pa_GetDeviceInfo(deviceid)->hostApi)->type == paWDMKS) 
	{
		global_inputParameters.hostApiSpecificStreamInfo = NULL;
	}
	else
	{
		//assert(false);
		global_inputParameters.hostApiSpecificStreamInfo = NULL;
	}
	return true;
}



bool SelectAudioOutputDevice()
{
	const PaDeviceInfo* deviceInfo;
    int numDevices = Pa_GetDeviceCount();
    for( int i=0; i<numDevices; i++ )
    {
        deviceInfo = Pa_GetDeviceInfo( i );
		string devicenamestring = deviceInfo->name;
		global_outputdevicemap.insert(pair<string,int>(devicenamestring,i));
		if(pFILE) fprintf(pFILE,"id=%d, name=%s\n", i, devicenamestring.c_str());
	}

	int deviceid = Pa_GetDefaultOutputDevice(); // default output device 
	std::map<string,int>::iterator it;
	it = global_outputdevicemap.find(global_audiooutputdevicename);
	if(it!=global_outputdevicemap.end())
	{
		deviceid = (*it).second;
		//printf("%s maps to %d\n", global_audiodevicename.c_str(), deviceid);
		deviceInfo = Pa_GetDeviceInfo(deviceid);
		//assert(inputAudioChannelSelectors[0]<deviceInfo->maxInputChannels);
		//assert(inputAudioChannelSelectors[1]<deviceInfo->maxInputChannels);
	}
	else
	{
		//Pa_Terminate();
		//return -1;
		//printf("error, audio device not found, will use default\n");
		//MessageBox(win,"error, audio device not found, will use default\n",0,0);
		deviceid = Pa_GetDefaultOutputDevice();
	}


	global_outputParameters.device = deviceid; 
	if (global_outputParameters.device == paNoDevice) 
	{
		//MessageBox(win,"error, no default output device.\n",0,0);
		return false;
	}
	//global_inputParameters.channelCount = 2;
	global_outputParameters.channelCount = NUM_CHANNELS;
	global_outputParameters.sampleFormat =  PA_SAMPLE_TYPE;
	global_outputParameters.suggestedLatency = Pa_GetDeviceInfo( global_outputParameters.device )->defaultLowOutputLatency;
	//outputParameters.hostApiSpecificStreamInfo = NULL;

	//Use an ASIO specific structure. WARNING - this is not portable. 
	//PaAsioStreamInfo asioInputInfo;
	global_asioOutputInfo.size = sizeof(PaAsioStreamInfo);
	global_asioOutputInfo.hostApiType = paASIO;
	global_asioOutputInfo.version = 1;
	global_asioOutputInfo.flags = paAsioUseChannelSelectors;
	global_asioOutputInfo.channelSelectors = global_outputAudioChannelSelectors;
	if(deviceid==Pa_GetDefaultOutputDevice())
	{
		global_outputParameters.hostApiSpecificStreamInfo = NULL;
	}
	else if(Pa_GetHostApiInfo(Pa_GetDeviceInfo(deviceid)->hostApi)->type == paASIO) 
	{
		global_outputParameters.hostApiSpecificStreamInfo = &global_asioOutputInfo;
	}
	else if(Pa_GetHostApiInfo(Pa_GetDeviceInfo(deviceid)->hostApi)->type == paWDMKS) 
	{
		global_outputParameters.hostApiSpecificStreamInfo = NULL;
	}
	else
	{
		//assert(false);
		global_outputParameters.hostApiSpecificStreamInfo = NULL;
	}
	return true;
}




// Convert a wide Unicode string to an UTF8 string
std::string utf8_encode(const std::wstring &wstr)
{
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo( size_needed, 0 );
    WideCharToMultiByte                  (CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode(const std::string &str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo( size_needed, 0 );
    MultiByteToWideChar                  (CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

char buffer[1024];
struct ijpair {
	int _i;
	int _j;
	ijpair(int i, int j){ _i = i; _j = j; }
};
vector<ijpair> global_ijpairs;
void CALLBACK StartGlobalProcess(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	WavSetLib_Initialize(global_hwnd, IDC_MAIN_STATIC, global_staticwidth, global_staticheight, global_fontwidth, global_fontheight, global_staticalignment);

	DWORD nowstamp_ms = GetTickCount();
	while( (global_duration_sec<0.0f) || ((nowstamp_ms-global_startstamp_ms)/1000.0f)<global_duration_sec )
	{
		Sleep((int)(1000));
		
		/*
		//dynamically change filter parameter
		int random_integer;
		int lowest=1, highest=10; 
		int range=(highest-lowest)+1;
		random_integer = lowest+int(range*rand()/(RAND_MAX + 1.0));
		float cutfreq_hz = global_cutfreq_hz * random_integer*1.0f/range;
		*/

		/*
		global_DspParams[0] = SAMPLE_RATE; // sample rate
		global_DspParams[1] = cutfreq_hz; // cutoff frequency in Hz
		global_DspParams[2] = 1.25; // Q
		global_pDspFilter->setParams (global_DspParams);
		*/
		/*
		sprintf(buffer, "%f Hz\n", cutfreq_hz);
		StatusAddText(buffer);
		*/
		
		/*
		///////////////////////////////
		//simple instrument synth begin
		static int twoOctavePentatonicScale[10] = { 0, 2, 4, 7, 9, 12, 14, 16, 19, 21 };

		// set a parameter that we created when we defined the synth
		//synth.setParameter("midiNumber", 44 + twoOctavePentatonicScale[random_integer-1]);
		synth.setParameter("midiNumber", 56 + twoOctavePentatonicScale[random_integer - 1]);

		// simply setting the value of a parameter causes that parameter to send a "trigger" message to any using them as triggers
		synth.setParameter("trigger", 1);
		//simple instrument synth end
		/////////////////////////////
		*/

		//////////////////////////////////
		//control switcher exp synth begin
		/*
		if(random_integer>5)
		synth.setParameter("clickVol", 0.0, true);
		else
		synth.setParameter("clickVol", 1.0, true);
		*/
		//synth.setParameter("bpm", cutfreq_hz);
		//synth.setParameter("freq", cutfreq_hz);
		//synth.setParameter("addtlFM", random_integer*10);
		//control switcher exp synth end
		//////////////////////////////////

		///////////////////////
		//fm drone synth begin
		//synth.setParameter("carrierPitch", 20+random_integer );
		//synth.setParameter("modIndex", random_integer/10.f);
		//synth.setParameter("lfoAmt", random_integer / 10.f);
		//fm drone synth end
		///////////////////////

		/*
		///////////////////////
		//StepSequencerBufferPlayerExpSynth synth begin
		//bass drum
		global_pSynth->setParameter("track1volume1", randomFloat(0.0, 1.0));
		Sleep((int)(100));
		global_pSynth->setParameter("track1volume2", randomFloat(0.0, 1.0));
		//Sleep((int)(100));
		//global_pSynth->setParameter("track1volume3", randomFloat(0.0, 1.0));
		Sleep((int)(100));
		global_pSynth->setParameter("track1volume4", randomFloat(0.0, 1.0));
		Sleep((int)(100));
		global_pSynth->setParameter("track1volume5", randomFloat(0.0, 1.0));
		//Sleep((int)(100));
		//global_pSynth->setParameter("track1volume6", randomFloat(0.0, 1.0));
		Sleep((int)(100));
		global_pSynth->setParameter("track1volume7", randomFloat(0.0, 1.0));
		Sleep((int)(100));
		global_pSynth->setParameter("track1volume8", randomFloat(0.0, 1.0));

		//hit hat
		global_pSynth->setParameter("track2volume1", randomFloat(0.0, 1.0));
		Sleep((int)(100));
		//global_pSynth->setParameter("track2volume2", randomFloat(0.0, 1.0));
		//Sleep((int)(100));
		global_pSynth->setParameter("track2volume3", randomFloat(0.0, 1.0));
		Sleep((int)(100));
		//global_pSynth->setParameter("track2volume4", randomFloat(0.0, 1.0));
		//Sleep((int)(100));
		global_pSynth->setParameter("track2volume5", randomFloat(0.0, 1.0));
		Sleep((int)(100));
		global_pSynth->setParameter("track2volume6", randomFloat(0.0, 1.0));
		Sleep((int)(100));
		global_pSynth->setParameter("track2volume7", randomFloat(0.0, 1.0));
		Sleep((int)(100));
		global_pSynth->setParameter("track2volume8", randomFloat(0.0, 1.0));
		if (1)
		{
			//BuildTriangleNetwork(global_pPOINTSET);
			//ComputeAllTriangleCenters(global_pPOINTSET);
		}
		//RedrawWindow(global_hwnd, NULL, NULL, RDW_INVALIDATE);
		//PostMessage(global_hwnd, WM_PAINT, 0, 0);
		//StepSequencerBufferPlayerExpSynth synth end
		///////////////////////
		*/

		if (global_automode=="auto")
		{
			///////////////////////////////////////////////////
			//StepSequencerBufferPlayerDynamicSynth synth begin
			//0.0) wait for synth to be initialized
			while (!global_pSynth->_ready) Sleep(10);
			int ntrack = global_pSynth->_ntrack;
			int nstep = global_pSynth->_nstep;
			//0.1) turn off remaining volumes
			for (vector<ijpair>::iterator it = global_ijpairs.begin(); it != global_ijpairs.end(); ++it)
			{
				int i = (*it)._i;
				int j = (*it)._j;
				sprintf(buffer, "t%i_v%i", i, j);
				global_pSynth->setParameter(buffer, 0.0);
			}
			//0.2) empty vector
			global_ijpairs.clear();

			//1) turn on initial volumes
			for (int ii = 0; ii < 10; ii++)
			{
				int i = RandomInt(0, ntrack - 1);
				int j = RandomInt(0, nstep - 1);
				sprintf(buffer, "t%i_v%i", i, j);
				global_pSynth->setParameter(buffer, 1.0);
				global_ijpairs.push_back(ijpair(i, j));
			}
			//2) sleep and change one volume on and one volume off
			for (int ii = 0; ii < 30; ii++)
			{
				Sleep(2000);
				//add sound
				int i = RandomInt(0, ntrack - 1);
				int j = RandomInt(0, nstep - 1);
				sprintf(buffer, "t%i_v%i", i, j);
				global_pSynth->setParameter(buffer, 1.0);
				global_ijpairs.push_back(ijpair(i, j));
				//remove sound
				vector<ijpair>::iterator it = global_ijpairs.begin();
				i = (*it)._i;
				j = (*it)._j;
				sprintf(buffer, "t%i_v%i", i, j);
				global_pSynth->setParameter(buffer, 0.0);
				global_ijpairs.erase(it);
			}
			//StepSequencerBufferPlayerDynamicSynth synth end
			///////////////////////////////////////////////////
		}


		/////////////////////////////////////
		//bandlimited oscillator synth begin
		//synth.setParameter("blend", random_integer / 10.f);
		//bandlimited oscillator synth end
		/////////////////////////////////////
		

		nowstamp_ms = GetTickCount();
	}
	PostMessage(global_hwnd, WM_DESTROY, 0, 0);
}




PCHAR*
    CommandLineToArgvA(
        PCHAR CmdLine,
        int* _argc
        )
    {
        PCHAR* argv;
        PCHAR  _argv;
        ULONG   len;
        ULONG   argc;
        CHAR   a;
        ULONG   i, j;

        BOOLEAN  in_QM;
        BOOLEAN  in_TEXT;
        BOOLEAN  in_SPACE;

        len = strlen(CmdLine);
        i = ((len+2)/2)*sizeof(PVOID) + sizeof(PVOID);

        argv = (PCHAR*)GlobalAlloc(GMEM_FIXED,
            i + (len+2)*sizeof(CHAR));

        _argv = (PCHAR)(((PUCHAR)argv)+i);

        argc = 0;
        argv[argc] = _argv;
        in_QM = FALSE;
        in_TEXT = FALSE;
        in_SPACE = TRUE;
        i = 0;
        j = 0;

        while( a = CmdLine[i] ) {
            if(in_QM) {
                if(a == '\"') {
                    in_QM = FALSE;
                } else {
                    _argv[j] = a;
                    j++;
                }
            } else {
                switch(a) {
                case '\"':
                    in_QM = TRUE;
                    in_TEXT = TRUE;
                    if(in_SPACE) {
                        argv[argc] = _argv+j;
                        argc++;
                    }
                    in_SPACE = FALSE;
                    break;
                case ' ':
                case '\t':
                case '\n':
                case '\r':
                    if(in_TEXT) {
                        _argv[j] = '\0';
                        j++;
                    }
                    in_TEXT = FALSE;
                    in_SPACE = TRUE;
                    break;
                default:
                    in_TEXT = TRUE;
                    if(in_SPACE) {
                        argv[argc] = _argv+j;
                        argc++;
                    }
                    _argv[j] = a;
                    j++;
                    in_SPACE = FALSE;
                    break;
                }
            }
            i++;
        }
        _argv[j] = '\0';
        argv[argc] = NULL;

        (*_argc) = argc;
        return argv;
    }

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	global_startstamp_ms = GetTickCount();

	//LPWSTR *szArgList;
	LPSTR *szArgList;
	int nArgs;
	int i;

	//szArgList = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	szArgList = CommandLineToArgvA(GetCommandLineA(), &nArgs);
	if( NULL == szArgList )
	{
		//wprintf(L"CommandLineToArgvW failed\n");
		return FALSE;
	}
	LPWSTR *szArgListW;
	int nArgsW;
	szArgListW = CommandLineToArgvW(GetCommandLineW(), &nArgsW);
	if( NULL == szArgListW )
	{
		//wprintf(L"CommandLineToArgvW failed\n");
		return FALSE;
	}

	global_audioinputdevicename="E-MU ASIO"; //"Wave (2- E-MU E-DSP Audio Proce"
	if(nArgs>1)
	{
		//global_filename = szArgList[1];
		global_audioinputdevicename = szArgList[1]; 
	}
	global_inputAudioChannelSelectors[0] = 0; // on emu patchmix ASIO device channel 1 (left)
	global_inputAudioChannelSelectors[1] = 1; // on emu patchmix ASIO device channel 2 (right)
	//global_inputAudioChannelSelectors[0] = 2; // on emu patchmix ASIO device channel 3 (left)
	//global_inputAudioChannelSelectors[1] = 3; // on emu patchmix ASIO device channel 4 (right)
	//global_inputAudioChannelSelectors[0] = 8; // on emu patchmix ASIO device channel 9 (left)
	//global_inputAudioChannelSelectors[1] = 9; // on emu patchmix ASIO device channel 10 (right)
	//global_inputAudioChannelSelectors[0] = 10; // on emu patchmix ASIO device channel 11 (left)
	//global_inputAudioChannelSelectors[1] = 11; // on emu patchmix ASIO device channel 12 (right)
	if(nArgs>2)
	{
		global_inputAudioChannelSelectors[0]=atoi((LPCSTR)(szArgList[2])); //0 for first asio channel (left) or 2, 4, 6, etc.
	}
	if(nArgs>3)
	{
		global_inputAudioChannelSelectors[1]=atoi((LPCSTR)(szArgList[3])); //1 for second asio channel (right) or 3, 5, 7, etc.
	}
	global_audiooutputdevicename="E-MU ASIO"; //"Wave (2- E-MU E-DSP Audio Proce"
	if(nArgs>4)
	{
		//global_filename = szArgList[1];
		global_audiooutputdevicename = szArgList[4]; 
	}
	global_outputAudioChannelSelectors[0] = 0; // on emu patchmix ASIO device channel 1 (left)
	global_outputAudioChannelSelectors[1] = 1; // on emu patchmix ASIO device channel 2 (right)
	//global_outputAudioChannelSelectors[0] = 2; // on emu patchmix ASIO device channel 3 (left)
	//global_outputAudioChannelSelectors[1] = 3; // on emu patchmix ASIO device channel 4 (right)
	//global_outputAudioChannelSelectors[0] = 8; // on emu patchmix ASIO device channel 9 (left)
	//global_outputAudioChannelSelectors[1] = 9; // on emu patchmix ASIO device channel 10 (right)
	//global_outputAudioChannelSelectors[0] = 10; // on emu patchmix ASIO device channel 11 (left)
	//global_outputAudioChannelSelectors[1] = 11; // on emu patchmix ASIO device channel 12 (right)
	if(nArgs>5)
	{
		global_outputAudioChannelSelectors[0]=atoi((LPCSTR)(szArgList[5])); //0 for first asio channel (left) or 2, 4, 6, etc.
	}
	if(nArgs>6)
	{
		global_outputAudioChannelSelectors[1]=atoi((LPCSTR)(szArgList[6])); //1 for second asio channel (right) or 3, 5, 7, etc.
	}

	
	if(nArgs>7)
	{
		global_cutfreq_hz = atof(szArgList[7]); 
	}
	if(nArgs>8)
	{
		global_duration_sec = atof(szArgList[8]);
	}


	if(nArgs>9)
	{
		global_x = atoi(szArgList[9]);
	}
	if(nArgs>10)
	{
		global_y = atoi(szArgList[10]);
	}
	if(nArgs>11)
	{
		global_xwidth = atoi(szArgList[11]);
	}
	if(nArgs>12)
	{
		global_yheight = atoi(szArgList[12]);
	}
	if(nArgs>13)
	{
		global_alpha = atoi(szArgList[13]);
	}
	if(nArgs>14)
	{
		global_titlebardisplay = atoi(szArgList[14]);
	}
	if(nArgs>15)
	{
		global_menubardisplay = atoi(szArgList[15]);
	}
	if(nArgs>16)
	{
		global_acceleratoractive = atoi(szArgList[16]);
	}
	if(nArgs>17)
	{
		global_fontheight = atoi(szArgList[17]);
	}

	//new parameters
	if(nArgs>18)
	{
		global_statictextcolor_red = atoi(szArgList[18]);
	}
	if(nArgs>19)
	{
		global_statictextcolor_green = atoi(szArgList[19]);
	}
	if(nArgs>20)
	{
		global_statictextcolor_blue = atoi(szArgList[20]);
	}
	if(nArgs>21)
	{
		wcscpy(szWindowClass, szArgListW[21]); 
	}
	if(nArgs>22)
	{
		wcscpy(szTitle, szArgListW[22]); 
	}
	if(nArgs>23)
	{
		global_begin = szArgList[23]; 
	}
	if(nArgs>24)
	{
		global_end = szArgList[24];
	}
	if (nArgs>25)
	{
		global_samplesfolder = szArgList[25];
	}
	if (nArgs>26)
	{
		global_samplesfilter = szArgList[26];
	}
	if (nArgs>27)
	{
		global_maxsamples = atoi(szArgList[27]);
	}
	if (nArgs>28)
	{
		global_numsteps = atoi(szArgList[28]);
	}
	if (nArgs>29)
	{
		global_initialbpm = atof(szArgList[29]);
	}
	if (nArgs>30)
	{
		global_automode = szArgList[30];
	}

	global_statictextcolor = RGB(global_statictextcolor_red, global_statictextcolor_green, global_statictextcolor_blue);
	LocalFree(szArgList);
	LocalFree(szArgListW);

	int nShowCmd = false;
	//ShellExecuteA(NULL, "open", "begin.bat", "", NULL, nShowCmd);
	ShellExecuteA(NULL, "open", global_begin.c_str(), "", NULL, nCmdShow);


	//////////////////////////
	//initialize random number
	//////////////////////////
	//srand((unsigned)time(0));
	srand((unsigned int)GetTickCount());



	pFILE = fopen("devices.txt","w");
	
	///////////////////////
	//initialize port audio
	///////////////////////
    global_err = Pa_Initialize();
    if( global_err != paNoError )
	{
		//MessageBox(0,"portaudio initialization failed",0,MB_ICONERROR);
		if(pFILE) fprintf(pFILE, "portaudio initialization failed.\n");
		fclose(pFILE);
		return 1;
	}
	

	
	////////////////////////
	//audio device selection
	////////////////////////
	//SelectAudioInputDevice();
	SelectAudioOutputDevice();
	
	/*
	// Create an api map.
	std::map<int, std::string> apiMap;
	apiMap[RtAudio::MACOSX_CORE] = "OS-X Core Audio";
	apiMap[RtAudio::WINDOWS_ASIO] = "Windows ASIO";
	apiMap[RtAudio::WINDOWS_DS] = "Windows Direct Sound";
	apiMap[RtAudio::UNIX_JACK] = "Jack Client";
	apiMap[RtAudio::LINUX_ALSA] = "Linux ALSA";
	apiMap[RtAudio::LINUX_OSS] = "Linux OSS";
	apiMap[RtAudio::RTAUDIO_DUMMY] = "RtAudio Dummy";

	std::vector< RtAudio::Api > apis;
	RtAudio::getCompiledApi(apis);

	fprintf(pFILE, "\nCompiled APIs:\n");
	for (unsigned int i = 0; i<apis.size(); i++)
		fprintf(pFILE, " %s\n ", apiMap[apis[i]] );


	fprintf(pFILE, "\nCurrent API: %s\n", apiMap[global_dac.getCurrentApi()]);

	unsigned int devices = global_dac.getDeviceCount();
	fprintf(pFILE, "\nFound %i device(s) ...\n", devices);
	*/

	/*
	// Configure RtAudio
	global_rtParams.deviceId = global_dac.getDefaultOutputDevice();
	global_rtParams.nChannels = NUM_CHANNELS;
	unsigned int sampleRate = SAMPLE_RATE;
	unsigned int bufferFrames = 512; // 512 sample frames
	*/

	// You don't necessarily have to do this - it will default to 44100 if not set.
	Tonic::setSampleRate(SAMPLE_RATE);

	global_pSynth = new StepSequencerBufferPlayerDynamicSynth(global_samplesfolder, global_samplesfilter, global_maxsamples, global_numsteps, global_initialbpm);

	VOROGUI_Init(global_pSynth);

	/* //would need something like this for vorogui
	vector<ControlParameter> params = global_pSynth->getParameters();
	for (unsigned int i = 0; i < params.size(); i++)
	{
		TonicFloat min = params[i].getMin();
		TonicFloat max = params[i].getMax();
		TonicFloat value = params[i].getValue();
		string mystring = params[i].getName();
	}
	*/
	//global_pPOINTSET = VOROGUI_CreatePointset();
	global_pPOINTSET = VOROGUI_ReadFromDisk();

	/////////////////////
	//init lowpass filter
	/////////////////////
	/*
	SetLPF(global_cutfreq_hz, SAMPLE_RATE);
	*/
	/*
	global_DspParams[0] = SAMPLE_RATE; // sample rate
	global_DspParams[1] = global_cutfreq_hz; // cutoff frequency in Hz
	global_DspParams[2] = 1.25; // Q

	global_pDspFilter->setParams (global_DspParams);
	*/

	/*
	audioData[0] = new float[10000]; //usually, buffer is much smaller than 10000
	audioData[1] = new float[10000];
	*/

	
	//////////////
    //setup stream  
	//////////////
    global_err = Pa_OpenStream(
        &global_stream,
		NULL, //NULL, //&global_inputParameters,
        &global_outputParameters,
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        0, //paClipOff,      // we won't output out of range samples so don't bother clipping them
        renderCallback,  
        NULL ); //no callback userData
    if( global_err != paNoError ) 
	{
		char errorbuf[2048];
        sprintf(errorbuf, "Unable to open stream: %s\n", Pa_GetErrorText(global_err));
		//MessageBox(0,errorbuf,0,MB_ICONERROR);
		if(pFILE) fprintf(pFILE, "%s\n", errorbuf);
		fclose(pFILE);
        return 1;
    }
	

	
	//////////////
    //start stream  
	//////////////
    global_err = Pa_StartStream( global_stream );
    if( global_err != paNoError ) 
	{
		char errorbuf[2048];
        sprintf(errorbuf, "Unable to start stream: %s\n", Pa_GetErrorText(global_err));
		//MessageBox(0,errorbuf,0,MB_ICONERROR);
		if(pFILE) fprintf(pFILE, "%s\n", errorbuf);
        fclose(pFILE);
		return 1;
    }
	

	/*
	// open rtaudio stream
	try {
		global_dac.openStream(&global_rtParams, NULL, RTAUDIO_FLOAT32, sampleRate, &bufferFrames, &renderCallback, NULL, NULL);

		global_dac.startStream();

		// hacky, yes, but let's just hang out for a while until someone presses a key
		//printf("\n\nPress Enter to stop\n\n");
		//cin.get();
		//dac.stopStream();
	}
	catch (RtError& e) {
		std::cout << '\n' << e.getMessage() << '\n' << std::endl;
		exit(0);
	}
	*/

	/*
	fclose(pFILE);
	pFILE=NULL;
	*/

	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	//LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	//LoadString(hInstance, IDC_SPIWAVWIN32, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	if(global_acceleratoractive)
	{
		hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SPIWAVWIN32));
	}
	else
	{
		hAccelTable = NULL;
	}
	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	//wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SPIWAVWIN32));
	wcex.hIcon			= (HICON)LoadImage(NULL, L"background_32x32x16.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);

	if(global_menubardisplay)
	{
		wcex.lpszMenuName = MAKEINTRESOURCE(IDC_SPIWAVWIN32); //original with menu
	}
	else
	{
		wcex.lpszMenuName = NULL; //no menu
	}
	wcex.lpszClassName	= szWindowClass;
	//wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	wcex.hIconSm		= (HICON)LoadImage(NULL, L"background_16x16x16.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // Store instance handle in our global variable

	global_dib = FreeImage_Load(FIF_JPEG, "background.jpg", JPEG_DEFAULT);


	FIBITMAP* local_16x16xrgbdib = FreeImage_Rescale(global_dib, 16, 16, FILTER_BICUBIC);
	FreeImage_Save(FIF_ICO, local_16x16xrgbdib, "background_16x16xrgb-new.ico");
	FreeImage_Unload(local_16x16xrgbdib);

	FIBITMAP* local_32x32xrgbdib = FreeImage_Rescale(global_dib, 32, 32, FILTER_BICUBIC);
	FreeImage_Save(FIF_ICO, local_32x32xrgbdib, "background_32x32xrgb-new.ico");
	FreeImage_Unload(local_32x32xrgbdib);

	FIBITMAP* local_48x48xrgbdib = FreeImage_Rescale(global_dib, 48, 48, FILTER_BICUBIC);
	FreeImage_Save(FIF_ICO, local_48x48xrgbdib, "background_48x48xrgb-new.ico");
	FreeImage_Unload(local_48x48xrgbdib);


	//global_hFont=CreateFontW(global_fontheight,0,0,0,FW_NORMAL,0,0,0,0,0,0,2,0,L"SYSTEM_FIXED_FONT");
	global_hFont=CreateFontW(global_fontheight,0,0,0,FW_NORMAL,0,0,0,0,0,0,2,0,L"Segoe Script");

	if(global_titlebardisplay)
	{
		hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, //original with WS_CAPTION etc.
			global_x, global_y, global_xwidth, global_yheight, NULL, NULL, hInstance, NULL);
	}
	else
	{
		hWnd = CreateWindow(szWindowClass, szTitle, WS_POPUP | WS_VISIBLE, //no WS_CAPTION etc.
			global_x, global_y, global_xwidth, global_yheight, NULL, NULL, hInstance, NULL);
	}
	if (!hWnd)
	{
		return FALSE;
	}
	global_hwnd = hWnd;

	SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(hWnd, 0, global_alpha, LWA_ALPHA);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	global_timer=timeSetEvent(500,25,(LPTIMECALLBACK)&StartGlobalProcess,0,TIME_ONESHOT);
	return TRUE;
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	HGDIOBJ hOldBrush;
	HGDIOBJ hOldPen;
	int iOldMixMode;
	COLORREF crOldBkColor;
	COLORREF crOldTextColor;
	int iOldBkMode;
	HFONT hOldFont, hFont;
	TEXTMETRIC myTEXTMETRIC;

	switch (message)
	{
	case WM_CREATE:
		{
			/*
			HWND hStatic = CreateWindowEx(WS_EX_TRANSPARENT, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_CENTER,  
				0, 100, 100, 100, hWnd, (HMENU)IDC_MAIN_STATIC, GetModuleHandle(NULL), NULL);
			*/
			HWND hStatic = CreateWindowEx(WS_EX_TRANSPARENT, L"STATIC", L"", WS_CHILD | WS_VISIBLE | global_staticalignment,  
				0, 100, 100, 100, hWnd, (HMENU)IDC_MAIN_STATIC, GetModuleHandle(NULL), NULL);
			if(hStatic == NULL)
				MessageBox(hWnd, L"Could not create static text.", L"Error", MB_OK | MB_ICONERROR);
			SendMessage(hStatic, WM_SETFONT, (WPARAM)global_hFont, MAKELPARAM(FALSE, 0));
		}
		break;
	case WM_SIZE:
		{
			RECT rcClient;
			GetClientRect(hWnd, &rcClient);
			HWND hStatic = GetDlgItem(hWnd, IDC_MAIN_STATIC);
			global_staticwidth = rcClient.right - 0;
			//global_staticheight = rcClient.bottom-(rcClient.bottom/2);
			global_staticheight = rcClient.bottom-0;
			//spi, begin
			global_imagewidth = rcClient.right - 0;
			global_imageheight = rcClient.bottom - 0; 
			WavSetLib_Initialize(global_hwnd, IDC_MAIN_STATIC, global_staticwidth, global_staticheight, global_fontwidth, global_fontheight, global_staticalignment);
			//spi, end
			//SetWindowPos(hStatic, NULL, 0, rcClient.bottom/2, global_staticwidth, global_staticheight, SWP_NOZORDER);
			SetWindowPos(hStatic, NULL, 0, 0, global_staticwidth, global_staticheight, SWP_NOZORDER);
		}
		break;
	case WM_CTLCOLOREDIT:
		{
			SetBkMode((HDC)wParam, TRANSPARENT);
			SetTextColor((HDC)wParam, RGB(0xFF, 0xFF, 0xFF));
			return (INT_PTR)::GetStockObject(NULL_PEN);
		}
		break;
	case WM_CTLCOLORSTATIC:
		{
			SetBkMode((HDC)wParam, TRANSPARENT);
			//SetTextColor((HDC)wParam, RGB(0xFF, 0xFF, 0xFF));
			SetTextColor((HDC)wParam, global_statictextcolor);
			return (INT_PTR)::GetStockObject(NULL_PEN);
		}
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		{
			hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code here...
			SetStretchBltMode(hdc, COLORONCOLOR);
			//spi, begin
			/*
			StretchDIBits(hdc, 0, 0, global_xwidth, global_yheight,
			0, 0, FreeImage_GetWidth(global_dib), FreeImage_GetHeight(global_dib),
			FreeImage_GetBits(global_dib), FreeImage_GetInfo(global_dib), DIB_RGB_COLORS, SRCCOPY);
			*/
			StretchDIBits(hdc, 0, 0, global_imagewidth, global_imageheight,
				0, 0, FreeImage_GetWidth(global_dib), FreeImage_GetHeight(global_dib),
				FreeImage_GetBits(global_dib), FreeImage_GetInfo(global_dib), DIB_RGB_COLORS, SRCCOPY);
			//spi, end
			hOldBrush = SelectObject(hdc, (HBRUSH)GetStockObject(GRAY_BRUSH));
			//hOldPen = SelectObject(hdc, (HPEN)GetStockObject(WHITE_PEN)); //original
			hOldPen = SelectObject(hdc, (HPEN)GetStockObject(WHITE_PEN)); //spi
			//iOldMixMode = SetROP2(hdc, R2_MASKPEN);
			iOldMixMode = SetROP2(hdc, R2_MERGEPEN); //original
			//iOldMixMode = SetROP2(hdc, R2_MERGEPENNOT); //spi
			//iOldMixMode = SetROP2(hdc, R2_MERGENOTPEN); //spi
			//Rectangle(hdc, 100, 100, 200, 200);



			//crOldBkColor = SetBkColor(hdc, RGB(0xFF, 0x00, 0x00)); //original
			crOldBkColor = SetBkColor(hdc, RGB(0x00, 0x00, 0x00)); //spi
			crOldTextColor = SetTextColor(hdc, RGB(0xFF, 0xFF, 0xFF));
			iOldBkMode = SetBkMode(hdc, TRANSPARENT);




			//hFont=CreateFontW(70,0,0,0,FW_BOLD,0,0,0,0,0,0,2,0,L"SYSTEM_FIXED_FONT");
			//hOldFont=(HFONT)SelectObject(hdc,global_hFont);
			hOldFont = (HFONT)SelectObject(hdc, global_hFont);
			GetTextMetrics(hdc, &myTEXTMETRIC);
			global_fontwidth = myTEXTMETRIC.tmAveCharWidth;
			//TextOutW(hdc, 100, 100, L"test string", 11);


			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			HDC tempHdc = CreateCompatibleDC(hdc);
			// create a bitmap of a size you need, let's say it 100x100
			HBITMAP canvas = CreateCompatibleBitmap(hdc, global_imagewidth, global_imageheight);
			// select new bitmap into context, don't forget to save old bitmap handle
			HBITMAP oldBmp = (HBITMAP)SelectObject(tempHdc, canvas);

			//BLENDFUNCTION blend = { AC_SRC_OVER, 0, 127, AC_SRC_ALPHA };
			BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
			//BLENDFUNCTION blend = { AC_SRC_OVER, 0, 64, AC_SRC_ALPHA };
			//BLENDFUNCTION blend = { AC_SRC_OVER, 0, 32, AC_SRC_ALPHA };

			//SetDCPenColor(tempHdc, RGB(255, 0, 0));
			//SetDCBrushColor(tempHdc, RGB(255, 0, 0));
			//Rectangle(tempHdc, dim.left, dim.top, dim.right, dim.bottom);
			if (global_pPOINTSET) VOROGUI_DrawPointset(global_pPOINTSET, tempHdc);

			bool res = AlphaBlend(hdc, 0, 0, global_imagewidth, global_imageheight, tempHdc, 0, 0, global_imagewidth, global_imageheight, blend);
			// reset the old bitmap
			SelectObject(tempHdc, oldBmp);
			// canvas is no longer needed and should be deleted to avoid GDI leaks
			DeleteObject(canvas);
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			SelectObject(hdc, hOldBrush);
			SelectObject(hdc, hOldPen);
			SetROP2(hdc, iOldMixMode);
			SetBkColor(hdc, crOldBkColor);
			SetTextColor(hdc, crOldTextColor);
			SetBkMode(hdc, iOldBkMode);
			SelectObject(hdc, hOldFont);
			//DeleteObject(hFont);
			EndPaint(hWnd, &ps);
		}
		break;
	case WM_LBUTTONDOWN:
		{
			VOROGUI_OnLButtonDown(global_pPOINTSET, hWnd, wParam, lParam);
		}
		break;
	case WM_LBUTTONUP:
		{
			VOROGUI_OnLButtonUp(global_pPOINTSET, hWnd, wParam, lParam);
		}
		break;
	case WM_RBUTTONUP:
		{
			VOROGUI_OnRButtonUp(global_pPOINTSET, hWnd, wParam, lParam);
		}
		break;
	case WM_KEYDOWN:
		{
			if (wParam == 'S')
			{
				VOROGUI_WriteToDisk(global_pPOINTSET);
			}
		}
		break;
	case WM_DESTROY:
		{
			if (global_timer) timeKillEvent(global_timer);
			global_abort=true;
			//Sleep(2000); //should be the same as the sleep time in the StartGlobalProcess() callback function
			WavSetLib_Terminate();
			FreeImage_Unload(global_dib);
			DeleteObject(global_hFont);
			//if(global_pFILE) fclose(global_pFILE);
			global_ifstream.close();

			if (pFILE) fclose(pFILE); //added by spi

			/*
			delete global_pDspFilter;
			*/
			/*
			delete[] audioData[0];
			delete[] audioData[1];
			*/

			/*
			global_dac.stopStream();
			*/
			//spi, begin
			/////////////////////
			//terminate portaudio
			/////////////////////
			global_err = Pa_StopStream(global_stream);
			if (global_err != paNoError)
			{
				char errorbuf[2048];
				sprintf(errorbuf, "Error stoping stream: %s\n", Pa_GetErrorText(global_err));
				MessageBoxA(0, errorbuf, 0, MB_ICONERROR);
				return 1;
			}
			global_err = Pa_CloseStream(global_stream);
			if (global_err != paNoError)
			{
				char errorbuf[2048];
				sprintf(errorbuf, "Error closing stream: %s\n", Pa_GetErrorText(global_err));
				MessageBoxA(0, errorbuf, 0, MB_ICONERROR);
				return 1;
			}
			Pa_Terminate();
			//spi, end

			if (global_pSynth) delete global_pSynth;
			VOROGUI_DestroyPointset(global_pPOINTSET);
			//int nShowCmd = false;
			//ShellExecuteA(NULL, "open", "end.bat", "", NULL, nShowCmd);
			ShellExecuteA(NULL, "open", global_end.c_str(), "", NULL, 0);
			PostQuitMessage(0);
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
