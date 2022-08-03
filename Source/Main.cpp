/*
  ==============================================================================

	This file contains the basic startup code for a JUCE application.

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Midigui.h"
#include "webgui.h"
#include "globals.h"
#include "webinter.h"
using namespace juce;


//==============================================================================
class Application : public juce::JUCEApplication
{
public:
	//==============================================================================
	Application() {}

	const juce::String getApplicationName() override { return ProjectInfo::projectName; }
	const juce::String getApplicationVersion() override { return ProjectInfo::versionString; }
	bool moreThanOneInstanceAllowed() override { return true; }
	void initialise(const juce::String&) override
	{
		mainWindow.reset(new MainWindow("MidiDemo", new MidiHandler, *this));
	}

	void shutdown() override
	{
		// Add your application's shutdown code here..

		mainWindow = nullptr; // (deletes our window)
	}


	//==============================================================================
	void systemRequestedQuit() override
	{
		// This is called when the app is being asked to quit: you can ignore this
		// request and let the app carry on running, or call quit() to allow the app to close.
		quit();
	}

	void anotherInstanceStarted(const juce::String& commandLine) override
	{
		// When another instance of the app is launched while this one is running,
		// this method is invoked, and the commandLine parameter tells you what
		// the other instance's command-line arguments were.
	}

	//==============================================================================
	/*
		This class implements the desktop window that contains an instance of
		our MainComponent class.
	*/
	class MainWindow : public juce::DocumentWindow, private Timer
	{
	public:
		const int kPortNumber = TCPPORT;

		MainWindow(const juce::String& name, juce::Component* c, JUCEApplication& a)
			: DocumentWindow(name, juce::Desktop::getInstance().getDefaultLookAndFeel()
				.findColour(ResizableWindow::backgroundColourId),
				juce::DocumentWindow::allButtons),
			app(a)
		{
			myMidi = (MidiHandler*)c;
			for (int i = 0; i < 128; i++)
			{
				counters[i] = String(i);
				triggerNote[i] = -1;
				voices[i] = 255;
			}
			setUsingNativeTitleBar(true);
			setContentOwned(c, true);
			clearPat();
			metTimer = new DrumTimer(25 / 6, myMidi);
			midiTimer = new MidiTimer(1, myMidi);
			webguiclient.waitUntilReady(true, 100);
			webgui.connect(&webguiclient, "localhost");
			//			favdir.add("E:/midicsv");
			//		favdir.add(File::getSpecialLocation(File::SpecialLocationType::globalApplicationsDirectoryX86).getFullPathName() +
			//				File::getSeparatorString() + "Toontrack" + File::getSeparatorString() + "EZDrummer" + File::getSeparatorString() + "Midi");
			File myFile = File(File::getSpecialLocation(File::SpecialLocationType::userDocumentsDirectory).getFullPathName() +
				File::getSeparatorString() + "TMS" + File::getSeparatorString() + "patterns.pat");
			//			favdir.add(File::getSpecialLocation(File::SpecialLocationType::userDocumentsDirectory).getFullPathName() +
			//				File::getSeparatorString() + "TMS");
			//			String allfd = favdir.joinIntoString(",");
			if (myFile.exists())
			{
				pwx = 0;
				patnames[pwx] = 0;
				patvals[pwx] = "PATTERN";
				pwx++;
				patnames[pwx] = 0;
				patvals[pwx] = " ";
				pwx++;
				String text = myFile.loadFileAsString();
				StringArray lines;
				lines.addTokens(text, "\n", "\"");
				for (int l = 0; l < lines.size(); l++)
				{
					String fi = lines[l];
					//         FDBG(fi + " " + SN(pwx));
					if (fi.length() > 0 && fi.getIntValue() < 4096)
					{
						patnames[pwx] = fi.getIntValue();
						patvals[pwx] = pat2string(fi.getIntValue(), -1);
						pwx++;
					}
				}
			}
			websetup(myMidi);
			loadDrum("TMS");
			update_pat(false);
			showStatus(patcnt, true);
			webloop();
			File frec = File(File::getSpecialLocation(File::SpecialLocationType::userDocumentsDirectory).getFullPathName() +
				File::getSeparatorString() + "TMS" + File::getSeparatorString() + "TMS.ini");
			if (frec.exists())
			{
				XmlDocument xmlstr(frec.loadFileAsString());
				TMSsettings.restoreFromXml(*xmlstr.getDocumentElement());
				String lm = TMSsettings.getValue("lm");
				shuffleDef = TMSsettings.getValue("sd");
				history = TMSsettings.getValue("hs");
				myMidi->moindex = TMSsettings.getIntValue("mo");
				myMidi->miindex = TMSsettings.getIntValue("mi");
				String fd = TMSsettings.getValue("fd");
				favdir.addTokens(fd, "`", "\"");
				File frec = File(lm);
				if (myMidi->setMidi(frec))
					myMidi->previewMIDI(frec, true);

			}
			sRand = Random(0);
			StringArray shuf;
			shuf.addTokens(shuffleDef, " ", "\"");
			shufflemax = shuf.size();
			int group = -1;
			for (int i = 0; i < shufflemax; i++)
			{
				int plx = shuf[i].indexOf("+");
				int asx = shuf[i].indexOf("*");
				int grx = shuf[i].indexOf(">");
				int cpx = shuf[i].indexOf(")");
				int opx = shuf[i].indexOf("(");
				shuffleinc[i] = 0;
				shufflerand[i] = 0;
				shufflegrp[i] = -1;
				if (opx != -1)
					group = i;
				Range< int >rr(1, (const int)shuf[i].getIntValue());

				if (plx == -1 && asx == -1 && grx == -1)
				{
					shufflelength[i] = 1;
					shufflebars[i] = (shuf[i].getIntValue() - 1) * 4;
					shufflegrp[i] = group;
					continue;
				}
				if (plx == -1 && asx != -1 && grx == -1)
				{
					shufflelength[i] = shuf[i].getIntValue();
					shufflebars[i] = (shuf[i].substring(asx+1).getIntValue() - 1) * 4;
					continue;
				}
				if (plx != -1 && asx != -1 && grx == -1)
				{
					shufflelength[i] = shuf[i].getIntValue();
					shufflebars[i] = (shuf[i].substring(asx + 1).getIntValue() - 1) * 4;
					if(opx!=-1)
						shufflebars[i] = (shuf[i].substring(opx +1).getIntValue() - 1) * 4;
					shuffleinc[i] = shuf[i].substring(plx + 1).getIntValue();
					continue;
				}
				if (plx != -1 && asx == -1 && grx != -1)
				{
					shufflelength[i] = sRand.nextInt(rr);
					shufflerand[i] = shuf[i].getIntValue();
					shufflebars[i] = (shuf[i].substring(grx + 1).getIntValue() - 1) * 4;
					if (opx != -1)
						shufflebars[i] = (shuf[i].substring(opx + 1).getIntValue() - 1) * 4;
					shuffleinc[i] = shuf[i].substring(plx + 1).getIntValue();
					continue;
				}
				if (plx == -1 && asx == -1 && grx != -1)
				{
					shufflelength[i] = sRand.nextInt(rr);
					shufflerand[i] = shuf[i].getIntValue();
					shufflebars[i] = (shuf[i].substring(grx + 1).getIntValue() - 1) * 4;
					if (opx != -1)
						shufflebars[i] = (shuf[i].substring(opx + 1).getIntValue() - 1) * 4;
					continue;
				}
				if (cpx != -1)
					group =-1;

			}
			shufflebarsidx = 0;
			shufflelengthidx = 0;
	//		isShuffle = true;


#if JUCE_ANDROID || JUCE_IOS
			setFullScreen(true);
#else
			setResizable(true, false);
			setResizeLimits(300, 250, 10000, 10000);
			centreWithSize(getWidth(), getHeight());
#endif
			startTimer(100);
			setVisible(true);

		}
		void closeButtonPressed() override
		{
#if 1
			saveDrum("TMS");
			File frec = File(File::getSpecialLocation(File::SpecialLocationType::userDocumentsDirectory).getFullPathName() +
				File::getSeparatorString() + "TMS" + File::getSeparatorString() + "TMS.ini");
			frec.deleteFile();
			frec.create();
			var lastm = lastMidiFile.getFullPathName();
			TMSsettings.setValue("lm", lastm);
			TMSsettings.setValue("hs", history);
			TMSsettings.setValue("mo", midiTimer->myMidi->moindex);
			TMSsettings.setValue("mi", midiTimer->myMidi->miindex);
			TMSsettings.setValue("fd", favdir.joinIntoString("`"));
			TMSsettings.setValue("sd",shuffleDef);

			std::unique_ptr<XmlElement> xm = TMSsettings.createXml("TMS");
			xm->writeToFile(frec, "");
#endif
			metTimer = nullptr;
			midiTimer = nullptr;
			SMF.clear();
			SMF2.clear();
			app.systemRequestedQuit();
		}
		MidiHandler* myMidi;
		void timerCallback()
		{
			webloop();
		}

	private:
		JUCEApplication& app;

		//==============================================================================
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
	};

	std::unique_ptr<MainWindow> mainWindow;

};

