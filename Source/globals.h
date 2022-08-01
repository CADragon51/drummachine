#ifndef GLOBALS_H
#define GLOBALS_H
using namespace juce;

int debug = 0;
bool showflat = true;

#define SN(x) juce::String(x) + " "
#define SB(x) juce::String(x ? "t" : "f")
#define SP(x) juce::String((int)x, HEX)
#define __NAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define __CALLER__ juce::String(juce::String(__LINE__) + " " + __NAME__)
#define F(x) x
juce::String history = "lo";
short voices[128];

juce::String lastCall = "stack";
//#define FSTACK Serial.println(__CALLER__)
//#define  lastCall = __CALLER__

juce::String MIDIinData;
juce::String outData;
float g_xoff = 0.0;
int oldsa = 0;


short pmstart[100];
short pmend[100];
short pmshape[100];
int lastXPos = 0;
short guiid[1400];
short id2para[1400];
short menId[1400];
int maxg = 0;
juce::String perc[127];



#define CLICK 0
#define RELEASE 1
#define CHECKBOX 2
#define SLIDER 3
#define OPTION 4


bool refresh = false;

int jj1 = 0;
int j3 = 0;

juce::String opts[100];

int onmidis[128];
unsigned char bitstream[100000];
bool mapwhite = true;
bool playSeq = 0;
unsigned char  midifile[1000000];

int midiptr = 0;
#define HAS_MORE_BYTES 0x80
MidiFile SMF,SMF2;

class MidiEvent
{
public:
	MidiEvent() {}
	void init(short event, short note, short velocity, short channel)
	{
		_event = event;
		_note = note;
		_velocity = velocity;
		_channel = channel;
		if (starttime == 0)
			starttime = Time::getMillisecondCounterHiRes();
		_time = Time::getMillisecondCounterHiRes() - starttime;
				//		_id = id;
				//		FDBG(show());
	}
	juce::String show()
	{
		return juce::String(SN(_time) + " " + SN(_event) + " " + SN(_note) + " " + SN(_velocity));
	}
	short _event;
	short _note;
	short _velocity;
	short _channel;
	double _time;
	static double starttime;
	//	int _length = 0;
	//	short _id = 0;
	//	short on_id = -1;
};
enum trans
{
	CONVERT,
	PLAYING,
	STOPPED,
	RECORDING,
	REPEAT,
	SHUFFLE,
	PAUSE,

};
double MidiEvent::starttime = 0;
short lastEvent = 0;

short transport = STOPPED;
int nextEvent = 0;
int firstEvent = 0;
short shufflebars[128];
short shufflelength[128];
short shufflebarsidx = 0; 
short shufflelengthidx = 0;
short shufflemax = 0;
bool isShuffle = false;
String shuffleDef = "1 3*2 2*4 5 2*6 8 2*10 6";
MidiEvent sequences[100000];
extern void webloop(void);
extern void playPattern(void);
class  MidiHandler;
class HGTimer : public juce::Timer, public juce::ReferenceCountedObject
{
public:
	HGTimer(int milliSeconds)
	{
		startTimer(milliSeconds);
	}

	using Ptr = juce::ReferenceCountedObjectPtr<HGTimer>;
	~HGTimer()
	{
		//		stopTimer();
	}
	void timerCallback()
	{
		webloop();
	}

private:

};

class DrumTimer : public juce::HighResolutionTimer, public juce::ReferenceCountedObject
{
public:
	DrumTimer(int milliSeconds, MidiHandler* pMidi)
	{
		myMidi = pMidi;
	}

	using Ptr = juce::ReferenceCountedObjectPtr<DrumTimer>;
	void  hiResTimerCallback();
	~DrumTimer()
	{
		//		stopTimer();
	}

	MidiHandler* myMidi;
private:

};
class MidiTimer : public juce::HighResolutionTimer, public juce::ReferenceCountedObject
{
public:
	MidiTimer(int milliSeconds, MidiHandler* pMidi)
	{
		myMidi = pMidi;
	}

	using Ptr = juce::ReferenceCountedObjectPtr<MidiTimer>;
	~MidiTimer()
	{
		//		stopTimer();
	}

	void  hiResTimerCallback();

	MidiHandler* myMidi;
private:

};


DrumTimer::Ptr metTimer;
MidiTimer::Ptr midiTimer;

