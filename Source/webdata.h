#pragma once
#ifndef Webdata_h
#define Webdata_h
#define TCPPORT 18124
#include "Webgui.h"

Webgui webgui;
juce::StreamingSocket webguiclient;
// initialize an instance of the class
// bool sws[] = {false, false, false, false, true};

bool addedDisplay = false;
short cli;
short cliitem, cdid;
short sliderID[45];
short menuId[20];
short transId[20];
short chordselId[20];
short xtransId[20];
short impid = 0;
short posx = 200;
short inc = 250;
short py = 100;
short smp = posx - 100 + inc;
short selitem = 0;
short bgitem = 0;
short showstatid;
short synthstate = 0;

short idt, idtg, sbp, bp, synthmen[3], men, target, gra, extId[30], chr, ledout, semp, semm, octp, octm, idt1, idt2, showstat, idjx, idjy, cped2, cfsr, csw;
short ipm, isb, iob, iab1, iab2, itb, imb, js2, z1, z2, zone, smen, imbb, sipm, home, openFile, metro, backmetro;
short delsav = 0, selback = 0, prenameinp = 0, predelsave = 0, mapvers = 0, mapcom = 0;
float val = 0;
unsigned int localPort = 6123;
bool forward = true;
bool inimport = false;
juce::File lastMidiFile;

//IPAddress server(192, 168, 6, 2);
juce::String mapres[99];
juce::String btns[2] = { "Save", "Delete" };
juce::String btnsi[2] = { "Select", "Back" };
juce::String version = "M";
juce::String comment;
juce::String blueled = "<div class=\"monitor-boolean blue\"><div class=\"notification active\"><div class=\"hover-effect\"></div><div class=\"metal\"><div class=\"light\"><div class=\"glowing\"><div class=\"glowing-blue\"></div></div></div></div></div></div>";
juce::String whiteled = "<div class=\"monitor-boolean white\"><div class=\"notification active\"><div class=\"hover-effect\"></div><div class=\"metal\"><div class=\"light\"><div class=\"glowing\"><div class=\"glowing-white\"></div></div></div></div></div></div>";
juce::String yellowled = "<div class=\"monitor-boolean yellow\"><div class=\"notification active\"><div class=\"hover-effect\"></div><div class=\"metal\"><div class=\"light\"><div class=\"glowing\"><div class=\"glowing-yellow\"></div></div></div></div></div></div>";
juce::String redled = "<div class=\"monitor-boolean red\"><div class=\"notification active\"><div class=\"hover-effect\"></div><div class=\"metal\"><div class=\"light\"><div class=\"glowing\"><div class=\"glowing-yellow\"></div></div></div></div></div></div>";
juce::String greenled = "<div class=\"monitor-boolean green\"><div class=\"notification active\"><div class=\"hover-effect\"></div><div class=\"metal\"><div class=\"light\"><div class=\"glowing\"><div class=\"glowing-yellow\"></div></div></div></div></div></div>";
juce::String offled = "<div class=\"monitor-boolean off\"><div class=\"notification active\"><div class=\"hover-effect\"></div><div class=\"metal\"><div class=\"light\"><div class=\"glowing\"><div class=\"glowing-yellow\"></div></div></div></div></div></div>";

juce::String o;
juce::String out;
// int metopt[maxvoices];
int actpatid, actgrpopt, midiopt, selopt;

juce::String presave;
juce::String grid;
juce::String help = "ex[port] [seq|ini|prs|map|<file>] <target>~"
"he[lp]~"
"im[port] <file>~"
"lo[ad] <file>~"
"ls [<pattern>]~"
"mo[re] <file>~"
"mv <file1> <file2>~"
"pl[ay] file|seq~"
"pc (program change) <program>~"
"Pr[ogression] <I..>|<i..>|<I7..>|<i7..><IM7..>~"
//              "q[uantize] <num> ~"
"rm <file>~"
"sa[ve] <file> <comment>~"
"sh[ow] seq|map|sca ~"
"tr[anspose] <m-1|m-2|synth|cv1> <0|1>~";
juce::String impFile;
juce::String btns2[] = { "Mode +", "Mode -" };
juce::String btns3[] = { "Semi +", "Semi -" };
juce::String btns4[] = { "Oct +", "Oct -" };
juce::String btns7[] = { "&larr;", "&#127968;", "&#x1F5AB;", "&#x1F941;", "&#x25C4;", ">_", "&#x21AA;" };
juce::String btnss[] = {
	"&#x1F3B9;",
	"&#x25B6;",
	"&#x23F9;",
	"&#x1F534;",
	"&#x1F501;",
//	"&#x1F5AB;" ,
    "&#x1f500;"};
#define MAXMOVE 20
short movenext[MAXMOVE]; //ledpin = 13, inleds[9], outleds[9], inbuts[9], outbuts[9], 
int mx[MAXMOVE] = { 40,100,100,100,100,100,0,30,0,60,0,50,50,0,50,0 ,140,0,50,180};
int my[MAXMOVE] = { 600,500,550,100,450,600,550,550,600,550,780,780,820,820,860,860,100,730,730,100 };
juce::String moveBtn[MAXMOVE] = {
"&#x1F5D1;",//setBlock
"&#x23EA;",//next
"&#x23E9;",//prev
"&#x21AA;",//insert
"&#x25B2;",//voicedwn
"&#x25BC;",//markleft
"&#x2770;",//voiceup
"&#x2771;",//markright
"&#x1F5AB;",//save
"&#x25AE;",//delete
"&#x1F50D;",//zoomout
"&#x1f50e;",//zoomin
"&#x1F507;",//solo
"&#x1F509;",//mute
"&#128077;",//down
"&#128078;",//up
"&#10084;",//favorite
"&#x1F3B9;",//Midi convert
"&#x21AA;",//insert
"&#x1F516;",//add fav
};
int drumopt;
#endif
