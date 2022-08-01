bool selection = false;

using namespace juce;
void saveTMS(void);
juce::String cdstack[20];
int cdptr = 0;

juce::String readline(juce::File file)
{
	juce::String ret = "";
	char ch;
	//   while (file.)
	{
		//        ch = file.read();
		if (ch == '\n' || ch == '\r')
		{
			return ret;
		}
		ret += ch;
	}
	return ret;
}
#define SN(x) String(x) + " "
byte getVoice(byte note)
{
	//   FDBG("note " + SN(note) + " " + voices[note]);
	if (voices[note] == -1)
	{
		voices[note] = lastvoice++;
		if (lastvoice >= MAXVOI)
			lastvoice = 128;
		minstr[voices[note]] = note;
	}
//	DBG(SN(note) << SN(voices[note]) << SN(minstr[voices[note]]));
	return voices[note];
}

juce::String saveDrum(juce::String file, bool show = false)
{
	juce::String ret = "";
	char sep = ',';
	File frec = File(File::getSpecialLocation(File::SpecialLocationType::userDocumentsDirectory).getFullPathName() +
		File::getSeparatorString() + "TMS" + File::getSeparatorString() + file + ".drm");

	if (!show)
	{
		FDBG("saving drums to " + file + ".drm");
		juce::String pname = file + ".drm";
		juce::String bname = "bak_" + file + ".drm";
		frec.deleteFile();
		frec.create();
	}

	//   FDBG("showing drums" + file + ".drm");
	juce::String line = "";
	for (int v = 0; v < MAXVOI; v++)
	{
		if (minstr[v] == 0)
			break;
		line = SN(minstr[v]);
		for (int p = 0; p < maxticks * MAXPAT; p++)
		{
			if (seqpattern[p][v] != -1)
				line += SN(p) + SN(delaypattern[p]) + SN(seqpattern[p][v]);
		}
		//            FDBG(line);

		if (line.length() > 10)
		{
			if (show)
				ret += line + "~";
			else
				frec.appendText(line + "\n");
		}
	}
	line = "K ";
	for (int i = 0; i < 128; i++)
	{
		//          FDBG(SN(triggerNote[i]) + SN(i)+SN(acttrigger[i]));
		if (triggerNote[i] > 0)
			line += SN(triggerNote[i]) + SN(i);
	}
	if (show)
		ret += line + "~";
	else
		frec.appendText(line + "\n");
	line = "B ";
	for (int i = 0; i < 6; i++)
	{
		//          FDBG(SN(triggerNote[i]) + SN(i)+SN(acttrigger[i]));
		line += coloring[i] + " ";
	}
	if (show)
		ret += line + "~";
	else
		frec.appendText(line + "\n");

	line = "C ";
	for (int p = 0; p < maxticks * MAXPAT; p++)
	{
		if (ccpattern[p] != 0 && ccpattern[p] < 128)
			line += SN(p) + SN(ccpattern[p]) + SN(ccval[p]);
	}
	//            FDBG(line);
	if (show)
		ret += line + "~";
	else
		frec.appendText(line + "\n");

	line = "P ";
	int bc = 0;
	for (int p = 0; p < MAXPAT; p++)
	{
		if (beatCount[p] <= bc)
		{
			bc = beatCount[p];
			continue;
		}
		line += juce::String(p) + " " + juce::String(beatCount[p]) + " ";
		bc = beatCount[p];
		//           keyl += juce::String(triggerNote[p]) + " ";
	}
	if (show)
		ret += line + "~";
	else
		frec.appendText(line + "\n");
	line = "F ";
	for (int p = 0; p < MAXPAT; p++)
	{
		if (patfiles[p].length() > 0)
			line += juce::String(p) + sep + patfiles[p] + sep;
		//               FDBG(SN(p)+line);
	}
	if (show)
		ret += line + "~";
	else
		frec.appendText(line + "\n");

	//   FDBG(ret);
	return ret;
}
short ires[2560];
int voice = 0;
void cleanpat(int l)
{
	int from = actpattern;
	int to = from + l;
	for (int p = from; p < to; p++)
	{
		////        FDBG(SN(p) + SN(beatCount[p]));
		patvoicehigh[p] = -1;
		patvoicelow[p] = 255;
		for (int s = p * maxticks; s < (p + 1) * maxticks; s++)
		{
			delaypattern[s] = 0;
			for (int v = 0; v < MAXVOI; v++)
			{
				seqpattern[s][v] = -1;
			}
		}
		beatCount[p] = 0;
		patfiles[p] = "";
		//           acttrigger[p] = -1;
	}
	//   FDBG(beatCount[to]);
	DBG("cleaned from " << SN(from) << " to "<<SN(to));
}
juce::String cline = "";
void file2Drum(int cc)
{
	short note = ires[0];
	if (note == 0)
		return;
	//   FDBG(__CALLER__);
	int v = 0;
	//    FDBG("pos " + SN(ires[1]));
	v = getVoice(note);
	if (v >= MAXVOI)
		return;
	//   int pos = actpattern + ires[1] / maxticks;
	//   int pos = ires[1] / maxticks;
	//   FDBG(SN(v) + SN(grp) + SN(pos) + " from " + SN(patvoicelow[pos][grp]) + " to " + SN(patvoicehigh[pos][grp]));

	for (int p = 1; p < cc - 1; p += 3)
	{
		short ipos = ires[p];
		short idel = ires[p + 1];
		short vel = ires[p + 2];
		short as = actpattern * maxticks + ipos;
		//       int pos = as / maxticks;
		seqpattern[as][v] = vel;
		delaypattern[as] = idel;
		//        Serial.println(SN(v) + " " + SN(note) + " " + SN(as) + " " + SN(seqpattern[as][v]) + " ");
		if (mvelo[v] < vel)
			mvelo[v] = vel;
		int pos = ires[p] / maxticks;
		if (patvoicelow[pos] > v)
			patvoicelow[pos] = v;
		if (patvoicehigh[pos] < v)
			patvoicehigh[pos] = v;
		//       FDBG(SN(v) + SN(grp) + SN(pos) + " from " + SN(patvoicelow[pos][grp]) + " to " + SN(patvoicehigh[pos][grp]));
	}

	//   Serial.println();
}

