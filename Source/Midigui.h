#pragma once
/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2022 - Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             MidiDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Handles incoming and outcoming midi messages.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
				   juce_audio_processors, juce_audio_utils, juce_core,
				   juce_data_structures, juce_events, juce_graphics,
				   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        MidiDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once
using namespace juce;
#include "globals.h"
//==============================================================================
struct MidiDeviceListEntry : ReferenceCountedObject
{
	MidiDeviceListEntry(MidiDeviceInfo info) : deviceInfo(info) {}

	MidiDeviceInfo deviceInfo;
	std::unique_ptr<MidiInput> inDevice;
	std::unique_ptr<MidiOutput> outDevice;

	using Ptr = ReferenceCountedObjectPtr<MidiDeviceListEntry>;
};


//==============================================================================
class MidiHandler : public Component,
	private Timer,
	private MidiKeyboardState::Listener,
	private MidiInputCallback,
	private AsyncUpdater
{
public:
	//==============================================================================
	MidiHandler()
		: midiKeyboard(keyboardState, MidiKeyboardComponent::horizontalKeyboard)

	{
		addLabelAndSetStyle(midiInputLabel);
		addLabelAndSetStyle(midiOutputLabel);
		addLabelAndSetStyle(incomingMidiLabel);
		addLabelAndSetStyle(outgoingMidiLabel);
		midiInputs.clear();
		midiOutputs.clear();
		midiInputSelector = (std::unique_ptr<MidiDeviceListBox>)new MidiDeviceListBox("Midi Input Selector", *this, true);
		midiOutputSelector = (std::unique_ptr<MidiDeviceListBox>)new MidiDeviceListBox("Midi Output Selector", *this, false);

		midiKeyboard.setName("MIDI Keyboard");
		addAndMakeVisible(midiKeyboard);

		midiMonitor.setMultiLine(true);
		midiMonitor.setReturnKeyStartsNewLine(false);
		midiMonitor.setReadOnly(true);
		midiMonitor.setScrollbarsShown(true);
		midiMonitor.setCaretVisible(false);
		midiMonitor.setPopupMenuEnabled(false);
		midiMonitor.setText({});
		addAndMakeVisible(midiMonitor);

		if (!BluetoothMidiDevicePairingDialogue::isAvailable())
			pairButton.setEnabled(false);

		addAndMakeVisible(pairButton);
		pairButton.onClick = []
		{
			RuntimePermissions::request(RuntimePermissions::bluetoothMidi,
				[](bool wasGranted)
				{
					if (wasGranted)
						BluetoothMidiDevicePairingDialogue::open();
				});
		};
		keyboardState.addListener(this);
		//		midiInputSelector.reset();
		//		midiOutputSelector.reset();
		addAndMakeVisible(midiInputSelector.get());
		addAndMakeVisible(midiOutputSelector.get());
		setSize(732, 520);

		startTimer(500);
	}

	~MidiHandler() override
	{
		stopTimer();
		midiInputs.clear();
		midiOutputs.clear();
		keyboardState.removeListener(this);

		midiInputSelector.reset();
		midiOutputSelector.reset();
	}

	//==============================================================================
	void timerCallback() override
	{
		updateDeviceList(true);
		updateDeviceList(false);
	}

	void handleNoteOn(MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override
	{

		int note = midiNoteNumber;
		MidiMessage m(MidiMessage::noteOn(midiChannel, midiNoteNumber, velocity));
		m.setTimeStamp(Time::getMillisecondCounterHiRes() * 0.001);
		sendToOutputs(m);
	}

	void handleNoteOff(MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override
	{
		MidiMessage m(MidiMessage::noteOff(midiChannel, midiNoteNumber, velocity));
		m.setTimeStamp(Time::getMillisecondCounterHiRes() * 0.001);
		sendToOutputs(m);
	}
	void handleControl(MidiKeyboardState*, int midiChannel, int CC, int value)
	{
		MidiMessage m(MidiMessage::controllerEvent(midiChannel, CC, value));
		m.setTimeStamp(Time::getMillisecondCounterHiRes() * 0.001);
		sendToOutputs(m);
	}
	void handleSilence()
	{
		MidiMessage m(MidiMessage::allNotesOff(10));
		m.setTimeStamp(Time::getMillisecondCounterHiRes() * 0.001);
		sendToOutputs(m);
	}
	void paint(Graphics&) override {}

	void resized() override
	{
		auto margin = 10;

		midiInputLabel.setBounds(margin, margin,
			(getWidth() / 2) - (2 * margin), 24);

		midiOutputLabel.setBounds((getWidth() / 2) + margin, margin,
			(getWidth() / 2) - (2 * margin), 24);

#if 1
		midiInputSelector->setBounds(margin, (2 * margin) + 24,
			(getWidth() / 2) - (2 * margin),
			(getHeight() / 2) - ((4 * margin) + 24 + 24));

		midiOutputSelector->setBounds((getWidth() / 2) + margin, (2 * margin) + 24,
			(getWidth() / 2) - (2 * margin),
			(getHeight() / 2) - ((4 * margin) + 24 + 24));
#endif // 0


		pairButton.setBounds(margin, (getHeight() / 2) - (margin + 24),
			getWidth() - (2 * margin), 24);

		outgoingMidiLabel.setBounds(margin, getHeight() / 2, getWidth() - (2 * margin), 24);
		midiKeyboard.setBounds(margin, (getHeight() / 2) + (24 + margin), getWidth() - (2 * margin), 64);

		incomingMidiLabel.setBounds(margin, (getHeight() / 2) + (24 + (2 * margin) + 64),
			getWidth() - (2 * margin), 24);

		auto y = (getHeight() / 2) + ((2 * 24) + (3 * margin) + 64);
		midiMonitor.setBounds(margin, y,
			getWidth() - (2 * margin), getHeight() - y - margin);
	}
	int moindex = -1;
	int miindex = -1;
	void openDevice(bool isInput, int index)
	{
		if (isInput)
		{
			jassert(midiInputs[index]->inDevice.get() == nullptr);
			midiInputs[index]->inDevice = MidiInput::openDevice(midiInputs[index]->deviceInfo.identifier, this);

			if (midiInputs[index]->inDevice.get() == nullptr)
			{
				// DBG("MidiDemo::openDevice: open input device for index = " << index << " failed!");
				return;
			}
			miindex = index;
			midiInputs[index]->inDevice->start();
		}
		else
		{
			jassert(midiOutputs[index]->outDevice.get() == nullptr);
			midiOutputs[index]->outDevice = MidiOutput::openDevice(midiOutputs[index]->deviceInfo.identifier);
			moindex = index;
			if (midiOutputs[index]->outDevice.get() == nullptr)
			{
				// DBG("MidiDemo::openDevice: open output device for index = " << index << " failed!");
			}
		}
	}

	void closeDevice(bool isInput, int index)
	{
		if (isInput)
		{
			jassert(midiInputs[index]->inDevice.get() != nullptr);
			midiInputs[index]->inDevice->stop();
			midiInputs[index]->inDevice.reset();
		}
		else
		{
			jassert(midiOutputs[index]->outDevice.get() != nullptr);
			midiOutputs[index]->outDevice.reset();
		}
	}

	int getNumMidiInputs() const noexcept
	{
		return midiInputs.size();
	}

	int getNumMidiOutputs() const noexcept
	{
		return midiOutputs.size();
	}

	ReferenceCountedObjectPtr<MidiDeviceListEntry> getMidiDevice(int index, bool isInput) const noexcept
	{
		return isInput ? midiInputs[index] : midiOutputs[index];
	}
	bool playMidi(void)
	{
		MidiMessage* tempMessage;
		static bool first = true;
		float p = (runtime - firstEventstart) / lt;
		int ip = p * 100;
		static int lastip = -1;
		runtime += 0.001;
		const MidiMessageSequence* notes = SMF.getTrack(tr);

		for (; nextEvent < midilastEvent && (transport == REPEAT || transport == PLAYING) && nextEvent < notes->getNumEvents(); nextEvent++)
		{
			tempMessage = &(notes->getEventPointer(nextEvent)->message);
			double time = tempMessage->getTimeStamp();
			if (time > runtime)
			{

				break;
			}
			//			// DBG(nextEvent << " " << midilastEvent << " " << runtime << " " << lt<<" "<<ip);
			if (tempMessage->isController())
			{
				handleControl(0, 10, tempMessage->getControllerNumber(), tempMessage->getControllerValue());
			}
			if (tempMessage->isNoteOn() || tempMessage->isNoteOff())
			{
				int note = tempMessage->getNoteNumber();
				if (note == 0)
					continue;
				int vel = tempMessage->getVelocity();
				if (tempMessage->isNoteOff())
					vel = 0;
				bool playnote = !soloPlay || voicestat[np[note]] == 1 || voicestat[np[note]] == 5;
				bool mute = voicestat[np[note]] == 2;
				if (voicestat[np[note]] >= 4)
					note = voicenote[np[note]];
				float vv = voiceamp[np[note]] * (vel / 127.0);
				if (vv > 1)
					vv = 1;
				if (playnote && !mute)
					handleNoteOn(0, 10, note, vv);

			}
		}
		if (transport == STOPPED)
		{
			handleSilence();
			return false;
		}
		if (lastip != ip)
		{
			String progress = "<meter id=\"file\" value=\"" + String(p) + "\"> 32% </meter>";
			webgui.setMonitor(progidt, progress);
		}
		lastip = ip;

		if (nextEvent >= midilastEvent)
		{
			first = true;
			if (transport == REPEAT)
			{
				runtime = startruntime;
				nextEvent = firstEvent;
			}
			else {

				return false;
			}
		}
		return true;
	}


	double lt;
	int midilastEvent;
	double runtime;
	double startruntime;
	float beatfactor;
	float barlength;
	int tr;
	int setMidi(File midiFile)
	{
		if (!midiFile.exists())
			return 0;
		FileInputStream source(midiFile);
		SMF.readFrom(source);
		float t = SMF.getLastTimestamp();
		float tf = SMF.getTimeFormat();
		float beats = t / tf;
		int bars = beats / 4;
		float beattime = 0;
		MidiMessageSequence tempoEvents;
		SMF.findAllTempoEvents(tempoEvents);
		SMF.findAllTimeSigEvents(tempoEvents);
		for (auto m : tempoEvents)
		{
			if (m->message.getTempoSecondsPerQuarterNote() > 0)
			{
				beattime = m->message.getTempoSecondsPerQuarterNote();
			}
		}
		MidiMessage* tempMessage;
		tr = SMF.getNumTracks() - 1;
		for (int i = 0; i < SMF.getNumTracks(); i++)
		{

			notes = SMF.getTrack(i);
			int e = 0;
			tempMessage = &(notes->getEventPointer(0)->message);
			while (!tempMessage->isNoteOn() && e < notes->getNumEvents())
				tempMessage = &(notes->getEventPointer(e++)->message);
			if (tempMessage->getChannel() == 10)
			{
				//				// DBG(i << " " << tempMessage->getChannel());
				tr = i;
				break;
			}
		}
		//		notes = SMF.getTrack(tr);
		SMF2.clear();
		SMF2.addTrack(*notes);
		SMF2.setTicksPerQuarterNote(SMF.getTimeFormat());
		SMF.convertTimestampTicksToSeconds();
		if (beattime == 0)
		{
			float t2 = SMF.getLastTimestamp();
			beattime = t2 / beats;
		}
		beatfactor = 2 * beattime;
		barlength = 2 * beatfactor;
		//		lt =notes->getEndTime();
		transport = REPEAT;
		if (beats < 4)
			beats = 4;
		firstEventstart = notes->getStartTime();
		return notes->getNumEvents();
	}
	noteshow* evs;
	double firstEventstart = -1;
	const MidiMessageSequence* notes;

	int previewMIDI(juce::File pfile, bool renew)
	{
		const MidiMessageSequence* notes2;
		notes2 = SMF2.getTrack(0);
		String header = " <svg height=\"600\" width=\"1400\">";
		String nl = header;
		MidiMessage* tempMessage;
		MidiMessage* tempMessage2;
		float t = SMF.getLastTimestamp() / barlength;
		int ne = notes->getNumEvents();
		if (renew)
		{
			minc = 1;
			startMidi = -1;
			lastMidi = -1;
			for (int i = 0; i < 32; i++)
				webgui.remove(midiVoiceID[i]);
			for (int i = 0; i < 128; i++)
				np[i] = -1;
			for (int i = 0; i < 1024; i++)
				bc2event[i] = -1;
			int nx = 0;
			int minNote = 128;
			//	nl += showLine(0, 50, 1220, 50);


			//	nl += svgend;
			events.clear();
			firstEventstart = -1;
			int tx = SMF.getTimeFormat();
			int lastcx = -1;
			noteshow* ev;
			for (int i = 0; i < ne; i++)
			{
				tempMessage = &(notes->getEventPointer(i)->message);
				tempMessage2 = &(notes2->getEventPointer(i)->message);
				if (tempMessage->isNoteOn() || tempMessage->isNoteOff())
				{
					int note = tempMessage->getNoteNumber();
					if (note == 0)
						continue;
					if (np[note] == -1)
					{
						np[note] = nx++;
					}
					int vel = tempMessage->getVelocity();
					int barnr = (tempMessage2->getTimeStamp() / tx) / 4;
					//					// DBG(i << " " << note << " " << vel << " " << tempMessage->getTimeStamp() << " " << tempMessage2->getTimeStamp() / tx<<" "<<barnr);
					if (tempMessage->isNoteOff() || vel == 0)
					{
						int idx = onEvents[tempMessage->getNoteNumber()];
						ev = events[idx];
						if (ev != nullptr)
							ev->length = tempMessage->getTimeStamp() - ev->start - firstEventstart;
					}
					else
					{
						ev = new noteshow;
						ev->note = tempMessage->getNoteNumber();
						if (firstEventstart < 0)
							firstEventstart = tempMessage->getTimeStamp();
						ev->start = tempMessage->getTimeStamp() - firstEventstart;
						ev->eventnr = i;
						ev->bar = barnr;
						if (barnr > lastcx)
						{
							bcc = barnr;
							lastcx = barnr;
							if (bc2event[barnr] == -1)
							{
								bc2event[barnr] = events.size();
								bc2eventnr[barnr] = i;
								//								// DBG(i<<" "<<ne<<" "<<bcc << " " << bc2event[bcc] << " " << bc2eventnr[bcc] << " " << ev->start / barlength);
							}
						}
						onEvents[ev->note] = events.size();
						//						DBG(ev->start << " " << ev->note << " "  << barnr);
						events.add(ev);
						if (note < minNote)
							minNote = note;
					}
				}
			}
			int evl = events.size();
			auto ex = events.getLast();
			if (bc2event[bcc + 1] == -1)
			{
				bc2event[bcc + 1] = events.size();
				bc2eventnr[bcc + 1] = ne - 1;
				ev->eventnr = ne - 1;
				events.add(ev);
				bcc++;
			}

			int headid = webgui.addStringDisplay("vel", 135, 800, "f");
			webgui.setMonitor(headid, "Amp&nbsp;Rp In");
			zoomStart = 0;
			zoomEnd = bcc;
		}

		midilastEvent = notes->getNumEvents();
		startruntime = 0;
		noteshow* ev = events[events.size() - 1];
		//		double lf = 1220 / (ev->start + ev->length);
				//	nl = header;
		showVoiceline();
		for (int i = 0; i < 32 && renew; i++)
		{
			webgui.remove(midiVoiceID[i]);
			webgui.remove(midiNoteID[i]);
			webgui.remove(midiVelID[i]);
		}
		for (int i = 0; i < 128 && renew; i++)
		{
			int nx = np[i];
			if (nx == -1)
				continue;
			int y = 80 + nx * 20;
			if (y > ly)
				ly = y;
			voicenote[nx] = i;
			voiceamp[nx] = 1.0;
			midiVoiceID[nx] = webgui.addButtons(String(i), &onButtonRelease, 190, y + 740, "f", "hide");
			midiNoteID[nx] = webgui.addStringDisplay("vel", 170, y + 740, "f");
			midiVelID[nx] = webgui.addStringDisplay("vel", 140, y + 740, "f");

			webgui.setMonitor(midiNoteID[nx], String(voicenote[nx]));
			webgui.setMonitor(midiVelID[nx], String(voiceamp[nx]));
			midiNoteID[nx];

		}
		for (int i = 0; i < 999; i++)
			webgui.remove(midiActbeatID[i]);
		int s = 0;
		int e = bcc;
		int es = 0;
		int ee = events.size() - 1;
		evs = events[es];
		noteshow* ev0 = events[0];
		lt = notes->getEndTime() - firstEventstart;
		delta = 1220 / (lt / barlength);

		if (startMidi > -1 && lastMidi > startMidi)
		{

			s = startMidi;
			e = lastMidi;
			if (lastMidi + minc > bcc)
				e++;
			while (bc2event[e] < 0)
				e--;
			es = bc2event[s];
			ee = bc2event[e];
			evs = events[es];
			noteshow* eve = events[ee];
			firstEvent = events[es]->eventnr;
			midilastEvent = events[ee]->eventnr;
			startruntime = evs->start;
			zoomStart = s;
			zoomEnd = e;
			lt = (e - s) * barlength;
			delta = 1220 / (e - s);
			startMidi = -1;
			lastMidi = -1;
			minc = 1;
			firstEventstart = startruntime;

		}
		if (delta <= 10)
			minc = 8;
		else if (delta <= 10)
			minc = 4;
		else if (delta < 20)
			minc = 2;
		//		// DBG("lne "<<events.getLast()->eventnr<<" ne " << notes->getNumEvents() << " ns " << events.size() << " SM " << startMidi << " LM " << lastMidi << "  es " << es << " ee " << ee << " fe " << firstEvent << " mlE " << midilastEvent << " " << evs->start);
		showBeats();
		for (int i = s; i < e + 1; i += minc)
		{
			midiActbeatID[i] = webgui.addButtons(String(i + 1), &onButtonRelease, 227 + (i - s) * delta, 760, "f", "hide");
		}
		noteshow* eve = events[ee];
		double lf = 1220 / lt;
		//		// DBG("delta " << delta << " time " << t<< " lf "<<lf);
		for (int i = es; i <= ee; i++)
		{
			noteshow* ev = events[i];
			if (ev->bar >= zoomEnd)
				break;
			// DBG(ev->eventnr << " " << ev->bar << " " << ev->note);
			int y = 80 + (np[ev->note]) * 20;
			int x1 = 5 + (int)((ev->start - evs->start) * lf);
			int x2 = 5 + (int)((ev->start - evs->start) * lf + lf / 16);
			nl += showLine(x1, y, x2, y, 8);

		}
		nl += svgend;
		webgui.setMonitor(beatnrid, nl);

		lastMidiFile = pfile;
		return ne;
	}
	void playPattern(void)
	{
		if (actpattern < 0)
		{
			metTimer->stopTimer();
			return;
		}
		patternc++;
		int bl = (actpattern + beatCount[actpattern]) * maxticks;

		if (editMode)
			bl = (actpattern + 1) * maxticks;
		bool beatend = patternc >= bl;
		//		// DBG(patternc << " " << bl << " " << (actpattern + 1) * maxticks);

		if (beatend)  //|| newpatternc > -1
		{
			static int barinc = 1;
			//       FDBG("bl " + SN(bl) + " " + SN(patternc) + " " + SN(beatCount[actpattern]) + SB(beatend));
			if (!editMode)
			{
				if (isShuffle)
				{
					shufflebarsidx++;
					// DBG(shufflebarsidx);
					if (shufflebarsidx >= shufflelength[shufflelengthidx])
					{
						shufflebarsidx = 0;
						shufflelengthidx+=barinc;
						if (shuffleinc[shufflelengthidx] == 0)
							barinc = 1;
						else
						{
							Range< int >rr(1, (const int)shuffleinc[shufflelengthidx]);
							barinc = sRand.nextInt(rr);

						}
						// DBG(shufflebarsidx << " " << shufflelengthidx);
						if (shufflelengthidx >= shufflemax)
						{
							shufflelengthidx = 0;
							// DBG(shufflebarsidx << " " << shufflelengthidx);
						}
						if (shufflegrp[shufflelengthidx] > -1)
						{
							shufflelength[shufflelengthidx] = shufflelength[shufflegrp[shufflelengthidx]];

						}
						else if (shufflerand[shufflelengthidx] > 0)
						{
							Range< int >rr(1, (const int)shufflerand[shufflelengthidx]);
							shufflelength[shufflelengthidx] = sRand.nextInt(rr);
						}

					}
					actpattern = shufflebars[shufflelengthidx];
				}
				// DBG(shufflebarsidx << " " << shufflelengthidx << " " << actpattern << " " << shufflelength[shufflelengthidx]);
				patternc = actpattern * maxticks;

					if (newpatternc > -1)
					{
						patternc = newpatternc;
						actpattern = patternc / maxticks;
						newpatternc = -1;
					}
				bl = patternc + beatCount[actpattern] * maxticks;
			}
			else
			{
				patternc = (actpattern)*maxticks;
			}
		}

		patcnt = patternc / maxticks;
		Time::waitForMillisecondCounter(630.0 * delaypattern[patternc] / 120);
		if (ccpattern[patternc] > 0)
		{
			byte cc = ccpattern[patternc];
			byte val = ccval[patternc];
			handleControl(0, 10, cc, val);

			if (transport == RECORDING)
			{
				if (lastEvent == 0)
					MidiEvent::starttime = 0;
				sequences[lastEvent++].init(0xB0, cc, val, 9);
			}
		}
		int aa = patcnt;

		for (int v = patvoicelow[aa]; v < patvoicehigh[aa] + 1 && v < MAXVOI; v++)
		{
			short ptest = seqpattern[patternc][v];
			//       FDBG("voice " + SN(v) + SN(ptest));
			if (ptest == -1)
				continue;
			//       FDBG(patternc);
			byte midinr = minstr[v];
			//			// DBG("v " << v << " pc " << patternc << " vel " << ptest << " nr " << minstr[v]);
			if (midinr == 0)
			{
				continue;
			}
			byte midivel = ptest;
			{
				handleNoteOn(0, 10, midinr, midivel / 127.0);
				if (transport == RECORDING)
				{
					if (lastEvent == 0)
						MidiEvent::starttime = 0;
					sequences[lastEvent++].init(0x90, midinr, midivel, 9);
				}
			}
		}
	}


	void loadMidi2Seq(MidiFile _SMF, int np)
	{
		MidiMessage* tempMessage;
		const MidiMessageSequence* lnotes;
		lnotes = _SMF.getTrack(_SMF.getNumTracks() - 1);
		int tempo = _SMF.getTimeFormat();

		cleanpat(np);
		int note;
		juce::String pcol = coloring[lastColor % 4];
		//    FDBG(patfiles[actpattern] + " LC " + SN(actpattern) + SN(lastColor) + SN(pcol));
		lastColor++;
		double  time;
		double t4 = beatfactor;
		tempo = tempo / 12;
		float tf = tempo / 8;
		for (int a = actpattern, b = np; a < actpattern + np; a++, b--)
		{
			beatCount[a] = b;
			patcolors[a] = pcol;
			//       FDBG(SN(a) + " PC " + SN(beatCount[a]) + patcolors[a]);
		}
		int j = 0;
		tempMessage = &(lnotes->getEventPointer(j)->message);
		while (!tempMessage->isNoteOn())
			tempMessage = &(lnotes->getEventPointer(++j)->message);
		double st = tempMessage->getTimeStamp();
		for (; j < lnotes->getNumEvents(); j++)
		{
			tempMessage = &(lnotes->getEventPointer(j)->message);
			if (tempMessage->isNoteOn() || tempMessage->isNoteOff())
			{
				note = tempMessage->getNoteNumber();
				int vel = tempMessage->getVelocity();
				if (tempMessage->isNoteOff())
					vel = 0;
				time = tempMessage->getTimeStamp() - st;
				int pos = t4 * time / tempo;
				short as = actpattern * maxticks + pos;
				int v = getVoice(note);
				if (v >= MAXVOI)
					continue;
				if (vel == 0)
					as++;
				int lim = MAXPAT * maxticks;
				if (as >= lim)
					break;
				int pp = as / maxticks;
				if (patvoicelow[pp] > v)
					patvoicelow[pp] = v;
				if (patvoicehigh[pp] < v)
					patvoicehigh[pp] = v;
				if (mvelo[v] < vel)
					mvelo[v] = vel;
				seqpattern[as][v] = vel;
				int del = (time - pos * tempo) / tf;
				if (del > delaypattern[as])
					delaypattern[as] = del;
				//			// DBG("v " << v << " as " << as << " vel " << vel);

			}
		}

		update_pat(true);
		lnotes = nullptr;
	}
	MidiFile clip2Midi(void)
	{
		MidiMessage* tempMessage;
		const MidiMessageSequence* notes;
		int tr = SMF2.getNumTracks() - 1;
		// DBG("zs " << zoomStart << " ze " << zoomEnd);
		notes = SMF2.getTrack(tr);
		MidiMessageSequence sequence;
		MidiMessageSequence tempoEvents;
		SMF.findAllTempoEvents(tempoEvents);
		SMF.findAllTimeSigEvents(tempoEvents);
		for (auto m : tempoEvents)
		{
			if (m->message.getTempoSecondsPerQuarterNote() > 0)
				sequence.addEvent(m->message);
		}
		int fe = firstEvent;
		tempMessage = &(notes->getEventPointer(fe)->message);
		while (!tempMessage->isNoteOn())
			tempMessage = &(notes->getEventPointer(++fe)->message);
		tempMessage = &(notes->getEventPointer(fe)->message);
		double ftime = tempMessage->getTimeStamp();


		for (int s = fe; s < midilastEvent; s++)
		{
			tempMessage = &(notes->getEventPointer(s)->message);
			double time = (tempMessage->getTimeStamp());
			tempMessage->setTimeStamp(time - ftime);
			sequence.addEvent(*tempMessage);
		}
		MidiFile file;
		file.setTicksPerQuarterNote(SMF2.getTimeFormat());
		file.addTrack(sequence);
		return file;
	}
	void loadClip(int np)
	{
		String fn = lastMidiFile.getFileNameWithoutExtension() + "_" + String(zoomStart + 1) + "-" + String(zoomEnd + 1) + ".mid";
		MidiFile file = clip2Midi();
		loadMidi2Seq(file, np);
		patfiles[actpattern] = fn + " " + SN(np);

	}
	void loadMIDI(juce::File file, int np, int v)
	{
		FileInputStream source(file);
		lastMidiFile = file;
		SMF.readFrom(source);
		loadMidi2Seq(SMF, np);
		patfiles[actpattern] = file.getFullPathName() + " " + SN(np) + "|" + SN(v);
		actpattern += np;
	}
	int loadDirectory(File dir, String search, int stn = 0)
	{
		String ldir = dir.getFileName();
		res[0] = dir;
		int xat = ldir.indexOf("@");
		int xat1 = ldir.indexOf("@_");
		if (xat == -1)
			dirres[0] = "&#x1F4C1;" + ldir;
		else
		{
			//          FDBG(fn + " " + SN(xat1));
			if (xat1 > -1)
			{
				dirres[0] = "_" + ldir.substring(xat1 + 2).replace("__", "&emsp;").replace("_", " ");
			}
			else
				dirres[0] = "&#x1F4C1;" + ldir.substring(xat + 1).replace("__", "&emsp;").replace("_", " ");
		}

		res[1] = dir.getParentDirectory();
		dirres[1] = "..";
		int n = 2;
		String ename;
		int nx = dir.getNumberOfChildFiles(3);
		if (search.length() == 0 || nx == 0)
		{
			nx = dir.getNumberOfChildFiles(3);
			search = "";
		}
		if (nx)
		{
			Array<File> entry = dir.findChildFiles(3, false);
			if (search.length() == 0)
				entry = dir.findChildFiles(3, false);
			FileComparator sorter;
			entry.sort(sorter);
			for (File f : entry)
			{
				ename = f.getFileName().toLowerCase();
				if (!f.isDirectory())
				{
					if (f.getFileExtension() != ".mid" && f.getFileExtension() != ".drm")
						continue;
					String size = String(f.getSize());
					if (f.getFileExtension() == ".mid")
					{
						int v = 0;
						int bc = sizeMIDI(f, v);
						if (bc == 0)
							continue;
						size = String(bc) + "|" + String(v);
					}

					if (n - 2 < stn)
					{
						//          FDBG("skip " + SN(n) + ename);
						n++;
						continue;
					}
					if (n - 2 - stn < 20)
					{
						res[n - stn] = f;
						dirres[n - stn] = ename + " " + size;
						n++;
					}
					else
					{
						dirres[n - stn] = stn - 20 > 0 ? String(stn - 20) + "&larr;" : " ";
						n++;
						dirres[n - stn] = String(n - 3) + "&rarr;";
						n++;
						return n - stn;
					}
				}
				else
				{
					//      dirout += "  " + fn + "~";
					bool match = true;
					//     FDBG(ename + " " + search + " " + SB(match));
					if (match)
					{
						res[n] = f;

						int xat = ename.indexOf("@");
						int xat1 = ename.indexOf("@_");
						if (xat == -1)
							dirres[n] = "&#x1F4C1;" + ename;
						else
						{
							//          FDBG(fn + " " + SN(xat1));
							if (xat1 > -1)
							{
								dirres[n] = "_" + ename.substring(xat1 + 2).replace("__", "&emsp;").replace("_", " ");
							}
							else
								dirres[n] = "&#x1F4C1;" + ename.substring(xat + 1).replace("__", "&emsp;").replace("_", " ");
						}
						n++;
					}

				}
			}
		}
		return (n - stn);
	}
	void changeDir(juce::String argv, bool load = false)
	{
		FDBG("change dir " + argv);
		if (argv == "" && load)
		{

			currentDirectory = File(File::getSpecialLocation(File::SpecialLocationType::globalApplicationsDirectoryX86).getFullPathName() +
				File::getSeparatorString() + "Toontrack" + File::getSeparatorString() + "EZDrummer" + File::getSeparatorString() + "Midi");

			int n = loadDirectory(currentDirectory, "", 0);
			diropts(n);
			return;
		}

		return;
	}
	void onFileSelect(juce::File file, bool found)
	{

		if (file.getFileExtension() == ".mid")
		{
			midilastEvent = setMidi(file);
			transport = REPEAT;
			nextEvent = 0;
			firstEvent = 0;

			runtime = 0;
			midiTimer->startTimer(1);
			previewMIDI(file, true);

		}
		if (file.getFileExtension() == ".drm")
		{
			loadDrum(file.getFullPathName());

			update_pat(true);

		}

	}
	int sizeMIDI(File file, int& v)
	{

		v = 0;
		short lv[256];
		for (int i = 0; i < 128; i++)
			lv[i] = -1;
		FileInputStream source(file);
		lastMidiFile = file;
		SMF.readFrom(source);
		int t = SMF.getLastTimestamp();
		if (t == 0)
			return 0;
		int note;
		const MidiMessageSequence* notes;
		MidiMessage* tempMessage;
		int tr = SMF.getNumTracks() - 1;
		for (int i = 0; i < SMF.getNumTracks(); i++)
		{

			notes = SMF.getTrack(i);
			int e = 0;
			tempMessage = &(notes->getEventPointer(0)->message);
			while (!tempMessage->isNoteOn() && e < notes->getNumEvents())
				tempMessage = &(notes->getEventPointer(e++)->message);
			if (tempMessage->getChannel() == 10)
			{
				// DBG(i << " " << tempMessage->getChannel());
				tr = i;
				break;
			}
		}
		notes = SMF.getTrack(tr);
		double lt = notes->getEndTime();
		int bcc = lt / SMF.getTimeFormat();
		bcc = 1 + bcc;
		for (int j = 0; j < notes->getNumEvents(); j++)
		{
			tempMessage = &(notes->getEventPointer(j)->message);
			if (tempMessage->isNoteOn())
			{
				byte note = tempMessage->getNoteNumber() % 128;
				if (lv[note] == -1)
				{
					lv[note] = note;
					v++;
				}
			}
		}
		nextEvent = 0;
		firstEvent = 0;

		midilastEvent = notes->getNumEvents();
		return bcc;
	}
private:
	//==============================================================================
	struct MidiDeviceListBox : public ListBox,
		private ListBoxModel
	{
		MidiDeviceListBox(const String& name,
			MidiHandler& contentComponent,
			bool isInputDeviceList) :
			parent(contentComponent),
			isInput(isInputDeviceList)
		{
			::ListBox(name, this);
			setOutlineThickness(1);
			setMultipleSelectionEnabled(true);
			setClickingTogglesRowSelection(true);
			setModel(this);
		}

		//==============================================================================
		int getNumRows() override
		{
			return isInput ? parent.getNumMidiInputs()
				: parent.getNumMidiOutputs();
		}

		void paintListBoxItem(int rowNumber, Graphics& g,
			int width, int height, bool rowIsSelected) override
		{
			auto textColour = getLookAndFeel().findColour(ListBox::textColourId);

			if (rowIsSelected)
				g.fillAll(textColour.interpolatedWith(getLookAndFeel().findColour(ListBox::backgroundColourId), 0.5));


			g.setColour(textColour);
			g.setFont((float)height * 0.7f);

			if (isInput)
			{
				if (rowNumber < parent.getNumMidiInputs())
					g.drawText(parent.getMidiDevice(rowNumber, true)->deviceInfo.name,
						5, 0, width, height,
						Justification::centredLeft, true);
			}
			else
			{
				if (rowNumber < parent.getNumMidiOutputs())
					g.drawText(parent.getMidiDevice(rowNumber, false)->deviceInfo.name,
						5, 0, width, height,
						Justification::centredLeft, true);
			}
		}

		//==============================================================================
		void selectedRowsChanged(int) override
		{
			auto newSelectedItems = getSelectedRows();
			if (newSelectedItems != lastSelectedItems)
			{
				for (auto i = 0; i < lastSelectedItems.size(); ++i)
				{
					if (!newSelectedItems.contains(lastSelectedItems[i]))
						parent.closeDevice(isInput, lastSelectedItems[i]);
				}

				for (auto i = 0; i < newSelectedItems.size(); ++i)
				{
					if (!lastSelectedItems.contains(newSelectedItems[i]))
						parent.openDevice(isInput, newSelectedItems[i]);
				}

				lastSelectedItems = newSelectedItems;
			}
		}

		//==============================================================================
		void syncSelectedItemsWithDeviceList(const ReferenceCountedArray<MidiDeviceListEntry>& midiDevices)
		{
			SparseSet<int> selectedRows;
			for (auto i = 0; i < midiDevices.size(); ++i)
				if (midiDevices[i]->inDevice.get() != nullptr || midiDevices[i]->outDevice.get() != nullptr)
					selectedRows.addRange(Range<int>(i, i + 1));

			lastSelectedItems = selectedRows;
			updateContent();
			setSelectedRows(selectedRows, dontSendNotification);
		}

	private:
		//==============================================================================
		MidiHandler& parent;
		bool isInput;
		SparseSet<int> lastSelectedItems;
	};

	//==============================================================================
	void handleIncomingMidiMessage(MidiInput* /*source*/, const MidiMessage& message) override
	{
		// This is called on the MIDI thread
		const ScopedLock sl(midiMonitorLock);
		if (message.isController())
		{
			lastcc = message.getControllerNumber();
			lastccval = message.getControllerValue();
			webgui.setMonitor(ccpatternidt, "CC " + String(lastcc));
			webgui.setMonitor(ccvpatternidt, "CC Value " + String(lastccval));

		}
		incomingMessages.add(message);
		triggerAsyncUpdate();
	}

	void handleAsyncUpdate() override
	{
		// This is called on the message loop
		Array<MidiMessage> messages;

		{
			const ScopedLock sl(midiMonitorLock);
			messages.swapWith(incomingMessages);
		}

		String messageText;

		for (auto& m : messages)
			messageText << m.getDescription() << "\n";

		midiMonitor.insertTextAtCaret(messageText);
	}

	void sendToOutputs(const MidiMessage& msg)
	{
		for (auto midiOutput : midiOutputs)
			if (midiOutput->outDevice.get() != nullptr)
				midiOutput->outDevice->sendMessageNow(msg);
	}

	//==============================================================================
	bool hasDeviceListChanged(const Array<MidiDeviceInfo>& availableDevices, bool isInputDevice)
	{
		ReferenceCountedArray<MidiDeviceListEntry>& midiDevices = isInputDevice ? midiInputs
			: midiOutputs;

		if (availableDevices.size() != midiDevices.size())
			return true;

		for (auto i = 0; i < availableDevices.size(); ++i)
			if (availableDevices[i] != midiDevices[i]->deviceInfo)
				return true;

		return false;
	}

	ReferenceCountedObjectPtr<MidiDeviceListEntry> findDevice(MidiDeviceInfo device, bool isInputDevice) const
	{
		const ReferenceCountedArray<MidiDeviceListEntry>& midiDevices = isInputDevice ? midiInputs
			: midiOutputs;

		for (auto& d : midiDevices)
			if (d->deviceInfo == device)
				return d;

		return nullptr;
	}

	void closeUnpluggedDevices(const Array<MidiDeviceInfo>& currentlyPluggedInDevices, bool isInputDevice)
	{
		ReferenceCountedArray<MidiDeviceListEntry>& midiDevices = isInputDevice ? midiInputs
			: midiOutputs;

		for (auto i = midiDevices.size(); --i >= 0;)
		{
			auto& d = *midiDevices[i];

			if (!currentlyPluggedInDevices.contains(d.deviceInfo))
			{
				if (isInputDevice ? d.inDevice.get() != nullptr
					: d.outDevice.get() != nullptr)
					closeDevice(isInputDevice, i);

				midiDevices.remove(i);
			}
		}
	}

	void updateDeviceList(bool isInputDeviceList)
	{
		auto availableDevices = isInputDeviceList ? MidiInput::getAvailableDevices()
			: MidiOutput::getAvailableDevices();
		int mi = midiInputSelector->getSelectedRow();
		int mo = midiOutputSelector->getSelectedRow();
		if (hasDeviceListChanged(availableDevices, isInputDeviceList) || mi == -1 || mo == -1)
		{

			ReferenceCountedArray<MidiDeviceListEntry>& midiDevices
				= isInputDeviceList ? midiInputs : midiOutputs;

			closeUnpluggedDevices(availableDevices, isInputDeviceList);

			ReferenceCountedArray<MidiDeviceListEntry> newDeviceList;

			// add all currently plugged-in devices to the device list
			for (auto& newDevice : availableDevices)
			{
				MidiDeviceListEntry::Ptr entry = findDevice(newDevice, isInputDeviceList);

				if (entry == nullptr)
					entry = new MidiDeviceListEntry(newDevice);

				newDeviceList.add(entry);
			}

			// actually update the device list
			midiDevices = newDeviceList;
			int size = midiDevices.size();
			// update the selection status of the combo-box
			if (miindex > -1 && isInputDeviceList && size > miindex)
				midiInputSelector->selectRow(miindex);
			else
				if (moindex > -1 && !isInputDeviceList && size > moindex)
					midiOutputSelector->selectRow(moindex);
			if (auto* midiSelector = isInputDeviceList ? midiInputSelector.get() : midiOutputSelector.get())
				midiSelector->syncSelectedItemsWithDeviceList(midiDevices);
		}
	}

	//==============================================================================
	void addLabelAndSetStyle(Label& label)
	{
		label.setFont(Font(15.00f, Font::plain));
		label.setJustificationType(Justification::centredLeft);
		label.setEditable(false, false, false);
		label.setColour(TextEditor::textColourId, Colours::black);
		label.setColour(TextEditor::backgroundColourId, Colour(0x00000000));

		addAndMakeVisible(label);
	}


	//==============================================================================
	Label midiInputLabel{ "Midi Input Label",  "MIDI Input:" };
	Label midiOutputLabel{ "Midi Output Label", "MIDI Output:" };
	Label incomingMidiLabel{ "Incoming Midi Label", "Received MIDI messages:" };
	Label outgoingMidiLabel{ "Outgoing Midi Label", "Play the keyboard to send MIDI messages..." };
	MidiKeyboardState keyboardState;
	MidiKeyboardComponent midiKeyboard;
	TextEditor midiMonitor{ "MIDI Monitor" };
	TextButton pairButton{ "MIDI Bluetooth devices..." };

	std::unique_ptr<MidiDeviceListBox> midiInputSelector, midiOutputSelector;
	ReferenceCountedArray<MidiDeviceListEntry> midiInputs, midiOutputs;

	CriticalSection midiMonitorLock;
	Array<MidiMessage> incomingMessages;

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiHandler)
};
void onMessage(juce::String value, int id)
{
	if (id == cli)
	{
		history = value;
		StringArray argv;
		argv.addTokens(value, " ", "\"");
		argv.removeEmptyStrings(true);
		int argc = argv.size();
		juce::String starts = argv[0].substring(0, 2);
		FDBG("starts with " + starts + " # " + SN(argc));
		webgui.remove(cliitem);
		File currentDirectory = File(File::getSpecialLocation(File::SpecialLocationType::globalApplicationsDirectoryX86).getFullPathName() +
			File::getSeparatorString() + "Toontrack" + File::getSeparatorString() + "EZDrummer" + File::getSeparatorString() + "Midi");
		//			File currentDirectory = File("C:\\Program Files(x86)\\Toontrack\\EZDrummer\\Midi");
		if (starts == "ls")
		{
			FDBG(currentDirectory);
			listDirectory(currentDirectory, argv[1]);
			outString = "<textarea class=\"scrollabletextbox\" name=\"note\" rows=10 cols=140>" + dirout + "</textarea>";
			setDisplay();
		}
		else if (starts == "sd")
		{
			shuffleDef = value.substring(3);
			StringArray shuf;
			shuf.addTokens(shuffleDef, " ", "\"");
			shufflemax = shuf.size();
			for (int i = 0; i < shufflemax; i++)
			{
				StringArray part;
				part.addTokens(shuf[i], "*", "\"");
				if (part.size() > 1)
				{
					shufflelength[i] = part[0].getIntValue();
					shufflebars[i] = (part[1].getIntValue() - 1) * 4;
				}
				else
				{
					shufflelength[i] = 1;
					shufflebars[i] = (part[0].getIntValue() - 1) * 4;
				}

			}
			shufflebarsidx = 0;
			shufflelengthidx = 0;
			return;
		}
		else if (starts == "lo")
		{
			if (argc == 1)
			{
				//               FDBG(__CALLER__ + " " + currentDirectory);
				midiTimer->myMidi->changeDir("", true);
				return;
			}
			if (argv[1] == "*")
			{
				int n = favdir.size();
				for (int i = 0; i < n; i++)
				{
					if (i)
						dirres[i] = "&#x1F4C1;" + favdir[i];
					else
						dirres[i] = favdir[i];
					if (i)
						res[i] = File(favdir[i]);
				}
				diropts(n);
				return;
			}
			if (argv[1] == "@")
			{
				int ap = actpattern;
				if (argc == 3)
					ap = (argv[2].getIntValue() - 1);
				File pdir = File(patfiles[ap * 4]).getParentDirectory();
				webgui.remove(cdid);
				int n = midiTimer->myMidi->loadDirectory(pdir, argv[1], 0);
				diropts(n);
				return;
			}
			else if (argv[1].endsWith(".mid") && !argv[1].contains("*"))
			{
				File frec = File(File::getSpecialLocation(File::SpecialLocationType::userDocumentsDirectory).getFullPathName() +
					File::getSeparatorString() + "TMS" + File::getSeparatorString() + argv[1]);

				midiTimer->myMidi->previewMIDI(frec, true);
			}
			else if (argv[1] == "~")
			{
				File frec = File(File::getSpecialLocation(File::SpecialLocationType::userDocumentsDirectory).getFullPathName() +
					File::getSeparatorString() + "TMS");

				int n = midiTimer->myMidi->loadDirectory(frec, "", 0);
				diropts(n);
			}
			else
			{
				String arg = argv[1];
				int ax = -1;
				String ext = "";
				if (argv[1].contains("*"))
				{
					ax = argv[1].indexOf("*");
					arg = argv[1].substring(0, ax - 1);
					ext = argv[1].substring(ax);
				}
				File frec = File(arg);

				int n = midiTimer->myMidi->loadDirectory(frec, ext, 0);
				diropts(n);
			}
		}
		else if (starts == "cl")
		{
			clearPat();
			update_pat();
		}
		else if (starts == "sh")
		{
			showData(argc, argv);
		}
	}

}
