#pragma once
int left = 20;
int offset = 12;
int top = 50;
int len = 620;
int idc = 100;
#include "globals.h"
#include "webdata.h"
extern void onButtonClick(int button, int id);
extern void onButtonRelease(int button, int id);
extern void onMessage(juce::String value, int id);
extern void onMessage(juce::String value, int id);
extern void onOptionSelect(int option, int id);
extern void onSlider(float value, int id);
extern void onSwitches(bool* value, int id);
juce::String style = "<style> .small { font: italic 11px sans-serif; color:#ffffff; }  .flat { font-size: 26px ;font-family: 'Music'; color:#ffffff;} .note { font-size: 26px;font-family: 'Music';color:#ffffff; }"
" .clef { font-size : 40px;font-family: 'Music'; color:#ffffff;} .thin{ stroke : rgb(0xff, 0xff, 0xff); stroke-width : 1px;} </style>";

juce::String header = " <svg height=\"300\" width=\"650\">";
juce::String defs = " _defs_ ";
//"<defs> < g transform = \"translate(x,y)\" /><path id = \"mysine\" d = \"M10 80  C 40 10, 65 10, 95 80 S 150 150, 180 80 \" stroke = \"#ffffff\" fill = \"transparent\" /></g></defs> ";
//+ style;
juce::String group = "<g>";
juce::String endgroup = "</g>";
juce::String svgend = "</svg>";
// juce::String transform = "<g     transform=\"";
byte notey[24] = { 0, 1, 1, 2, 2, 3, 4, 4, 5, 5, 6, 6, 7, 8, 8, 9, 9, 10, 11, 11, 12, 12, 13, 13 };
byte noteyS[24] = { 0, 0, 1, 1, 2, 3, 3, 4, 4, 5, 5, 6, 7, 7, 8, 8, 9, 10, 10, 11, 12, 12, 13, 13 };
byte noteflat[12] = { 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0 };
byte lastflat = 0;

juce::String P[12];




juce::String notew = "fill =\"#ffffff\"  class =\"note\"  >w</text>";
juce::String noteb = "fill =\"#ffffff\"  class =\"flat\"  >b</text>";
juce::String ctext = "fill =\"#ffffff\"  class =\"small\" >";
juce::String tx = "<text X=\"";
juce::String ty = "\"Y =\"";
juce::String ta = " text-anchor = \"middle\" ";
String showLine(float x1, float y1, float x2, float y2, int width = 1, String color="#ffffff")
{
	String x1P = "M " + String(x1) + " " + String(y1);
	String x2P = " l " + String(x2 - x1) + " " + String(y2 - y1);
	String ret = "<path d=\"" + x1P + x2P + "\" stroke-width=\"" + String(width) + " \" fill = \"none\" stroke=\"" + color + "\" /> ";
	//    DBG(ret);
	String swnw = "stroke-width=\"1 \" fill = \"none\" stroke=\""+color+"\"";
	return ret.replace(swnw, "_w@");
}
juce::String showText(juce::String text, int x, int y, juce::String tclass = "", juce::String fill = "#ffffff")
{
	juce::String ret = "_x@" + juce::String(x) + "_y@" + juce::String(y) + "\" ";
	if (tclass.indexOf("small") > -1)
		ret += "_c@" + text + "</text>";
	else if (tclass.indexOf("tiny") > -1)
		ret += "_t@" + text + "</text>";
	else if (tclass.indexOf("teeny") > -1)
		ret += "_u@" + text + "</text>";
	else
		ret += " text-anchor = \" middle \"  fill =\"" + fill + "\"   >" + text + "</text>";
	return ret;
}
int second = 0;

juce::String ltot[] = { "3t", "3s", "s", "3e", "s.", "e", "3q", "3q", "e.", "e.", "q", "q" };
juce::String erg[maxticks * 4];
static juce::String rests[16] = { "", ".", "..", "&#119103;",
						   "&#119103;.", "&#119103;..", "&#119102;", "&#119102;.",
						   "&#119102;..", "&#119102;&#119103;", "&#119102;&#119103;.", "%&#119103;..",
						   "&#119101;", "&#119100;", "&#119100;.", "&#119099;" };