void file2File(int cl)
{
	StringArray sres;
	sres.addTokens(cline, ",", "\"");
	for (int p = 0; p < sres.size() - 1; p += 2)
	{
		short pos = sres[p].getIntValue();
		patfiles[pos] = sres[p + 1];
		int ix = sres[p + 1].lastIndexOf(".mid ");
		if (ix > -1)
		{
			int np = patfiles[pos].substring(ix + 4).getIntValue();
			// patcolor[pos] = cyan[c++];
			// FDBG(SN(p) + SN(pos) + patfiles[pos] + SN(np));
			for (int pp = pos; pp < pos + np; pp++)
				patcolors[pp] = coloring[lastColor % 4];
			// c++;
			//          FDBG(SN(np) + SN(p) + SN(pos) + patcolors[pos]);
			//      FDBG("LC " + SN(lastColor));
			lastColor++;
		}
	}
	//   Serial.println();
	return;
}
void file2CC(int cl)
{
	int p =1;
	while (p < cl&&cl>3)
	{
		short pos = ires[p++];
		short cc = ires[p++];
		short val = ires[p++];
		short as = actpattern * maxticks + pos;
		ccpattern[as] = cc;
		ccval[as] = val;
		//       Serial.println(SN(p)+"CC " + SN(pos) + SN(cc) + SN(val));
		//       Serial.println("CC " + SN(as) + SN(ccpattern[as]) + SN(ccval[as]));
	}
	//   Serial.println();
	return;
}
void file2Color(int cl)
{
	int p = 1;
	StringArray sres;
	sres.addTokens(cline, " ", "\"");
	while (p < sres.size() && p < 6)
	{
		coloring[p-1] = sres[p];
		p++;
	}
	//   Serial.println();
	return;
}
void file2Key(int cl)
{
	int p = 1;
	while (p < cl&&cl>4)
	{
		short act = ires[p++];
		short note = ires[p++];

		triggerNote[note] = act;
		acttrigger[act - 1] = note;
		//        FDBG("key " + SN(act - 1) + SN(note));
		//       Serial.println("CC " + SN(as) + SN(ccpattern[as]) + SN(ccval[as]));
	}
	//   Serial.println();
	return;
}
void file2Pat(int cl)
{
	int p = 1;
	while (p < cl-1)
	{
		short sp = ires[p++];
		short bc = ires[p++];
		//        FDBG(SN(p) + SN(bc));
		while (bc > 0)
		{
			beatCount[sp++] = bc--;
		}
	}
}
int midi2Line(String pline)
{

	StringArray rr;
	rr.addTokens(pline, " ", "\"");
	for (unsigned int i = 0; i < rr.size(); i++)
	{
		ires[i] = rr[i].getIntValue();
	}
	return  rr.size();
}
void loadDrum(String file)
{
	File frec = File(File::getSpecialLocation(File::SpecialLocationType::userDocumentsDirectory).getFullPathName() +
		File::getSeparatorString() + "TMS" + File::getSeparatorString() + file + ".drm");
	if (file.contains(".drm"))
		frec = File(file);
	if (!frec.existsAsFile())
		return;

	voice = -1;
	String fileText = frec.loadFileAsString();
	StringArray dline;
	dline.addTokens(fileText, "\n", "\"");
	int cc = 0;
	ires[cc] = 0;
	for (int i = 0; i < dline.size(); i++)
	{
		cline = dline[i];
		char c = cline[0];
		cc = midi2Line(cline);
		if (c == 'K')
			file2Key(cc);
		if (c == 'B')
			file2Color(cc);
		if (c == 'P')
			file2Pat(cc);
		else if (c == 'F')
			file2File(cc);
		else if (c == 'C')
			file2CC(cc);
		else if (cc > 2)
			file2Drum(cc);
	}

}



