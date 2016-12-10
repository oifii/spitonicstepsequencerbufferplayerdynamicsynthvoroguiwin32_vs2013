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

#ifndef StepSequencerBufferPlayerDynamicSynth_h
#define StepSequencerBufferPlayerDynamicSynth_h

#include "Tonic.h"
#include <functional>

#include <sndfile.hh>

using namespace Tonic;

#define INT_TO_STRING( x ) dynamic_cast< std::ostringstream & >( \
	(std::ostringstream() << std::dec << x)).str()

int RandomInt(int lowest, int highest)
{
	int range = (highest - lowest) + 1;
	int random_integer = lowest + int(range*rand() / (RAND_MAX + 1.0));
	return random_integer;
}

class StepSequencerBufferPlayerDynamicSynth : public Synth{
public:
	bool _ready;
	int _ntrack;
	int _nstep;
private:
	vector<string> _samplefilenames;
	//int _ntrack;
	ControlSwitcher* _ptrackvolumes;
	Generator* _ptrack;
	SampleTable** _ppbuffer;
	BufferPlayer* _pplayer;
	ControlGenerator** _pptrackvolume;

public:
	StepSequencerBufferPlayerDynamicSynth(string samplesfolder, string samplesfilter, int maxsamples, int numsteps, float initialbpm)
	{

		//#if TONIC_HAS_CPP_11
#if 1
		_ready = false;
		///////////////////////////
		//populate sample filenames
		///////////////////////////
		if (samplesfolder != "" && samplesfilter != "")
		{
			//1) execute cmd line to get all folder's image filenames
			string quote = "\"";
			string pathfilter;
			string path = samplesfolder;
			//pathfilter = path + "\\*.bmp";
			pathfilter = path + "\\" + samplesfilter;
			string systemcommand;
			//systemcommand = "DIR " + quote + pathfilter + quote + "/B /O:N > wsic_filenames.txt"; //wsip tag standing for wav set (library) instrumentset (class) populate (function)
			systemcommand = "DIR " + quote + pathfilter + quote + "/B /S /O:N > spitssbps_filenames.txt"; // /S for adding path into "spiwtmvs_filenames.txt"
			system(systemcommand.c_str());
			//2) load in all "spiwtmvs_filenames.txt" file
			//Sleep(1000);
			//vector<string> global_imagefilenames;
			ifstream ifs("spitssbps_filenames.txt");
			string temp;
			while (getline(ifs, temp))
			{
				//txtfilenames.push_back(path + "\\" + temp);
				_samplefilenames.push_back(temp);
			}
		}

		char charbuf[256];

		//const int NUM_STEPS = 8;
		_nstep = numsteps;

		// synth paramters are like instance variables -- they're values you can set later, by calling synth.setParameter()
		//ControlGenerator bpm = addParameter("tempo", 80).min(50).max(300);
		ControlGenerator bpm = addParameter("tempo", initialbpm).min(1).max(200);

		// ControlMetro generates a "trigger" message at a given bpm. We multiply it by four because we
		// want four 16th notes for every beat
		//ControlGenerator metro = ControlMetro().bpm(4 * bpm);
		ControlGenerator metro = ControlMetro().bpm((_nstep/2.0) * bpm);

		// ControlStepper increments a value every time it's triggered, and then starts at the beginning again
		// Here, we're using it to move forward in the sequence
		ControlGenerator step = ControlStepper().end(_nstep).trigger(metro);
		
		// ControlSwitcher holds a list of ControlGenerators, and routes whichever one the inputIndex is pointing
		// to to its output.
		//ControlSwitcher track1volumes = ControlSwitcher().inputIndex(step);
		_ntrack = _samplefilenames.size();
		if (_ntrack < 1) return;
		if (_ntrack>maxsamples) _ntrack = maxsamples;

		_ptrackvolumes = new ControlSwitcher[_ntrack];
		_ptrack  = new Generator[_ntrack];
		_ppbuffer = new SampleTable*[_ntrack];
		_pplayer = new BufferPlayer[_ntrack];
		_pptrackvolume = new ControlGenerator*[_ntrack];

		/*
		ControlGenerator track1volume1 = addParameter("track1volume1", 1.0f).min(0.0f).max(1.0f);
		ControlGenerator track1volume2 = addParameter("track1volume2", 0.0f).min(0.0f).max(1.0f);
		ControlGenerator track1volume3 = addParameter("track1volume3", 1.0f).min(0.0f).max(1.0f);
		ControlGenerator track1volume4 = addParameter("track1volume4", 0.0f).min(0.0f).max(1.0f);
		ControlGenerator track1volume5 = addParameter("track1volume5", 1.0f).min(0.0f).max(1.0f);
		ControlGenerator track1volume6 = addParameter("track1volume6", 0.0f).min(0.0f).max(1.0f);
		ControlGenerator track1volume7 = addParameter("track1volume7", 1.0f).min(0.0f).max(1.0f);
		ControlGenerator track1volume8 = addParameter("track1volume8", 0.0f).min(0.0f).max(1.0f);
		*/

		for (int i = 0; i < _ntrack; i++)
		{
			_ptrackvolumes[i] = ControlSwitcher().inputIndex(step); //& ??
			

			//SampleTable buffer = loadAudioFile("D:\\oifii-org\\httpdocs\\ha-org\\had\\dj-oifii\\BASSDRUMS (House Minimal Trance D&B Techno) PACK 1\\BD (1).wav",2);
			SndfileHandle file1;
			//file1 = SndfileHandle("D:\\oifii-org\\httpdocs\\ha-org\\had\\dj-oifii\\BASSDRUMS (House Minimal Trance D&B Techno) PACK 1\\BD (9).wav");
			if (_samplefilenames.size()<=maxsamples)
			{
				file1 = SndfileHandle(_samplefilenames[i]);
			}
			else
			{
				file1 = SndfileHandle(_samplefilenames[RandomInt(0, _samplefilenames.size()-1)]);
			}
			assert(file1.samplerate() == 44100);
			assert(file1.channels() == 2);
			_ppbuffer[i] = new SampleTable(file1.frames(), file1.channels());
			file1.read(_ppbuffer[i]->dataPointer(), file1.frames()*file1.channels());
			_pplayer[i].setBuffer(*(_ppbuffer[i])).loop(false);

			_pptrackvolume[i] = new ControlGenerator[_nstep];
			for (int j = 0; j < _nstep; j++)
			{
				sprintf(charbuf, "t%i_v%i", i,j);
				_pptrackvolume[i][j] = addParameter(charbuf, 0.0f).min(0.0f).max(1.0f);
				_ptrackvolumes[i].addInput(_pptrackvolume[i][j]);
			}
			/*
			ControlGenerator track1volume1 = addParameter("track1volume1", 1.0f).min(0.0f).max(1.0f);
			ControlGenerator track1volume2 = addParameter("track1volume2", 0.0f).min(0.0f).max(1.0f);
			ControlGenerator track1volume3 = addParameter("track1volume3", 1.0f).min(0.0f).max(1.0f);
			ControlGenerator track1volume4 = addParameter("track1volume4", 0.0f).min(0.0f).max(1.0f);
			ControlGenerator track1volume5 = addParameter("track1volume5", 1.0f).min(0.0f).max(1.0f);
			ControlGenerator track1volume6 = addParameter("track1volume6", 0.0f).min(0.0f).max(1.0f);
			ControlGenerator track1volume7 = addParameter("track1volume7", 1.0f).min(0.0f).max(1.0f);
			ControlGenerator track1volume8 = addParameter("track1volume8", 0.0f).min(0.0f).max(1.0f);
			
			_ptrackvolumes[i].addInput(track1volume1);
			_ptrackvolumes[i].addInput(track1volume2);
			_ptrackvolumes[i].addInput(track1volume3);
			_ptrackvolumes[i].addInput(track1volume4);
			_ptrackvolumes[i].addInput(track1volume5);
			_ptrackvolumes[i].addInput(track1volume6);
			_ptrackvolumes[i].addInput(track1volume7);
			_ptrackvolumes[i].addInput(track1volume8);
			*/

			/*
			// ControlSwitcher holds a list of ControlGenerators, and routes whichever one the inputIndex is pointing to to its output.
			ControlSwitcher track2volumes = ControlSwitcher().inputIndex(step);

			SndfileHandle file2;
			file2 = SndfileHandle("D:\\oifii-org\\httpdocs\\ha-org\\had\\dj-oifii\\Function.Loops.Spirit.Progressive.Trance.and.Psy.WAV\\DRUMS\\DRUM_01_HIHAT_138bpm(single-128ms).wav");
			assert(file2.samplerate() == 44100);
			assert(file2.channels() == 2);
			SampleTable buffer2(file2.frames(), file2.channels());
			file2.read(buffer2.dataPointer(), file2.frames()*file2.channels());
			BufferPlayer bPlayer2;
			//bPlayer2.setBuffer(buffer2).loop(false).trigger(ControlMetro().bpm(120 * 4));
			//bPlayer2.setBuffer(buffer2).loop(false).trigger(metro);
			bPlayer2.setBuffer(buffer2).loop(false);

			ControlGenerator track2volume1 = addParameter("track2volume1", 0.0f).min(0.0f).max(1.0f);
			ControlGenerator track2volume2 = addParameter("track2volume2", 1.0f).min(0.0f).max(1.0f);
			ControlGenerator track2volume3 = addParameter("track2volume3", 0.0f).min(0.0f).max(1.0f);
			ControlGenerator track2volume4 = addParameter("track2volume4", 1.0f).min(0.0f).max(1.0f);
			ControlGenerator track2volume5 = addParameter("track2volume5", 0.0f).min(0.0f).max(1.0f);
			ControlGenerator track2volume6 = addParameter("track2volume6", 1.0f).min(0.0f).max(1.0f);
			ControlGenerator track2volume7 = addParameter("track2volume7", 0.0f).min(0.0f).max(1.0f);
			ControlGenerator track2volume8 = addParameter("track2volume8", 1.0f).min(0.0f).max(1.0f);
			track2volumes.addInput(track2volume1);
			track2volumes.addInput(track2volume2);
			track2volumes.addInput(track2volume3);
			track2volumes.addInput(track2volume4);
			track2volumes.addInput(track2volume5);
			track2volumes.addInput(track2volume6);
			track2volumes.addInput(track2volume7);
			track2volumes.addInput(track2volume8);
			*/

			//_ptrack[i] = _pplayer[i].trigger(metro) * _ptrackvolumes[i]; //spi original
			//_ptrack[i] = _pplayer[i].trigger(metro) * _ptrackvolumes[i].smoothed(0.025); //spi, useless at this point
			//_ptrack[i] = _pplayer[i].trigger(metro) * ADSR(0.001, 0.0, 1.0, 0.100).doesSustain(false).trigger(metro) * _ptrackvolumes[i]; //spi tweak
			//_ptrack[i] = _pplayer[i].trigger(metro) * ControlPulse(0.100).trigger(metro).smoothed(0.005) * _ptrackvolumes[i]; //spi tweak
			_ptrack[i] = _pplayer[i].trigger(metro) * ControlPulse(0.9*60.0/(((float)_nstep/2.0)*initialbpm)).trigger(metro).smoothed(0.005) * _ptrackvolumes[i]; //spi tweak
			/*
			Generator track2 = bPlayer2.trigger(metro) * track2volumes;
			*/
		}
		
		/*
		StereoDelay delay = StereoDelay(3.0f, 3.0f, 3.0f, 3.0f)
			//.delayTimeLeft(0.5 + SineWave().freq(0.2) * 0.01)
			.delayTimeLeft(addParameter("delay_left", 0.5).min(0.0).max(3.0) + SineWave().freq(0.2) * 0.01)
			//.delayTimeRight(0.55 + SineWave().freq(0.23) * 0.01)
			.delayTimeRight(addParameter("delay_right", 0.55).min(0.0).max(3.0) + SineWave().freq(0.23) * 0.01)
			//.feedback(0.3)
			.feedback(addParameter("delay_feedback", 0.7).min(0.0).max(1.0))
			.dryLevel(addParameter("delay_drylevel", 0.8).min(0.0).max(1.0))
			//.wetLevel(0.2);
			.wetLevel(addParameter("delay_wetlevel", 0.8).min(0.0).max(1.0));
		*/

		//setOutputGen(delay.input(track1 + track2));
		for (int i = 0; i < _ntrack; i++)
		{
			setOutputGen(getOutputGen()+_ptrack[i]);
		}
		setOutputGen(getOutputGen()*addParameter("volume", 1.0).min(0.0).max(1.0));
		_ready=true;
#else

#pragma message("C++11 Support not available - StepSequencerBufferPlayerDynamicSynth disabled")

#endif
	}
	~StepSequencerBufferPlayerDynamicSynth()
	{
		if (_ntrack < 1) return;

		delete[] _ptrackvolumes;
		delete[] _ptrack;
		for (int i = 0; i < _ntrack; i++)
		{
			delete _ppbuffer[i];
			delete[] _pptrackvolume[i];
		}
		delete[] _ppbuffer;
		delete[] _pplayer;
		delete[] _pptrackvolume;
	}

};

//TONIC_REGISTER_SYNTH(StepSequencerBufferPlayerDynamicSynth)

#endif
