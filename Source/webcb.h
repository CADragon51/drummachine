//extern void midiSilence(void);
#pragma once
#ifndef webcd_H
#define webcd_H
#include "globals.h"
#include "svgdraw.h"
int pwx;


void diropts(int n)
{
	webgui.remove(cdid);
	cdid = webgui.addOptions("files", n, dirres, &onOptionSelect, 0, 310, 0, "f");
	addedDisplay = true;
}
void update_pat(bool scroll = true)
{

	//   FDBG(__CALLER__);
	int ap = actpattern;
	if (patvoicelow[ap] < MAXVOI - 4 && !startOver)
		startvoice = patvoicelow[ap];

	webgui.remove(ccmetid);
	//   FDBG(__CALLER__);
	int end = startvoice + 4;

	for (int v = 0; v < 4; v++)
	{

		webgui.remove(instopt[v]);
		webgui.remove(velopt[v]);
		webgui.remove(metopt[v]);
		webgui.remove(metid[v]);
		//       webgui.remove(voiceidt[v]);
	}
	showCCs();
	showStatus(actpattern, scroll);

	int ypos = 500;
	webgui.setMonitor(patternidt, " Pattern #" + juce::String(actpattern + zerobase));
	for (int i = 0; i < maxticks; i++)
		seqswitch[i] = ccpattern[actpattern * maxticks + i] > 0;
	// ccpattern[actpattern * maxticks + i] > 0;
	ccmetid = webgui.addSwitches("steps", maxticks, seqswitch, &onSwitches, 730, ypos, "x", "nomonitor");
	//   webgui.setMonitor(ccpatternidt, "CC " + juce::String(lastcc < 128 ? lastcc : 0));
	//   webgui.setMonitor(ccvpatternidt, "CC Value " + juce::String(lastccval < 128 ? lastccval : 0));
	//   FDBG(__CALLER__);
	int sv = startvoice >= 0 ? startvoice : 0;
	end = sv + 4;
	for (int v = sv, vv = 0; v < end && v < MAXVOI; v++, vv++)
	{
		//       FDBG("update voice " + SN(v));
		int ypos = 550 + (vv) * 50;

		int inx = minstr[v];
		if (inx < 1 || inx > 127)
			inx = 1;
		if (pwx < 1 || pwx > 127)
			pwx = 16;
		//       FDBG("voice " + SN(voiceidt[vv]) + " " + SN(v + zerobase));
		//      webgui.remove(voiceidt[v]);
		webgui.setMonitor(voiceidt[vv], String(v + 1));
		instopt[vv] = webgui.addOptions("instr", 128, perc, &onOptionSelect, 270, ypos, inx, "f");
		velopt[vv] = webgui.addOptions("instr", 128, perc, &onOptionSelect, 460, ypos, mvelo[v], "f");
		metopt[vv] = webgui.addOptions("Pattern", pwx, patvals, &onOptionSelect, 560, ypos, lastmet[v], "f", "metro");

		for (int i = 0; i < 2; i++)
		{
			juce::String pas = pat2string(actpattern + i, v);
			juce::String r = showRhythm(pas, i * 10 + v);
			//          FDBG("pattern " + SN(i) + "/" + SN(v) + " " + pas + " " + r);
			webgui.setMonitor(patidt[vv][i], r);
		}

		for (int i = 0; i < maxticks; i++)
		{
			seqswitch[i] = seqpattern[actpattern * maxticks + i][v] > 0;
		}
		metid[vv] = webgui.addSwitches("steps", maxticks, seqswitch, &onSwitches, 730, ypos, "x", "nomonitor");
		//        FDBG(__CALLER__);
	}
	//    FDBG(__CALLER__);
}
void addPattern(short patin)
{
	//   FDBG(pat2string(patin) + " " +SN(patin));
	for (int i = 0; i < pwx; i++)
	{
		//       FDBG(patnames[i]);
		if (patnames[i] == patin)
			return;
	}
	if (pwx < 100)
	{
		patnames[pwx] = patin;
		patvals[pwx] = pat2string(patin, -1);
		//        FDBG(pwx);
		pwx++;
	}
}
bool cleared = false;
void onMessage(juce::String value, int id);
juce::String outString;

