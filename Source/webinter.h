#include "webdata.h"

#include "webcb.h"

void websetup(MidiHandler* midip)
{
	myMidi = midip;
	sbase = 470;
	tbase = sbase + 15;
	fbase = tbase + 40;
	xbase = fbase + 20;
	base5 = xbase + 80;
	base6 = base5 + 20;
	base7 = base6 + 30;
	base8 = base7 + 50;
	base9 = base8 + 50;
	base10 = base9 + 50;
	maxg = 0;
	//   DBG("Submenu " + name + " my ID " + SN(sb->myID));
	int myID = 0;
	lastXPos = 0;
	//   DBG(F("Connected to webgui server !"));
	webgui.reset(); // clear the board
					//  bgitem = webgui.addStringDisplay("_bg_", 0, 0, false, "nomonitor");
	for (int p = 0; p < MAXPAT; p++)
	{
		actbeatID[p] = -1;
	}

	;
	selitem = webgui.addStringDisplay("<t3>TMS</t3>", 0, 0, "f", "nomonitor");


	metro = webgui.addButtons(btns7[3], &onButtonRelease, 0, mbase, "f");
	openFile = webgui.addButtons(btns7[5], &onButtonRelease, 50, mbase, "f");

	webgui.setMonitor(men, "Hello, me TMS");
	cliitem = -1;

	if (selitem == -1)
		selitem = webgui.addStringDisplay("<t3>TMS</t3>", 0, 0, "f", "nomonitor");
	;
	webgui.setMonitor(selitem, "<t3>Rhythm</t3>_rhy_");
	actpatid = webgui.addSwitches("edit", 1, actpat, &onSwitches, 0, 450, "t", "nomonitor");
	//       FDBG("actpatopt " + SN(actpatopt) + " " + SN(actpattern));
	//        midiopt = webgui.addOptions("Note", 127, midiNamesLong, &onOptionSelect, 0, 650, triggerNote[actpattern], "title");
//        actgrpopt = webgui.addOptions("Goto", 4, pcount, &onOptionSelect, 0, 480, 0, "title");
 //       inleds[3] = webgui.addStringDisplay("MIDI IN", 0, 350, "title", "monframe");
	sipm = webgui.addButtons("TRANSPORT", 6, btnss, &onButtonRelease, 0, 150, "title");
	for (int i = 0; i < MAXMOVE; i++)
	{
		movenext[i] = webgui.addButtons(moveBtn[i], &onButtonRelease, mx[i], my[i], "f");  //  set Block
	}
	bpmret = webgui.addNumericDisplay("BPM",0, 300, "f", "nomonitor");
	webgui.setMonitor(bpmret,BPM);
	bpmguiid = webgui.addInputAnalog("BPM", 60, 240,BPM, &onSlider, 0, 300, "title", "hor");

	drumopt = webgui.addOptions("instr", 128, perc,  &onOptionSelect, 10, 910, 36, "f");

	sdlidt = webgui.addStringDisplay("Pattern #", 220, 60, "f");
	patternidt = webgui.addStringDisplay("Term", 400, 480, "f");
	ccpatternidt = webgui.addStringDisplay("Pattern #", 250, 500, "f");
	ccvpatternidt = webgui.addStringDisplay("Pattern #", 550, 500, "f");
	for (int i = 0; i < 4; i++)
	{
		patternidt2[i] = webgui.addStringDisplay("Pattern #", 250, 150 + 80 * i, "f");
		nltidt[i] = webgui.addStringDisplay("", 230, 100 + 80 * i, "f");
	}
	ccidt = webgui.addStringDisplay("", 600, 380, "f");
	keyidt = webgui.addStringDisplay("", 25, 730, "f");
	beatidt = webgui.addStringDisplay("", 25, 750, "f");
	beatnrid = webgui.addStringDisplay("", 230, 750, "f");
	beatline = webgui.addStringDisplay("", 230, 750, "f");
	voiceline = webgui.addStringDisplay("", 230, 750, "f");
	progidt = webgui.addStringDisplay("", 230, 770, "f");
	String progress = "<meter id=\"file\" value=\"" + String(0) + "\"> 32% </meter>";
	webgui.setMonitor(progidt, progress);

	for (int v = 0; v < 4; v++)
	{
		int ypos = 550 + (v) * 50;
		voiceidt[v] = webgui.addStringDisplay("", 250, ypos, "f");
		for (int i = 0; i < 2; i++)
		{
			patidt[v][i] = webgui.addStringDisplay("Rhythm", 1050 + i * 150, ypos - 10, "f");
			//                FDBG(SN(v) + SN(i) + SN(patidt[v][i]));
		} //       FDBG("ap " + SN(ap) + " " + SN(beatlength * numq) + " " + SN(beatlength) + " " + SN(numq));
	}
	;
	update_pat(true);


}
void webloop()
{
	static int mstate = -1;
	static int oldsb = -1;
	static juce::String ooled = "";

	juce::String outmen = "<style>table td  {  font-size : 16px; } </style> ";
	if (!webguiclient.isConnected())
	{
		webgui.connect(&webguiclient, "localhost");
		websetup(myMidi);
	}
	if (!webguiclient.isConnected())
		return;
	//
	//    osb = -1;
	webgui.update();

	webgui.setMonitor(men, outmen);
	if (metison)
	{
		showStatus(patcnt, false);
	}

}