juce::String pat2string(int actpat, int v)
{
	int pat = actpat;
	juce::String pzero = "";
	int f1 = 48;
	bool empty = true;
	;
	if (v > -1)
	{
		//        int pos = (actpattern / 4 + actpat) * 4 * maxticks;
		int pos = actpat * maxticks;
		//       FDBG("pos " + SN(pos) + " " + SN(actpattern) + " "+ SN(actpat));
		juce::String sv = "1";
		for (int i = 0; i < maxticks * 2; i++)
		{
			short val = seqpattern[i + pos][v];
//			if(val>-1&&v==0)
//			DBG("v " << v << " pos " << i + pos << " vel " << val);
			if (val > 0)
			{
				//              FDBG("val " + SN(val) + SN(i + pos) + SN(v));
				empty = false;
				sv = "1";
				if (i == 0)
					f1 = 0;
				else if (f1 == 48)
					f1 = i;
				//               FDBG("f1 " + SN(f1));
			}
			else
				sv = "0";
			pzero += sv;
		}
		//      FDBG(SN(actpat) + pzero);
	}
	else
	{
		for (int i = 0; i < maxticks; i++)
		{
			bool pt = (pat & (1 << i)) > 0;
			if (i == 0 && pt)
				f1 = 0;
			if (pt)
			{
				pzero += "1";
				if (f1 == 48 && i)
					f1 = i;
				empty = false;
			}
			else
				pzero += "0";
		}
		//       FDBG(pzero);
	}
	if (empty)
		return rests[15] + "&nbsp;";
	int p = 0;
	StringArray erg;
	erg.addTokens(pzero, "1", "\"");
//	DBG(minstr[v] << " " << pzero);
	p = erg.size();

	//    FDBG(p);
	juce::String rs = "";
	;
	for (int i = 1; i < p; i++)
	{
		byte nl = erg[i].length();
		//       FDBG(SN(i) + " notelen " + SN(nl));
		if (nl < 12)
		{
			rs += ltot[nl];
			//          FDBG(rs + " notelen " + SN(nl));
		}
		else if (nl <= 17)
		{
			rs += "q.";
		}
		else if (nl <= 23)
		{
			rs += "h";
		}
		else if (nl < 47)
		{
			rs += "h.";
		}
		else
		{
			rs += "w";
		}
		//        FDBG(rs + " notelen " + SN(nl));
	}
	if (f1 > 12)
	{
		if (f1 == 24)
			f1 = 13;
		if (f1 == 36)
			f1 = 14;
		else
			f1 = 15;
	}
	if (rs.length() > 0)
		rs.replace("3s3s3s", "3sss").replace("3e3e3e", "3eee").replace("3ssse", "3sss e");

	//    FDBG(pzero+" rest " + SN(f1));
	if (f1 > 0)
		rs = rests[f1] + "&nbsp;" + rs;
	//    FDBG("pat 2 string "+SN(pat) + " " + pzero + " " + SN(f1) + " " + rs);
	return rs;
}
byte checkKey(int p)
{
	byte ret = acttrigger[p] > -1 ? acttrigger[p] : 0;
	//    if (ret > 0)
	//        FDBG("key @" + SN(p) + SN(ret));
	return ret;
}
#define patstate(x) beatCount[x] == 0 ? "&#x25AF;" : "&#x25AE;"
#define actstate(x) beatCount[x] == 0 ? "&#x25C7;" : "&#x25C6;"
//#define patkey(x) checkkey(x) ? "&#x251C;" : "&#x250B;"
#define patkey(x) "&#x251C;"
juce::String openblock[2] = { "&#x2591; ", "&#x2588; " };
juce::String markleft[2] = { "&#x25B7;", "&#x25B6;" };
juce::String markright[2] = { "&#x25C1;", "&#x25C0;" };
short kc = 0, bc = 0, oc = 0;

juce::String oblocks(int i, int b)
{

	juce::String mb;
	//    FDBG(SN(i) + SN(startpattern) + SN(lastpattern));
	if (i == startpattern)
		mb = markleft[b];
	else if (i == lastpattern)
		mb = markright[b];
	else
		mb = openblock[b];
	if (b)
		bc++;
	else
		oc++;
	return mb;
}
float charsize[8] = { 10, 8.345, 5.83, 4.5, 3.66, 3.1, 2.675, 2.36 };
juce::String showFiles(int sp, float xpos, int& f)
{

	juce::String nlt;

	if (patfiles[sp] != "")
	{
		juce::String fn = patfiles[sp];
		fn = fn.replace("_", " ").replace(".mid", "");
		int ix = fn.lastIndexOf("\\");
		if (ix > -1)
			nlt = showText(fn.substring(ix + 1), xpos, 10 + (f++ % 2) * 13, "teeny");
		else
			nlt = showText(fn, xpos, 10 + (f++ % 2) * 13, "teeny");
		//        FDBG(SN(sp) + patfiles[sp] +" "+ SN(xpos) + SN(f));
	}
	return nlt;
}