class MainWindow;
MainWindow* mainwin = 0;
int patnames[500];
juce::String patvals[500];
int maxsamples;
short lastmet[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

short maxrepl = 0;
short actkeyidx = -1;
short onnotes[128];
// short offnotes[128];
short xpos[12], firstnote;


#define MCP4725_ADDR 0x60
bool foundserver = false;
class MidiHandler;
MidiHandler *myMidi = 0;
unsigned char _channel;
int baseNoteID[128];
signed char basenote = 0;
signed char chordptr = 0;
signed char sleepNoW = 0;
int deltat = 0;
int id = 0, bn = 0;
signed char checked = 0;
signed char extSW = 0;
int reso = 1;

bool incoming[3];
float outgoing[7];
short onoff = 0;																													 

short scalebase = 0;
bool scend = false;
int clickc = 0;
bool actpat[2];
#define editMode actpat[0]
#define MAXPAT 256
#define MAXVOI 32
juce::String g_search;
juce::String patfiles[MAXPAT];

short triggerNote[128];
short acttrigger[MAXPAT];
short actbeatID[MAXPAT]; 
short midiActbeatID[1024];
short midiVoiceID[32]; 
short midiVelID[32]; 
short midiNoteID[32];
int np[128];
bool soloPlay = false;
struct noteshow {
	int note;
	int eventnr;
	double start;
	double length;
	int bar;
} anEvent;
Array <noteshow*> events;
int onEvents[128];
int bc2event[1024];
int bc2eventnr[1024];
float delta;
int minc = 1;
int ly = 0;
int bcc = 0;
juce::String patcolors[MAXPAT];
int sa = 0;
int se;
float g_xpos = 0;
short actred = -1;
short startvoice = -1;
bool startOver = false;
#define maxticks 12
int numq = 1;
float metpart = 1;
float mettime = 2000000;
bool metison = false;
bool metisback = false;
short voicestat[32];
short voicenote[32];
float voiceamp[32];
bool selvoice[32] = { false,false,false,false,false,false,false,false, false,false,false,false, false,false,false,false, false,false,false,false, false,false,false,false, false,false,false,false, false,false,false,false};
short patternc = -1, newpatternc = -1;
short patcnt = 0;
int actpattern = 0;
int actMidibeat = -1;
int startpattern = -1;
int lastpattern = -1;
int startMidi = -1;
int lastMidi= -1;
short seqpattern[MAXPAT * maxticks][MAXVOI];
short delaypattern[MAXPAT * maxticks];
short velpattern[MAXPAT * maxticks][MAXVOI];
short zerobase = 1;
short minstr[MAXVOI];
short mvelo[MAXVOI];
int zoomStart, zoomEnd;
juce::File currentDirectory;
juce::File previousDir;
short oldtrigger = 0;
bool seqswitch[maxticks];
int patidt[4][2];
int metopt[4];
int instopt[4];
int voiceidt[4];
int velopt[4];
int metid[4];
extern void showVoiceline(void);
extern void showBeats();
StringArray favdir;
short beatlength = 4;
// juce::String pout = "";
bool beatstate = false;
int keyidt, beatidt, ccidt, nltidt[4],progidt,beatnrid,beatline,voiceline;
PropertySet TMSsettings;
short mbase = 100, ledbase = mbase + 40, sbase = 200 + 35, tbase = sbase + 15, fbase = tbase + 40, xbase = fbase + 20, base5 = xbase + 80, base6 = base5 + 20, base7 = base6 + 30, base8 = base7 + 50, base9 = base8 + 50, base10 = base9 + 50;
int mtarget = 0;
int centerx[10], centery[10];
int ccopt, lastcc = 0;
short ccvalopt = 0, lastccval = 0;
int ccmetid;
short ccpattern[MAXPAT * maxticks];
short ccval[MAXPAT * maxticks];
short patvoicelow[MAXPAT];
short patvoicehigh[MAXPAT];
juce::String counters[128];
juce::String pcount[4] = { "1", "17", "33", "49" };
bool patset = false;
long lastmill = 0;
short lastBeat = 0;
short beatCount[MAXPAT];
short lastvoice = 0;
int patternidt = 0;
int patternidt2[4];
int ccpatternidt = 0;
int ccvpatternidt = 0;

juce::String coloring[6] = {
   "cyan",
   "#66CDAA",
   "#008B8B",
   "#5F9EA0",
   "gold",
   "beige" };
juce::String orgcoloring[6] = {
   "cyan",
   "#66CDAA",
   "#008B8B",
   "#5F9EA0",
   "gold",
   "beige" };
short lastColor = 0;

short inscale[12];
#define byte unsigned char
#define FDBG(x) 
// **************************************************

bool nodisplay = false;
unsigned long lastTime;
unsigned long onTime;
unsigned long myTime;
//	File gfrec;

bool trigger = false;

#include "gfunctions.h"
#include "svgdraw.h"
#endif
