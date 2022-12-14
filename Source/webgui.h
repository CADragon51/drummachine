#pragma once

#ifndef Webgui_h
#define Webgui_h
#include <JuceHeader.h>
using namespace juce;
#define TCPPORT 8124
#define INPUTSTREAMSIZE 128
#define MAXACTIONS 1000
#define MAXMOUSEDOWNACTIONS 200

#define CHECKCONNECTION(RET) \
	if (!this->connected()){ \
		if(this->_connect()){\
		}\
		else{\
			return RET;\
		}\
	}


#define INTEGER 1
#define FLOAT 2
#define STRING 3
#define BOOLEANS 4


typedef void (*CallbackTypeInt)(int, int);	  // this type is a function that returns void and receives an int.
typedef void (*CallbackTypeFloat)(float, int); //this type is a function that returns void and receives a float.
typedef void (*CallbackTypeCharp)(String, int); //this type is a function that returns void and receives a char*.
typedef void (*CallbackTypeBoolp)(bool*, int); //this type is a function that returns void and receives a bool*.

typedef struct {
	int id;
	unsigned char type; //necessary to distinguish if 12 is a float or an int
	void* fnAction;
}action_generic_t;
typedef struct {
	int id;
	CallbackTypeInt fnActionMousedown;
}action_mousedown_t;

#define __NAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define __CALLER__ String(String(__LINE__) + " " + __NAME__)

class Webgui {
public:
	Webgui();
	//		bool connect(WiFiClient * client, String host);
	bool connect(StreamingSocket* client, String host);
	bool disconnect(StreamingSocket* client, String host);
	bool connected();
	bool Webgui::_disconnect();
	int addButtons(String name, unsigned char numbuttons, String* buttons, CallbackTypeInt fnMouseUp /*click*/, CallbackTypeInt fnMouseDown, int x, int y, String title, String classname = "control");
	int addButtons(String name, unsigned char numbuttons, String* buttons, CallbackTypeInt fnActionClick, int x, int y, String title, String classname = "control");
	int addButtons(String name, CallbackTypeInt fnActionClick, int x, int y, String title, String classname = "control");
	int addSwitches(String name, unsigned char numswitches, bool* switches, CallbackTypeBoolp fnAction, int x, int y, String title, String classname = "control");
	int addOptions(String name, int numoptions, String* options, CallbackTypeInt fnAction, int x, int y, int selection, String title, String classname = "control");
	int addInputAnalog(String name, float minvalue, float maxvalue, float defaultvalue, CallbackTypeFloat fnAction, int x, int y, String title, String classname = "control");
	int addInputString(String name, CallbackTypeCharp fnAction, int x, int y, String title, String classname = "control", String value = "");
	int addLED(String name, int x, int y, String title, String classname = "monitor");
	int addNeedleIndicator(String name, float minvalue, float maxvalue, int x, int y, String title, String classname = "monitor");
	int addNumericDisplay(String name, int x, int y, String title, String classname = "monitor");
	int addStringDisplay(String name, int x, int y, String title, String classname = "monitor");
	void setMonitor(int id, float value);
	void setMonitor(int id, String value);
	void setMonitorT(int id, String value, short type);
	//		void setMonitor(int id, bool value);
	void remove(int id);
	void reset();

	void update();

private:
	//		WiFiClient * _wificlient;
	StreamingSocket* _ethernetclient;
	void _print(String);
	void _println(String);
	void _println();
	String _server;

	int actionsList[MAXACTIONS]; //array of pointers to action_generic_t
	unsigned char numactions;
	action_mousedown_t* mousedownActionsList[MAXMOUSEDOWNACTIONS]; //array of pointers to action_mousedown_t
	unsigned char nummousedownactions;
	void _addAction(int id, unsigned char type, void* fnAction);


	int idcounter;
	int _getId();
	inline String _idtostr(int id) { return String(id); }
	inline int _stridtoint(String id) { return id.getIntValue(); };

	bool _connect();
	void _callAction(int id,String val);
	void _analyzeStream();

	unsigned long _lastConnectionAttemptTime;
	unsigned char _connectconsecutivefails;
	String outString;

};

#endif