//==============================================================================
START_JUCE_APPLICATION(Application)

void createMidi(String pname)
{


	File frec = File(File::getSpecialLocation(File::SpecialLocationType::userDocumentsDirectory).getFullPathName() +
		File::getSeparatorString() + "TMS" + File::getSeparatorString() + pname + ".mid");

	FileOutputStream stream(frec);

	if (stream.openedOk())
	{
		stream.setPosition(0);
		stream.truncate();
		createMidiFile();
		int delta = 0;
		int lasttime = 0;
		int lE = lastEvent;

		//            FDBG("midi " + SN(midiptr));
		laste = 0;
		for (int s = 0; s < lE; s++)
		{
			delta = sequences[s]._time - lasttime;
			writeToFile(sequences[s]._event + sequences[s]._channel, sequences[s]._note, sequences[s]._velocity, delta);
			lasttime = sequences[s]._time;
		}
		setTrackLength(midiptr - 14);
		stream.write(midifile, midiptr);
		stream.flush();
	}

}

void createClip(void)
{
	MidiFile file = midiTimer->myMidi->clip2Midi();

	String fn = lastMidiFile.getFileNameWithoutExtension() + "_" + String(zoomStart + 1) + "-" + String(zoomEnd + 1) + ".mid";
	File frec = File(File::getSpecialLocation(File::SpecialLocationType::userDocumentsDirectory).getFullPathName() +
		File::getSeparatorString() + "TMS" + File::getSeparatorString() + fn);
	FileOutputStream stream(frec);
	if (stream.openedOk())
	{
		stream.setPosition(0);
		stream.truncate();
	}
	file.writeTo(stream);
}

