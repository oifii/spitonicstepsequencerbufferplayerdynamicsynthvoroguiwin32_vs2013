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

#ifndef EventsExpBufferPlayerSynth_h
#define EventsExpBufferPlayerSynth_h

#include "Tonic.h"
#include <functional>

#include <sndfile.hh>

using namespace Tonic;


#define INT_TO_STRING( x ) dynamic_cast< std::ostringstream & >( \
	(std::ostringstream() << std::dec << x)).str()

class EventsExpBufferPlayerSynth : public Synth{

public:
	const int NUM_VOICES = 5;

	EventsExpBufferPlayerSynth(){

		//#if TONIC_HAS_CPP_11
#if 1
		Generator allVoices;

		Generator noise = PinkNoise(); //original
		//Generator noise = 0.5* SineWave().freq(110) + 0.5* SineWave().freq(220); //spi
		//Generator noise = SineWave().freq(220); //spi
		//Generator noise = SawtoothWave().freq(220); //spi
		//Generator noise = LFNoise().setFreq(220); //spi

		//1) load a sample file
		//SampleTable buffer = loadAudioFile("",2);
		SndfileHandle file1;
		//file1 = SndfileHandle("D:\\Program Files\\Native Instruments\\Sample Libraries\\Kontakt 3 Library\\Orchestral\\Z - Samples\\03 Cello ensemble - 8\\VC-8_trem_RS_ff\\VC-8_trem_RS_ff_B3.wav");
		//file1 = SndfileHandle("D:\\Program Files\\Native Instruments\\Sample Libraries\\Kontakt 3 Library\\Band\\Z - Samples\\1 - Horns Samples\\Alto Saxophone Samples\\ASaxA_sus_a#2_3.wav");
		file1 = SndfileHandle("D:\\Program Files\\Native Instruments\\Sample Libraries\\Kontakt 3 Library\\World\\Z - Samples\\Highland Pipes Samples\\HPdrones\\HPdrones.wav");
		assert(file1.samplerate() == 44100);
		assert(file1.channels() == 2);
		float file1duration_s = ((float)file1.frames()) / ((float)file1.samplerate());
		SampleTable buffer1(file1.frames(), file1.channels());
		file1.read(buffer1.dataPointer(), file1.frames()*file1.channels());
		BufferPlayer bPlayer1;
		bPlayer1.setBuffer(buffer1).loop(false);


		vector<float> scale;
		scale.push_back(0);
		scale.push_back(2);
		scale.push_back(4);
		scale.push_back(7);
		scale.push_back(11);

		for (int i = 0; i < NUM_VOICES; i++){


			////////////////////////////////////////////
			// Create the synthesis network
			////////////////////////////////////////////

			Generator voice;

			// Send a trigger message immediately, don't wait for the first beat to happen with the metrenome 
			ControlTrigger initialTrigger;
			initialTrigger.trigger();
			//ControlGenerator resetTrigger = initialTrigger + ControlMetro().bpm(ofRandom(10, 15)); //original - does not compile
			ControlGenerator resetTrigger = initialTrigger + ControlMetro().bpm(ControlRandom().min(10).max(15));

			ControlGenerator noiseTrigger = ControlMetro().bpm(ControlRandom().min(50).max(200).trigger(resetTrigger));

			ControlGenerator pulseLen = ControlRandom().min(0.1).max(0.5).trigger(resetTrigger);

			//ControlGenerator pulse = ControlPulse().length(pulseLen).input(noiseTrigger); //original - does not compile
			ControlGenerator pulse = ControlPulse().length(pulseLen).trigger(noiseTrigger);

			Generator env = ADSR(0.01, 0, 0.5, 0.01).decay(pulseLen * 0.5).trigger(pulse); //original
			//Generator env = ADSR(0.01, 0, 0.05, 0.01).decay(pulseLen * 0.5).trigger(pulse);

			Generator tone = bPlayer1.trigger(pulse);

			//voice = noise * env; //original
			voice = tone * env; //spi
			//voice = noise ;

			ControlGenerator highPassAmount = ControlRandom().min(0).max(1).trigger(resetTrigger);

			ControlSnapToScale scaleSnapper = ControlSnapToScale().setScale(scale);
			scaleSnapper.input(40 + highPassAmount * 50); //original
			//scaleSnapper.input(10 + highPassAmount * 50); //spi
			ControlGenerator filterFreq = ControlMidiToFreq().input(scaleSnapper); //original
			//ControlGenerator filterFreq = ControlMidiToFreq().input(40 + highPassAmount * 50); //spi, bypass snap to scale
			ControlGenerator q = ControlRandom().min(0.5).max(1).trigger(resetTrigger);
			q = q * q * 50;

			//voice = HPF24().cutoff(filterFreq).input(voice).Q(q); //original
			//voice = LPF24().cutoff(filterFreq).input(voice).Q(q); //spi
			voice = BPF24().cutoff(filterFreq).input(voice).Q(0.1); //spi

			float pan = (2 * (float)i / NUM_VOICES) - 1;
			voice = MonoToStereoPanner().pan(pan).input(voice) * (15.0 / NUM_VOICES);

			allVoices = allVoices + voice;


		}

		Generator allvoicesPlusReverb = Reverb().input(allVoices).density(1).dryLevel(0).wetLevel(0.2).decayTime(1); //original
		//Generator allvoicesPlusReverb = Reverb().input(allVoices).density(1).dryLevel(0).wetLevel(0.9).decayTime(1); //spi

		// set the synth's final output generator
		setOutputGen(allVoices + allvoicesPlusReverb);

#else

#pragma message("C++11 Support not available - EventsExpBufferPlayerSynth disabled")

#endif
	}
};

//TONIC_REGISTER_SYNTH(EventsExpBufferPlayerSynth)

#endif