long readlong(byte* fp)
{
	unsigned char c[4];

	memcpy((char*)c, fp, 4);
	return (long)((c[0] << 24) | (c[1] << 16) | (c[2] << 8) | c[3]);
}

/*  READSHORT  --  Read short from a file (byte-order independent)  */

short readshort(byte* fp)
{
	unsigned char c[2];

	memcpy((char*)c, fp, 2);
	return (short)((c[0] << 8) | c[1]);
}

/*  READVARLEN	--  Parse variable length value from MIDI file	*/

long readVarLen(byte* fp)
{
	long value;
	int ch;

	if ((value = *fp) & 0x80)
	{
		value &= 0x7F;
		do
		{
			value = (value << 7) | ((ch = *fp++) & 0x7F);
		} while (ch & 0x80);
	}
	return value;
}

/*  High level input functions.  */

/*  READMIDIFILEHEADER	--  Read file header structure.  */
/*  MIDI command codes  */

typedef enum
{

	/* Channel voice messages */

	NoteOff = 0x80,
	NoteOn = 0x90,
	PolyphonicKeyPressure = 0xA0,
	ControlChange = 0xB0,
	ProgramChange = 0xC0,
	ChannelPressure = 0xD0,
	PitchBend = 0xE0,

	/* Channel mode messages */

	ChannelMode = 0xB8,

	/* System messages */

	SystemExclusive = 0xF0,
	SystemCommon = 0xF0,
	SystemExclusivePacket = 0xF7,
	SystemRealTime = 0xF8,
	SystemStartCurrentSequence = 0xFA,
	SystemContinueCurrentSequence = 0xFB,
	SystemStop = 0xFC,

	/* MIDI file-only messages */

	FileMetaEvent = 0xFF
} midi_command;

/*  MIDI file meta-event codes  */

typedef enum
{
	SequenceNumberMetaEvent = 0,
	TextMetaEvent = 1,
	CopyrightMetaEvent = 2,
	TrackTitleMetaEvent = 3,
	TrackInstrumentNameMetaEvent = 4,
	LyricMetaEvent = 5,
	MarkerMetaEvent = 6,
	CuePointMetaEvent = 7,

	ChannelPrefixMetaEvent = 0x20,
	PortMetaEvent = 0x21,
	EndTrackMetaEvent = 0x2F,

	SetTempoMetaEvent = 0x51,
	SMPTEOffsetMetaEvent = 0x54,
	TimeSignatureMetaEvent = 0x58,
	KeySignatureMetaEvent = 0x59,

	SequencerSpecificMetaEvent = 0x7F
} midifile_meta_event;