void showVoiceline(void);
void doButtons(int);
void onButtonRelease(int button, int id)
{
	int inc = button == 0 ? 1 : -1;
	static int lastpat = -1;
	//    FDBG(juce::String(id) + F(" button was released") + " " + juce::String(button) + " @ " + SN(Menus[menuState]->backstate));


	{
		for (int i = 0; i < 32; i++)
		{
			if (midiVoiceID[i] == id)
			{
				selvoice[i] = !selvoice[i];
				voicestat[i] = 0;
				showVoiceline();
				return;
			}
		}

		for (int i = 0; i < 1024; i++)
		{
			if (midiActbeatID[i] == id)
			{
				actMidibeat = i;
				if (startMidi == i)
				{
					startMidi = -1;
				}
				else if (lastMidi == i)
				{
					lastMidi = -1;
				}
				else if (startMidi == -1)
					startMidi = actMidibeat;
				else if (lastMidi == -1)
					lastMidi = actMidibeat;
				if (startMidi > lastMidi)
				{
					int m = lastMidi;
					lastMidi = startMidi;
					startMidi = m;
				}
				showBeats();
				return;
			}
		}
		for (int i = 0; i < MAXPAT; i++)
		{
			if (actbeatID[i] == id)
			{

				if (!metison)
					actpattern = i;
				//             FDBG("new " + SN(i));
				if (patvoicelow[actpattern] < MAXVOI - 4)
					startvoice = patvoicelow[actpattern];
				update_pat(false);
				showStatus();
				if (editMode || beatCount[i] == 0)
					return;
				//				if (metison )
				//				{
				//					metison = false;
				//					metTimer->stopTimer();
				//					return;
				//				}
								//               FDBG(SN(i)+SN(beatCount[i]));
				int ap = i;
				//               FDBG(SN(i) + SN(beatCount[i]));



				newpatternc = ap * maxticks;
				if (!metison)
				{
					metison = true;
					isShuffle = false;
					mettime = 2000 * 120.0 / 120;
					metTimer->startTimer(mettime / 48);
					playSeq = true;
				}
				else if (lastpat == ap)
				{
					isShuffle = false;
					metison = false;
					mettime = 2000 * 120.0 / 120;
					metTimer->stopTimer();
					playSeq = true;
				}
				lastpat = ap;
				return;
			}
		}
		for (int i = 0; i < MAXMOVE; i++)
		{
			if (movenext[i] == id)
			{
				doButtons(i);
				return;
			}
		}
	}
	if (id == backmetro)
		metisback = !metisback;
	if (id == metro)
	{
		metison = !metison;
		isShuffle = false;
		if (metison)
		{
			clickc = 0;
			mettime = 2000 * 120.0 / 120;
			metTimer->stopTimer();
			patternc = actpattern * maxticks;
			metTimer->startTimer(mettime / 48);

		}
		else
		{
			metTimer->stopTimer();
			//           midiSilence();
			showStatus(actpattern, false);
		}
		return;
	}


	if (id == openFile)
	{
		static bool notfile = true;
		//        FDBG(cliitem);
		if (!addedDisplay)
			webgui.remove(cli);
		else
		{
			webgui.remove(cliitem);
			addedDisplay = false;
			webgui.remove(impid);
			webgui.remove(cdid);
		}
		inimport = false;
		//       FDBG("clear? " + SB(cleared));
		if (notfile)
		{
			webgui.remove(cli);
			cli = webgui.addInputString("Command ", &onMessage, 0, 250, "title", "control", history);
		}
		notfile = !notfile;
	}
	else if (id == sipm)
	{
		//        FDBG("transport " + SN(button - id * 10));
		transport = button - id * 10;
		myMidi->runtime = myMidi->evs->start;
		nextEvent = firstEvent;
		switch (transport)
		{
		case SHUFFLE:
			isShuffle = true;
		case RECORDING:
			MidiEvent::starttime = 0;
			shufflebarsidx=0;
			shufflelengthidx = 0;
			playSeq = true;
			lastEvent = 0;
			clickc = 0;
			mettime = 2000 * 120.0 / 120;
			metTimer->stopTimer();
			patternc = actpattern * maxticks;
			metTimer->startTimer(mettime / 48);
			metison = true;
			break;
		case STOPPED:
		{

			juce::String notes[16];
			;
			metison = false;
			metTimer->stopTimer();
			showStatus(actpattern, false);

			sequences[lastEvent].init(0x90, sequences[lastEvent - 1]._note, 0, sequences[lastEvent - 1]._channel);
			sequences[lastEvent]._time = sequences[lastEvent - 1]._time + 10;
			lastEvent++;

			//            midiSilence();
			;
		}
		break;
		case PLAYING:
		case REPEAT:
			isShuffle = false;
			midiTimer->startTimer(1);
			webgui.setMonitor(sbp, greenled);
			break;
		case CONVERT:
			;
			nextEvent = 0;
			firstEvent = 0;
			createMidi("TMS");
			break;
		}
	}

}