void setDisplay()
{
	//    FDBG(outString);
	int block = 1000;
	int ol = outString.length();
	//    FDBG(ol);
	webgui.remove(men);

	if (cliitem > 0)
		webgui.remove(cliitem);
	cliitem = webgui.addStringDisplay("cli", 0,200, "f", "nomonitor");

	if (ol < block)
		webgui.setMonitor(cliitem, outString);
	else
	{
		int n = ol / block;
		//       int r = ol % block;
		juce::String tmp = outString.substring(0, block);
		webgui.setMonitorT(cliitem, tmp, 1);
		//       FDBG(tmp);
		//       FDBG(SN(r) + SN(n) + SN(ol));
		for (int32_t i = 1; i < n; i++)
		{
			tmp = outString.substring(i * block, (i + 1) * block);
			webgui.setMonitorT(cliitem, tmp, 2);
			//            FDBG(tmp);
		}
		tmp = outString.substring(n * block);
		//       FDBG(tmp);
		webgui.setMonitorT(cliitem, tmp, 3);
	}
	cleared = true;
	addedDisplay = true;
}
void setBPM(int argc, juce::String* argv)
{
	mettime = 2000000 * 120.0 / 120;
	//            if (metison)
	clickc = 0;
	beatlength = argv[2].getIntValue();
	//  metTimer.end();
	  //          if (metison)
	{
		playSeq = true;

		//        metTimer.begin(playPattern, mettime / 48);
		patternc = -1;
	}
}
void clearPat(int from, int to)
{

	for (int p = from; p <= to; p++)
	{
		beatCount[p] = 0;
		patfiles[p] = "";
		acttrigger[p] = -1;
		if (actbeatID[p] > -1)
			webgui.remove(actbeatID[p]);
		actbeatID[p] = -1;
	}

	lastvoice = 0;
	for (int v = 0; v < MAXVOI; v++)
	{
		for (int p = from; p <= 12 * to; p++)
		{
			{
				seqpattern[p][v] = -1;
				if (!v)
					delaypattern[p] = 0;
			}
		}
	}
	update_pat(true);
}
void clearPat(void)
{

	for (int p = 0; p < 128; p++)
	{
		triggerNote[p] = -1;
		voices[p] = -1;
	}

	for (int p = 0; p < MAXPAT; p++)
	{
		beatCount[p] = 0;
		patfiles[p] = "";
		acttrigger[p] = -1;
	}

	lastvoice = 0;
	for (int v = 0; v < MAXVOI; v++)
	{
		for (int p = 0; p < 12 * MAXPAT; p++)
		{
			{
				seqpattern[p][v] = -1;
				if (!v)
					delaypattern[p] = 0;
			}
		}
	}
}