struct mhead
{
	char chunktype[4]; /* Chunk type: "MThd" */
	long length;       /* Length: 6 */
	short format;      /* juce::File format */
	short ntrks;       /* Number of tracks in file */
	short division;    /* Time division */
};

/*  MIDI track header  */

#define MIDI_Track_Sentinel "MTrk"

struct mtrack
{
	char chunktype[4]; /* Chunk type: "MTrk" */
	long length;       /* Length of track */
};

byte* readMidiFileHeader(byte* fp, struct mhead* h)
{
	memcpy((char*)h->chunktype, fp, sizeof h->chunktype);
	fp += sizeof h->chunktype;
	h->length = readlong(fp);
	fp += 4;
	h->format = readshort(fp);
	fp += 2;
	h->ntrks = readshort(fp);
	fp += 2;
	h->division = readshort(fp);
	fp += 2;
	return fp;
}

/*  READMIDITRACKHEADER  --  Read track header structure.  */

byte* readMidiTrackHeader(byte* fp, struct mtrack* t)
{
	memcpy((char*)t->chunktype, fp, sizeof t->chunktype);
	fp += sizeof t->chunktype;
	t->length = readlong(fp);
	fp += 4;
	return fp;
}
long vlength(byte* trk, int& x)
{
	long value;
	byte ch;
	if ((value = trk[x++]) & 0x80)
	{
		value &= 0x7F;
		//       FDBG(" x " + SN(x) + " " + SN(value));
		do
		{
			ch = trk[x];
			value = (value << 7) | (ch & 0x7F);
			//           FDBG(" x " + SN(x) + " " + SN(ch));
			x++;
		} while (ch & 0x80);
	}
	return value;
}

extern void update_pat(bool);
extern juce::File lastMidiFile;
byte posplus[MAXVOI];








void saveAll(juce::File frec)
{

	return;
}

signed char readrec(juce::File frec)
{
	signed char res = 0;
	//   if (frec.available())
	//       res = frec.read();
	return (res);
}
float floatrec(juce::File frec)
{
	float res = 0;
	//   if (frec.available())
	 //      frec.read(&res, 4);
	return (res);
}
int intrec(juce::File frec)
{
	int res = 0;
	//   if (frec.available())
   //        frec.read(&res, 4);
	return (res);
}

