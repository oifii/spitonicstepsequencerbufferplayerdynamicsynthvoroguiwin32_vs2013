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

#ifndef SimpleStepSequencerBufferPlayerSynth_h
#define SimpleStepSequencerBufferPlayerSynth_h


#include "Tonic.h"

#include <sndfile.hh>

using namespace Tonic;

class SimpleStepSequencerBufferPlayerSynth : public Synth {


public:
	SimpleStepSequencerBufferPlayerSynth(){

		//#if TONIC_HAS_CPP_11
#if 1 
		//1) load a sample file
		//SampleTable buffer = loadAudioFile("",2);
		SndfileHandle file1;
		file1 = SndfileHandle("D:\\Program Files\\Native Instruments\\Sample Libraries\\Kontakt 3 Library\\World\\Z - Samples\\Djembe Kit Samples\\DJK_OPN_LRl_05.wav");
		assert(file1.samplerate() == 44100);
		assert(file1.channels() == 2);
		float file1duration_s = ((float)file1.frames()) / ((float)file1.samplerate());
		SampleTable buffer1(file1.frames(), file1.channels());
		file1.read(buffer1.dataPointer(), file1.frames()*file1.channels());
		BufferPlayer bPlayer1;
		bPlayer1.setBuffer(buffer1).loop(false);

		const int NUM_STEPS = 8; //original
		//const int NUM_STEPS = 64; //spi

		ControlGenerator metro = ControlMetro().bpm(4 * addParameter("tempo", 150).min(50).max(300));

		ControlGenerator step = ControlStepper().end(NUM_STEPS).trigger(metro);
		ControlGenerator transpose = addParameter("transpose", 0).min(-6).max(6);
		ControlSwitcher pitches = ControlSwitcher().inputIndex(step);
		ControlSwitcher cutoffs = ControlSwitcher().inputIndex(step);

		for (int i = 0; i < NUM_STEPS; i++){
			float initialValue = randomFloat(10, 80);
			//float initialValue = randomFloat(10, 127);
			pitches.addInput(addParameter("step" + to_string(i) + "Pitch", initialValue).min(10).max(80));
			cutoffs.addInput(addParameter("step" + to_string(i) + "Cutoff", 500).min(30).max(1500));
		}

		//ControlGenerator midiNote = transpose + (pitches >> ControlSnapToScale().setScale({0,2,3,5,7,10})); //original
		ControlGenerator midiNote = transpose + pitches;
		//3) define a note frequency control generator
		// convert a midi note to a frequency (plugging that parameter into another object)
		ControlGenerator noteFreq = ControlMidiToFreq().input(midiNote);

		//5) define a tone generator
		Generator tone = bPlayer1.trigger(metro); 

		//6) let's put a bandpass filter on the tone
		//tone = BPF24().input(tone).Q(1).cutoff(noteFreq);
		tone = BPF24().input(tone*0.5).Q(0.01).cutoff(noteFreq);
		//tone = LPF12().input(tone*0.5).Q(0.1).cutoff(cutoffs);

		Generator outputGen = tone * ADSR(0.01, 0.1, 0, 0).trigger(metro);

		Compressor compressor = Compressor()
			.attack(0.001)
			.release(0.06)
			.threshold(ControlDbToLinear().input(-6.))
			.ratio(5.0)
			.lookahead(0.001)
			.makeupGain(ControlDbToLinear().input(0.))
			.bypass(false);

		Compressor compressor2 = Compressor()
			.attack(0.001)
			.release(0.06)
			.threshold(ControlDbToLinear().input(-6.))
			.ratio(5.0)
			.lookahead(0.001)
			.makeupGain(ControlDbToLinear().input(0.))
			.bypass(false);

		Compressor compressor3 = Compressor()
			.attack(0.001)
			.release(1)
			.threshold(ControlDbToLinear().input(-6.))
			.ratio(5.0)
			.lookahead(0.001)
			.makeupGain(ControlDbToLinear().input(0.))
			.bypass(false);

		//outputGen = outputGen >> compressor;
		
		//outputGen = outputGen >> Reverb().wetLevel(0.3).dryLevel(0.3).decayTime(100);
		
		//outputGen = outputGen >> compressor2;

		//outputGen = outputGen >> Reverb().wetLevel(0.3).dryLevel(0.3).decayTime(100);
		/*
		outputGen = outputGen >> Reverb().wetLevel(0.3).dryLevel(0.3).decayTime(100);
		outputGen = outputGen >> StereoDelay(0.75, 1.25, 1.5, 1.5);
		*/
		//outputGen = outputGen >> compressor3;

		setOutputGen(outputGen);

#else

#pragma message("C++11 Support not available - SimpleStepSequencerBufferPlayerSynth disabled")

#endif

	}

};

//TONIC_REGISTER_SYNTH(SimpleStepSequencerBufferPlayerSynth);


#endif