void saveData(int argc, juce::String* argv)
{
#if 0
	if (argc > 2 && argv[1] == "drm")
	{
		//        saveDrum(argv[2]);
		return;
	}
	if (argc > 1)
	{
		juce::String pname = argv[1];
		FDBG("Saving " + pname);
		int lE = lastEvent;

		if (argv[1].indexOf(".mid") > -1 && lE > 0)
		{
			seqFile = pname;
			if (SD.exists(pname.c_str()))
			{
				SD.remove(pname.c_str());
				//    if(debug==1)DBG("ini deleted");
			}
			File frec = SD.open(pname.c_str(), FILE_WRITE);
			if (frec)
			{
				createMidiFile();
				int delta = 0;
				int lasttime = 0;

				//            FDBG("midi " + SN(midiptr));
				for (int s = 0; s < lE; s++)
				{
					delta = sequences[s]._time - lasttime;
					//                FDBG(" " + SB(sequences[s]._event) + " delta " + SN(delta) + " " + SN(midiptr-22));
					writeToFile(sequences[s]._event + sequences[s]._channel, sequences[s]._note, sequences[s]._velocity, delta);
					lasttime = sequences[s]._time;
				}
				setTrackLength(midiptr - 14);
				Serial.printf("\n%04x ", 0);
				for (int i = 0; i < midiptr; i++)
				{
					Serial.printf("%02x ", midifile[i]);
					if (i % 16 == 15)
						Serial.printf("\n%04x ", i + 1);
				}
				Serial.println();
				frec.write(midifile, midiptr);
				frec.close();
			}
		}
#endif

}
	juce::String outarea = "<textarea class=\"scrollabletextbox\" name=\"note\" rows=10 cols=40>";
	void showData(int argc, juce::StringArray argv)
	{
		if (argc < 2)
			return;
		juce::String event = "";
		if (argv[1].indexOf("seq") > -1)
		{
			event = "step time event note velocity~";
			for (int s = 0; s < lastEvent; s++)
			{
				event += SN(s) + " " + sequences[s].show() + "~";
			}
			//           FDBG(out);
		}
		if (argv[1].indexOf("drm") > -1)
		{
			event = saveDrum("", true);
			FDBG(event);
		}
		outString = outarea + event + "</textarea>";
		int ol = outString.length();
		FDBG(ol);
		setDisplay();
	}
	void procVoice(juce::String proc, int from, int to)
	{
		if (proc == "cp")
		{
			FDBG("copy voice from " + SN(from) + SN(to));
			for (int i = startpattern * maxticks; i < actpattern * maxticks; i++)
			{
				seqpattern[i][to] = seqpattern[i][from];
			}
		}
		if (proc == "mv")
		{
			FDBG("move voice from " + SN(from) + SN(to));
			for (int i = startpattern * maxticks; i < actpattern * maxticks; i++)
			{
				seqpattern[i][to] = seqpattern[i][from];
				seqpattern[i][from] = -1;
			}
		}
		if (proc == "sw")
		{
			FDBG("swap voice from " + SN(from) + SN(to));
			for (int i = startpattern * maxticks; i < actpattern * maxticks; i++)
			{
				short sw = seqpattern[i][to];
				seqpattern[i][to] = seqpattern[i][from];
				seqpattern[i][from] = sw;
			}
		}
		if (proc == "rm")
		{
			FDBG("delete voice from " + SN(from) + SN(to));
			for (int i = startpattern * maxticks; i < actpattern * maxticks; i++)
			{
				seqpattern[i][from] = -1;
			}
		}
		actpattern = startpattern;
		update_pat(false);
	}
	juce::String mos = "";
	//  juce::String outString;



	void onButtonClick(int button, int id)
	{
		FDBG(juce::String(id) + F(" button was clicked") + " " + juce::String(button));

		refresh = true;
	}
	class DrumTimer;

	int backState = 0;
	extern void websetup(void);
	//	extern DrumTimer::Ptr metTimer;

	void onSlider(float value, int id)
	{

	}

	void onSwitches(bool* value, int id)
	{
		FDBG("New switch status: " + SN(id) + " " + SN(value[0] ? "on" : "off"));
		//    if (menuState == SETTINGS)
		{
			if (id == actpatid)
			{
				editMode = value[0];
				return;
			}
			//       short ap = (actpattern / (beatlength));
			if (id == ccmetid)
			{
				for (int i = 0; i < maxticks; i++)
				{
					int p = actpattern * maxticks + i;
					//               FDBG("old cc " +SN(i)+ SN(value[i]) + " @" + SN(ccpattern[p]) + SN(ccval[p]));
					if (value[i] && ccpattern[p] == 0)
					{
						ccpattern[p] = lastcc;
						ccval[p] = lastccval;
						//                   FDBG("set cc " + SN(value[i]) + " @" + SN(ccpattern[p]) + SN(ccval[p]));
					}
					else if (!value[i])
					{
						ccpattern[p] = 0;
						ccval[p] = 0;
					}
				}
				showCCs();
			}

			if (patvoicelow[actpattern] < MAXVOI - 4)
				startvoice = patvoicelow[actpattern];

			for (int v = 0; v < 4; v++)
			{
				if (id == metid[v])
				{
					for (int i = 11; i > -1; i--)
					{
						int p = actpattern * maxticks + i;
						//                  FDBG("set metro " + SN(value[i]) + " @" + SN(p));
						seqpattern[p][v + startvoice] = (value[i] ? mvelo[v + startvoice] : -1);
						//                   if (seqpattern[p][v] > 0)
						//                       seqpattern[p][v] = 0;
					}
					juce::String pas = pat2string(actpattern, v + startvoice);
					juce::String r = showRhythm(pas, actpattern * 10 + v + startvoice);
					//               FDBG("pattern " + SN(i) + "/" + SN(v) + " " + pas + " " + r);
					//              webgui.remove(patidt[v][0]);
					//               int ypos = 450 + (v - startvoice) * 50;
					// patidt[v][0] = webgui.addStringDisplay("Rhythm", 1000, ypos - 10, "f");
					webgui.setMonitor(patidt[v][0], r);
				}
			}
		}

	}
#endif