void Restore(juce::File frec)
{

	return;
}
void saveTMS(void)
{


}
float fmap(float x, float in_min, float in_max, float out_min, float out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
int imap(int x, int in_min, int in_max, int out_min, int out_max)
{
	if (x < in_min)
		x = in_min;
	if (x > in_max)
		x = in_max;
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void writeVarLen(unsigned long value)
{
	// Start with the first 7 bit block
	unsigned long buffer = value & 0x7f;

	// Then shift in 7 bit blocks with "has-more" bit from the
	// right for as long as `value` has more bits to encode.
	while ((value >>= 7) > 0)
	{
		buffer <<= 8;
		buffer |= HAS_MORE_BYTES;
		buffer |= value & 0x7f;
	}

	// Then unshift bytes one at a time for as long as the has-more bit is high.
	//    Serial.print(midiptr);
	//    Serial.print("\t");

	while (true)
	{
		midifile[midiptr] = (byte)(buffer & 0xff);
		//       Serial.print(midifile[midiptr], HEX);
		//       Serial.print(" ");
		midiptr++;
		if (buffer & HAS_MORE_BYTES)
		{
			buffer >>= 8;
		}
		else
		{
			break;
		}
	}
	//   Serial.print(":\t");
}
union intbyte
{
	int i;
	byte b[4];
};

void setTrackLength(int length)
{
	//    Serial.print("\n");
	//    Serial.print(midiptr);
	//    Serial.print("\t");
	intbyte x;
	x.i = length - 4;
	midifile[midiptr++] = 0x00;
	midifile[midiptr++] = 0xff;
	midifile[midiptr++] = 0x2f;
	midifile[midiptr++] = 0x00;
	//    Serial.print(" 00 0f 2f 00");
	//   memcpy(midifile + 18, x.b, 4);

	midifile[18] = x.b[3];
	midifile[19] = x.b[2];
	midifile[20] = x.b[1];
	midifile[21] = x.b[0];
}
void createMidiFile()
{
	midiptr = 0;
	byte header[] = {
		0x4D, 0x54, 0x68, 0x64, // "MThd" chunk
		0x00, 0x00, 0x00, 0x06, // chunk length (from this point on): 6 bytes
		0x00, 0x00,             // format: 0
		0x00, 0x01,             // number of tracks: 1
		0x01, 0xC2              // data rate: 450 ticks per quaver/quarter note
	};
	memcpy(midifile + midiptr, header, 14);
	midiptr += 14;
	byte track[] = {
		0x4D, 0x54, 0x72, 0x6B, // "MTrk" chunk
		0x00, 0x00, 0x00, 0x00  // chunk length placeholder
	};
	memcpy(midifile + midiptr, track, 8);
	midiptr += 8;
	byte tempo[] = {
		0x00,             // time delta for the first MIDI event: zero
		0xFF, 0x51, 0x03, // MIDI event type: "tempo" instruction
		0x06, 0xDD, 0xD0  // tempo value: 450,000μs per quaver/quarter note
	};
	//       Serial.print("\n");
	//       Serial.print(midiptr);
	//      Serial.print("\t");
	//       Serial.print("00 FF 51 03 06 DD D0\n");
	memcpy(midifile + midiptr, tempo, 7);
	midiptr += 7;
}
byte laste = 0;
void writeToFile(byte eventType, byte b1, byte b2, int delta)
{

	writeVarLen(delta);
	// FDBG(SN(laste) + " " + SN(eventType));
	if (laste != eventType)
	{
		midifile[midiptr] = eventType;
		//       Serial.print(midifile[midiptr], HEX);
		midiptr++;
	}
	laste = eventType;
	midifile[midiptr] = b1;
	//   Serial.print("\t");
	//   Serial.print(midifile[midiptr], HEX);
	//   Serial.print(" ");
	midiptr++;
	midifile[midiptr] = b2;
	//   Serial.print(midifile[midiptr], HEX);
	//   Serial.print(" ");
	midiptr++;
}



String dirout = "";
void listDirectory(File dir, String search)
{
	bool isS = search.length() > 1;
	//  FDBG(currentDirectory);
	String ldir = dir.getFileName();
	if (isS)
	{
		search.replace(".", "[.]");
		search.replace("*", ".*");
		int x = search.lastIndexOf("/");
		if (x > 0)
		{
			ldir = search.substring(0, x);
			search = search.substring(x + 1);
			File path(ldir);
			dir = path;
		}
		//    FDBG("looking for " + search + " in " + ldir);
	}
	dirout = "&#x1f5c2;" + ldir + "~";

	bool d = dir.isDirectory();
	int n = dir.getNumberOfChildFiles(3);
	if (n)
	{
		Array<File> entry = dir.findChildFiles(3, false);
		for (File f : entry)
		{
			if (f.isDirectory())
			{
				dirout += "&emsp;&#9507;&#x1F4C1;" + f.getFileName() + "~";

			}
			else
			{
				//    FDBG(sr + " " + SN(sr.length()) + " " + fn);
				String size = String(f.getSize());
				if (f.getFileExtension() == ".mid")
				{
					//                  int v = 0;
									  //       FDBG(ldir + "/" + entry.name());
				   //                   int bc = sizeMIDI(ldir + "/" + entry.name(), v);
					 //                 size = String(bc) + "/" + String(v);
				}
				dirout += "&emsp;&#9507;" + f.getFileName() + " " + size + "~";

			}
		}
	}
	dirout += "~";
}
File res[1000];
String dirres[1000];
class FileComparator {
public:
	static int compareElements(const File& first, const File& second)
	{
		String f1 = first.getFileName();
		String f2 = second.getFileName();
		if (!f1.contains("@") && !f2.contains("@"))
			return f1.compare(f2);
		int i1 = f1.getIntValue();
		int i2 = f2.getIntValue();
		return  i2 > i1;
	}
};

#include "webcb.h"