void showBeats()
{
	String header = " <svg height=\"600\" width=\"1400\">";
	String vnl = header;
	int s = zoomStart;
	int e = zoomEnd;
	//	DBG(s << " " << e << " " << minc << " " << delta << " " << startMidi << " " << lastMidi);
	if (e - s <= 8)
	{
		for (int i = s * 4; i < (e + 1) * 4 - 3; i++)
		{
			int x = (i - s * 4) * delta / 4;
			//			x *= midiTimer->myMidi->beatfactor;
			x += 5;
			if (i % 4)
				vnl += showLine(x, 30, x, ly, 1);
			else
			{
				if (i / 4 == startMidi || i / 4 == lastMidi)
					vnl += showLine(x, 30, x, ly, 4, "gold");
				else
					vnl += showLine(x, 30, x, ly, 2);
				if (i > (1 + e) * 4)
					break;

			}
		}
	}
	else
	{
		for (int i = s; i < e + 1; i += minc)
		{
			int x = (i - s) * delta;
			//			x *= midiTimer->myMidi->beatfactor;
			x += 5;
			if (i == startMidi || i == lastMidi)
				vnl += showLine(x, 30, x, ly, 3, "gold");
			else
				vnl += showLine(x, 30, x, ly, 1);
		}
	}
	vnl += svgend;
	webgui.setMonitor(beatline, vnl);
}
void showVoiceline(void)
{
	String header = " <svg height=\"600\" width=\"1400\">";
	String vnl = header;
	soloPlay = false;
	for (int i = 0; i < 128; i++)
	{
		int ix = np[i];
		if (ix == -1)
			continue;
		int y = 80 + ix * 20;
		if (selvoice[ix])
		{
			DBG(ix << " " << " " << voicestat[ix]);
			if (voicestat[ix] == 0)
				vnl += showLine(0, y, 1220, y, 3, "aqua");
			if (voicestat[ix] == 1)
			{
				vnl += showLine(0, y, 1220, y, 3, "yellow");
				soloPlay = true;
			}
			if (voicestat[ix] == 2)
				vnl += showLine(0, y, 1220, y, 3, "red");
			if (voicestat[ix] == 4)
				vnl += showLine(0, y, 1220, y, 3, "green");
			if (voicestat[ix] == 5)
			{
				vnl += showLine(0, y, 1220, y, 3, "orange");
				soloPlay = true;
			}
		}
		else
			vnl += showLine(0, y, 1220, y, 1);
		if (y > ly)
			ly = y;
	}
	vnl += svgend;
	webgui.setMonitor(voiceline, vnl);
}