const juce::String actblock[2] = { "&#x25BD;", "&#x25BC;" };
const juce::String markact[2] = { "&#x25C7;", "&#x25C6;" };
const juce::String header1600 = " <svg height=\"50\" width=\"1600\">";
extern juce::String outString;
char outbuff[100000];
char dirbuff[100000];
void showStatus(int pos = actpattern, bool refresh = false)
{

	for (int g = 0; g < 4; g++)
	{
//		int g = actpattern / 64;
		sa = g * 64;
		outString = F("<span style =\"color:") + coloring[5] + F(";font-size:20px;\">");
		juce::String markblocks = F("<text style =\"color:") + coloring[4] + F(";font-size:20px;\">");
		juce::String key = " |";
		juce::String nlt = header1600;
		static int oldmid = -1;
		//    bool isOver = false;
		//  int lastbeat = 0;
		float xpos = 0;
		int ppos = pos;
		kc = 0, bc = 0, oc = 0;

		se = sa + 64;
		bool wasBlock = false;
		int f = 0;

		juce::String ocol = "";

		for (int sp = sa; sp < se; sp++)
		{
			//       int i = sp % 64;
			bool isBeat = (sp % beatlength) == 0;
			int b = beatCount[sp] == 0 ? 0 : 1;
			if (beatlength == 1)
				isBeat = false;
			if (isBeat)
			{
				xpos = (oc) * (23.3 - 6) + bc * (17.75 - 0.3) + kc * (14.5)+50;
//				if (refresh)
					nlt += showFiles(sp, xpos-30, f);
				if (actbeatID[sp] == -1)
				{
					int add = -2;
					int butnr = sp / beatlength + zerobase;
					juce::String bn = juce::String(butnr);
					add = 193;
					if (butnr > 9)
						add = 188;
					actbeatID[sp] = webgui.addButtons(bn, &onButtonRelease, add + xpos, 130 + 80 * g, "f", "hide");
				}

			}
			if (patcolors[sp] == "" && sp > 0)
				patcolors[sp] = patcolors[sp - 1];

			if (b && (!wasBlock || ocol != patcolors[sp]))
			{
				outString += F("</text><text style =\"color:") +
					patcolors[sp] + F(";\">");
				wasBlock = true;
				ocol = patcolors[sp];
				//            if (patcolors[sp] == "")
				 //               FDBG(sp);
			}
			if (!b && wasBlock)
			{
				//           FDBG(SN(i)+SN(g)+patcolors[i][g]);
				outString += F("</text><text style =\"color:") + coloring[5] + F(";\">");
				wasBlock = false;
			}
			//      
			if (sp >= actpattern && sp < ppos)
			{
				if (isBeat)
				{
					//                 FDBG(SN(i) + SN(markblocks.length()));
					markblocks += key;
					kc++;
				}
				markblocks += oblocks(sp, b);
				continue;
			}
			//       

			if (sp == ppos)
			{
				if (isBeat)
				{
					//               FDBG(SN(i) + SN(markblocks.length()));
					markblocks += key;
					kc++;
				}

				markblocks += actblock[b] + F("</text>");
				if (!b)
					oc++;
				else
					bc++;
				outString += markblocks;
				continue;
			}
			if (isBeat)
			{
				//           FDBG(SN(i)+SN(outblocks.length()));
				outString += key;
				kc++;
			}

			outString += oblocks(sp, b);
			//        FDBG(outblocks);
		}
		if(ppos>=(g+1)*64)
			outString += markblocks;
		outString += F("</text></span>");
		webgui.setMonitor(patternidt2[g], outString);
		if (!metison )
			webgui.setMonitor(nltidt[g], nlt + svgend);
	}
}
juce::String showRhythm(juce::String what, int id)
{

	juce::String mettext = "<svg width=\"60mm\" height=\"80mm\" id=\"" + juce::String(id) + "\">" +
		"<text><tspan style=\" font-size:16px;" +
		"font-family:'Metric';stroke-width:0.5px;fill:#f6ffd5\" x=\"0\" y=\"30\">" +
		what + "</tspan></text></svg>";
	//   FDBG(mettext);
	return mettext;
}
void showCCs(void)
{
	if (metison)
		return;
	juce::String header = " <svg height=\"40\" width=\"1000\">";
	juce::String nl = header;
	for (int i = actpattern * maxticks, j = 0; i < (actpattern + 1) * maxticks; i++, j++)
	{
		if (ccpattern[i] > 127 || ccpattern[i] == 0)
			continue;
		juce::String out = juce::String(ccpattern[i]) + "/" + juce::String(ccval[i]);
		//       FDBG(SN(i) + " " + mid);
		nl += showText(out, 80 + j * 25.5, 15, "tiny");
	}
	//   FDBG("show CC " + SN(nl.length()));
	webgui.setMonitor(ccidt, nl + svgend);
	//    FDBG(nl + svgend);
}



extern int centerx[10], centery[10];

extern short xbase;