void DrumTimer::hiResTimerCallback()
{
	myMidi->playPattern();
}
void  MidiTimer::hiResTimerCallback()
{
	if (SMF.getNumTracks() == 0 || !myMidi->playMidi())
		stopTimer();
}

void onOptionSelect(int option, int id)
{
	if (id == drumopt)
	{
		for (int v = 0; v < 32; v++)
		{
			if (selvoice[v])
			{
				if (voicestat[v] < 4 && option != 0)
				{
					voicestat[v] += 4;
					showVoiceline();
				}
				if (voicestat[v] >= 4 && option == 0)
				{
					voicestat[v] -= 4;
					for (int i = 0; i < 128; i++)
					{
						if (np[i] == v)
						{
							option = i;
							break;
						}
					}
					showVoiceline();
				}
				voicenote[v] = option;
				webgui.setMonitor(midiNoteID[v], String(voicenote[v]));
				return;
			}
		}
		return;
	}
	if (id == actgrpopt)
	{
		actpattern = option * 64;
		selopt = option;
		if (patvoicelow[actpattern] < MAXVOI - 4)
			startvoice = patvoicelow[actpattern];

		sa = (actpattern / 64) * 64;
		update_pat(true);
		return;
	}	for (int vv = startvoice; vv < MAXVOI && vv < startvoice + 4; vv++)
	{
		int v = (vv - startvoice) % 4;
		if (id == metopt[v])
		{
			//           FDBG("met " + SN(option) + " " + SN(patnames[option]));
			lastmet[v] = option;
			int pos = actpattern * maxticks;
			//       FDBG("pos " + SN(pos) + " " + SN(actpattern) + " "+ SN(actpat));
			for (int i = 0; i < maxticks; i++)
			{
				bool pt = (patnames[option] & (1 << i)) > 0;
				if (pt)
				{
					seqpattern[i + pos][v] = 100;
				}
				else
					seqpattern[i + pos][v] = -1;
				//           FDBG(metid[i]);
			}
			update_pat(false);
			return;
		}
		if (id == instopt[v])
		{
			minstr[v] = option;
			short voicestat[32];
			//           FDBG("met instr " + SN(minstr[v]));
			saveTMS();
			return;
		}
		if (id == velopt[v])
		{
			mvelo[v] = option;
			saveTMS();
			return;
		}
	}
	//   FDBG(SN(id) + " " + SN(upopt) + " " + SN(option) + " " + SN(mclick1) + " " + SN(mclick2));

	if (id == cdid)
	{

		juce::String opt = dirres[option];
		int nx = opt.getIntValue();
		if (opt.contains("arr;"))
		{
			int n = midiTimer->myMidi->loadDirectory(res[0], "*.mid;*.drm", nx);
			diropts(n);
			return;
		}
		File entry = res[option];
		if (!entry.exists())
			return;
		bool isd = entry.isDirectory();
		if (!isd)
		{
			midiTimer->myMidi->onFileSelect(entry, true);
			return;
		}
		int n = midiTimer->myMidi->loadDirectory(res[option], "*.mid;*.drm", nx);
		diropts(n);
		return;
	}
	refresh = true;
}
void doButtons(int i)
{
	switch (i)
	{
	case 0:
	{
		if (editMode && startpattern > -1 && lastpattern > startpattern)
			clearPat(startpattern, lastpattern);
	}
	break;
	case 9:
	{
		if (editMode && startpattern > -1 && lastpattern > startpattern)
		{
			int np = (lastpattern + 1 - startpattern);
			//                    FDBG(np);
			juce::String pcol = coloring[lastColor % 4];
			//    FDBG(patfiles[actpattern] + " LC " + SN(actpattern) + SN(lastColor) + SN(pcol));
			lastColor++;
			for (int a = startpattern, b = np; a <= lastpattern; a++, b--)
			{
				beatCount[a] = b;
				patcolors[a] = pcol;
				//                       FDBG(SN(np) + " " + SN(a) + " set beats " + SN(beatCount[a]));
			}
		}
	}
	break;
	case 3:
	{
		metison = false;
		transport = STOPPED;
		int v = 0;
		int bc = midiTimer->myMidi->sizeMIDI(lastMidiFile, v);
		midiTimer->myMidi->loadMIDI(lastMidiFile, bc, v);
		if (editMode)
			actpattern += bc;
		if (actpattern > MAXPAT - 1)
		{
			actpattern = MAXPAT - 1;
		}
	}
	break;
	case 1:
		actpattern--;

	case 2:
		if (i == 2)
			actpattern++;
		if (actpattern < 0)
		{
			actpattern = 0;
		}
		if (actpattern > MAXPAT - 1)
		{
			actpattern = MAXPAT - 1;
		}
		if (patvoicelow[actpattern] < MAXVOI - 4)
			startvoice = patvoicelow[actpattern];
		startOver = false;

		break;
	case 5:
		startOver = true;
		startvoice--;
	case 4:
		startOver = true;
		if (i == 4)
		{
			if (startvoice != -1)
				startvoice++;
		}
		if (startvoice < -1)
			startvoice = -1;
		if (startvoice > MAXVOI - 4)
			startvoice = MAXVOI - 4;

		break;
	case 6:
	{
		if (editMode)
			startpattern = actpattern;
	}
	break;
	case 10:
	{
		startMidi = -1;
		lastMidi = -1;
		zoomStart = 0;
		zoomEnd = bcc;
		midiTimer->myMidi->previewMIDI(lastMidiFile, false);
		int s = 0;
		int e = bcc;
		int es = 0;
		int ee = events.size() - 1;
		es = bc2event[0];
		ee = bc2event[bcc];
		firstEvent = events[es]->eventnr;
		midiTimer->myMidi->midilastEvent = events[ee]->eventnr;

		return;
	}
	break;
	case 11:
	{
		lastMidi = actMidibeat;
		midiTimer->myMidi->previewMIDI(lastMidiFile, false);
		nextEvent = firstEvent;
		midiTimer->myMidi->runtime = midiTimer->myMidi->evs->start;
		midiTimer->startTimer(1);
		transport = PLAYING;
		return;
	}
	break;
	case 13:
	{
		for (int v = 0; v < 32; v++)
		{
			if (selvoice[v])
			{
				if (voicestat[v] == 1 || voicestat[v] == 5)
					voicestat[v]--;

				else
					voicestat[v] += 1;
			}
		}
		showVoiceline();
		return;
	}
	break;
	case 15:
	{
		for (int v = 0; v < 32; v++)
		{
			if (selvoice[v])
			{
				voiceamp[v] -= 0.1;
				if (voiceamp[v] <= 0)
					voiceamp[v] = 0;
				webgui.setMonitor(midiVelID[v], String(voiceamp[v]));

			}
		}
		return;
	}
	break;
	case 17:
	{
		nextEvent = 0;
		firstEvent = 0;

		createClip();
		return;
		break;

	}
	case 18:
	{

		metison = false;
		transport = STOPPED;
		int bc = 4 * (lastMidi - startMidi + 1);
		midiTimer->myMidi->loadClip(bc);
		if (editMode)
			actpattern += bc;
		if (actpattern > MAXPAT - 1)
		{
			actpattern = MAXPAT - 1;
		}

		return;
	}
	break;
	case 19:
	{
		std::unique_ptr<FileChooser> myChooser;

		myChooser = std::make_unique<FileChooser>("Add to Favorites",
			File::getSpecialLocation(File::userHomeDirectory));
		myChooser->browseForDirectory();
		File res = myChooser->getResult();
		if (res.isDirectory())
			favdir.add(res.getFullPathName());
	}
	break;
	case 14:
	{
		for (int v = 0; v < 32; v++)
		{
			if (selvoice[v])
			{
				voiceamp[v] += 0.1;
				webgui.setMonitor(midiVelID[v], String(voiceamp[v]));

			}
		}
		return;
	}
	break;
	case 12:
	{
		for (int v = 0; v < 32; v++)
		{
			if (selvoice[v])
			{
				if (voicestat[v] == 2)
					voicestat[v] = 0;
				else
					voicestat[v] = 2;
			}
		}
		showVoiceline();
		return;
	}
	break;
	case 7:
	{
		if (startpattern != -1)
		{
			lastpattern = actpattern - 1;
			{
				for (int a = startpattern * maxticks, b = 0; a <= lastpattern * maxticks; a++, b++)
				{
					for (int v = 0; v < MAXVOI; v++)
						velpattern[a][v] = seqpattern[a][v];
				}
				//                       FDBG(SN(editMode) + SN(lastpattern));
			}
		}

	}
	break;
	case 8:
	{
		saveDrum("TMS");
		File frec = File(File::getSpecialLocation(File::SpecialLocationType::userDocumentsDirectory).getFullPathName() +
			File::getSeparatorString() + "TMS" + File::getSeparatorString() + "TMS.ini");
		frec.deleteFile();
		frec.create();
		var lastm = lastMidiFile.getFullPathName();
		TMSsettings.setValue("lm", lastm);
		TMSsettings.setValue("hs", history);
		TMSsettings.setValue("mo", midiTimer->myMidi->moindex);
		TMSsettings.setValue("mi", midiTimer->myMidi->miindex);
		TMSsettings.setValue("fd", favdir.joinIntoString("`"));

		std::unique_ptr<XmlElement> xm = TMSsettings.createXml("TMS");
		xm->writeToFile(frec, "");
	}
	break;
	case 16:
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
		break;
	}
	update_pat(true);
	return;
}

