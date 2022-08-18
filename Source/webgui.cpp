#include "Webgui.h"
using namespace juce;
#define SN(x) String(x)
 int colidx;
 int fq;
 int eq;
 int commas[20];
 int commidx;
 int bitidx=0;
 char bitstream[1000000];
action_generic_t act[1000];
String lastout[1000];
// define GUISTACK //DBG(__LINE__)
#define GUISTACK
Webgui::Webgui()
{
	numactions = 0;
	idcounter = 1;
	_lastConnectionAttemptTime = 0;
	_connectconsecutivefails = 0;
	_ethernetclient = NULL;
}

int Webgui::_getId()
{
	return idcounter++;
}

bool Webgui::_connect()
{
	return this->connect(_ethernetclient, _server);
}
bool Webgui::_disconnect()
{
	return this->disconnect(_ethernetclient, _server);
}

void Webgui::_print(String str)
{
	_ethernetclient->write(str.getCharPointer(),str.length());
}

void Webgui::_println(String str)
{
	_print(str+"\n");
}

void Webgui::_println()
{
	_ethernetclient->write("\n",1);
}

bool Webgui::connect(StreamingSocket* client, String host)
{
	_ethernetclient = client;
	_server = host;
#if 1
	if (Time::getMillisecondCounterHiRes()- _lastConnectionAttemptTime < 100)
	{
		//DBG(("Waiting some time before connecting to webgui server."));
		Time::waitForMillisecondCounter(100);
		return false;
	}
	_lastConnectionAttemptTime = Time::getMillisecondCounterHiRes();
#endif
	int status = (_ethernetclient)->connect(_server, TCPPORT);
	if (status)
	{
		_ethernetclient->write("\n",1); // send some data in order to avoid arduino gets crazy
		_connectconsecutivefails = 0;
		return true;
	}




	return false;
}

bool Webgui::disconnect(StreamingSocket* client, String host)
{
	_ethernetclient = client;
	_server = host;
	(_ethernetclient)->close();
	return false;
}
bool Webgui::connected()
{
	int _res=_ethernetclient->waitUntilReady(true, 1);
	return _ethernetclient->isConnected();
}
void Webgui::update()
{
	if (!this->connected())
	{
		if (!this->_connect())
		{
			return;
		}
	}
//	while (_ethernetclient->waitUntilReady(true,100))
	{
		int n=_ethernetclient->read(bitstream,1000,false);
		for (bitidx = 0; bitidx < n; bitidx++)
		{
			unsigned char c = bitstream[bitidx];
			if (c == ':'&&colidx==0)
				colidx = bitidx;
			if (fq == 0 && c == '`')
				fq = bitidx;
			else if (fq > 0 && c == '`')
				eq = bitidx;
			if (c == ','&&fq==0)
				commas[commidx++] = bitidx;
			if (c == '\n'&&eq>fq)
				bitstream[bitidx] = 0;
		}
		//		Serial.print(c);
	}
	if (!strcmp(bitstream,"ERROR"))
	{
		this->_disconnect();
		bitstream[0] = 0;
		return;
	}
	if (bitidx > 0)
	{
		if (eq + fq == 0 || eq * fq > 0)
		{
			bitstream[bitidx++] = 0;
			bitstream[colidx] = 0;
			//			//DBG();
			//			//DBG(bitstream);
			if (!strncmp(bitstream, "ACTION", 6))
				this->_analyzeStream();
			colidx = 0;
			fq = 0;
			eq = 0;
			commidx = 0;
			bitidx = 0;
		}
	}
}

void Webgui::_analyzeStream()
{
	int lc = colidx + 1;
	String para[20];
	String value;
	for (int i = 0; i < commidx; i++)
	{
		bitstream[commas[i]] = 0;

		//		//DBG(SN(i) + " " + String(bitstream + lc));
		para[i] = String(bitstream + lc);
		lc = commas[i] + 1;
	}

	int sid = para[0].getIntValue();
	////DBG("sid " + SN(sid));
//	if (bitidx < 100)
	{
		if (eq)
			bitstream[eq] = 0;
		if (fq)
			value = String(bitstream + fq + 1);
		else
			value = String(bitstream + commas[0] + 1);
		//		//DBG("val " + value);
	}
//	else
//		value = "###";
	//	//DBG(value);
	this->_callAction(sid,value);
}

void Webgui::_callAction(int id,String value)
{
	int valueInt = value.getIntValue();
	float valueFloat = value.getFloatValue();

	{
		for (int i = 0; i < numactions; ++i)
		{
			if (id == actionsList[i])
			{
				if (act[id].type == INTEGER)
				{
					((CallbackTypeInt)act[id].fnAction)(valueInt, id);
					return;
				}
				else if (act[id].type == FLOAT)
				{
					((CallbackTypeFloat)act[id].fnAction)(valueFloat, id);
					return;
				}
				else if (act[id].type == STRING)
				{
					((CallbackTypeCharp)act[id].fnAction)(value, id);
					return;
				}
				if (act[id].type == BOOLEANS)
				{
					StringArray res;
					res.addTokens(value, "|", "\"");
					int numpipes = res.size();
					bool valueBoolp[30];
					for (int boolindex = 0; boolindex < numpipes; boolindex++)
					{
						valueBoolp[boolindex] = false;
						if (res[boolindex] == "on")
							valueBoolp[boolindex] = true;
					}
					((CallbackTypeBoolp)act[id].fnAction)(valueBoolp, id);
					return;
				}
			}
		}
	}
}

void Webgui::_addAction(int id, unsigned char type, void* fnAction)
{
	if (numactions >= MAXACTIONS)
	{
		//DBG(("There are too many actions. It could not save more actions"));
		return;
	}

	act[id].id = id;
	act[id].type = type;
	act[id].fnAction = fnAction;
	actionsList[numactions % MAXACTIONS] = id;
	numactions++;
}

int Webgui::addButtons(String name, unsigned char numbuttons, String* buttons, CallbackTypeInt fnMouseUp /*click*/, int x, int y, String title, String classname)
{
	String sid;
	int id;
	name += "," + String(x) + "," + String(y) + "," + title + "," + classname;
	if (!numbuttons)
	{
		//DBG("id error @" + String(__LINE__));
		return -1;
	}
	CHECKCONNECTION(-1);
	
	_print("ADD_CONTROL:");
	id = this->_getId();
	sid = _idtostr(id);
	_print(sid);
	_print(",`");
	_print(name);
	_print("`,buttons,");
	for (unsigned char i = 0; i < numbuttons; i++)
	{
		_print("`");
		_print(buttons[i]);
		_print("`");
		if (i != numbuttons - 1)
		{
			_print(",");
		}
	}
	_println();
	_addAction(id, INTEGER, (void*)fnMouseUp);

	// if(id<0)
		//DBG(name + " " + SN(id));
	return id;
}
int Webgui::addButtons(String name, CallbackTypeInt fnMouseUp /*click*/, int x, int y, String title, String classname)
{
	String sid;
	int id;
	String bname = name + "," + String(x) + "," + String(y) + "," + title + "," + classname;
	CHECKCONNECTION(-1);
	
	_print("ADD_CONTROL:");
	id = this->_getId();
	sid = _idtostr(id);
	_print(sid);
	_print(",`");
	_print(bname);
	_print("`,buttons,");
	_print("`");
	_print(name);
	_print("`");
	_println();
	_addAction(id, INTEGER, (void*)fnMouseUp);

	// if(id<0)
		//DBG("id error @" + String(__LINE__));

	return id;
}
int Webgui::addSwitches(String name, unsigned char numswitches, bool* switches, CallbackTypeBoolp fnAction, int x, int y, String title, String classname)
{
	String sid;
	int id;
	name += "," + String(x) + "," + String(y) + "," + title + "," + classname;
	if (!numswitches)
	{
		//DBG("id error @" + String(__LINE__));
		return -1;
	}
	CHECKCONNECTION(-1);
	
	_print("ADD_CONTROL:");
	id = this->_getId();
	sid = _idtostr(id);
	_print(sid);
	_print(",`");
	_print(name);
	_print("`,switches,");
	for (unsigned char i = 0; i < numswitches; i++)
	{
		if (switches[i])
		{
			_print("on");
			//			Serial.print("on");
		}
		else
		{
			_print("off");
			//			Serial.print("off");
		}
		if (i != numswitches - 1)
		{
			_print(",");
			//			Serial.print(",");
		}
	}
	_println();
	//	//DBG(numswitches);

	_addAction(id, BOOLEANS, (void*)fnAction);
	// if(id<0)
		//DBG(name + " " + SN(id));
	return id;
}
int Webgui::addOptions(String name, int numoptions, String* options, CallbackTypeInt fnAction, int x, int y, int sel, String title, String classname)
{
	String sid;
	int id;
	bool deb = false;
	if (name == "instr")
		deb = true;
	name += "," + String(x) + "," + String(y) + "," + title + "," + classname + "," + String(sel);
	if (!numoptions)
	{
		//DBG("id error @" + String(__LINE__) + " " + name + " " + SN(numoptions));
		return -1;
	}
	CHECKCONNECTION(-1);
	
	_print("ADD_CONTROL:");
	id = this->_getId();
	sid = _idtostr(id);
	_print(sid);
	_print(",`");
	_print(name);
	if (!deb)
		_print("`,options,");
	else
		_print("`,options127,");
	//	if (deb)
	//		Serial.print("`,options,");

	for (unsigned char i = 0; i < numoptions && !deb; i++)
	{
		_print("`");
		_print(options[i]);
		_print("`");
		if (i != numoptions - 1)
		{
			_print(",");
		}
	}
	_println();
	_addAction(id, INTEGER, (void*)fnAction);
	return id;
}
int Webgui::addInputAnalog(String name, float minvalue, float maxvalue, float defaultvalue, CallbackTypeFloat fnAction, int x, int y, String title, String classname)
{
	String sid;
	int id;
	if (defaultvalue > maxvalue || defaultvalue < minvalue)
	{
		defaultvalue = minvalue;
		//DBG("id error @" + String(__LINE__) + " " + name);
		//		return -1;
	}
	name += "," + String(x) + "," + String(y) + "," + title + "," + classname;
	CHECKCONNECTION(-1);
	
	_print("ADD_CONTROL:");
	id = this->_getId();
	sid = _idtostr(id);
	_print(sid);
	_print(",`");
	_print(name);
	_print("`,analog,");
	_print(SN(minvalue));
	_print(",");
	_print(SN(maxvalue));
	_print(",");
	_println(SN(defaultvalue));
	_addAction(id, FLOAT, (void*)fnAction);
	// if(id<0)
		//DBG(name + " " + SN(id));
	return id;
}
int Webgui::addInputString(String name, CallbackTypeCharp fnAction, int x, int y, String title, String classname, String pvalue)
{
	String sid;
	int id;
	name += "," + String(x) + "," + String(y) + "," + title + "," + classname + "," + pvalue;
	CHECKCONNECTION(-1);
	
	_print("ADD_CONTROL:");
	id = this->_getId();
	sid = _idtostr(id);
	_print(sid);
	_print(",`");
	_print(name);
	_println("`,string");
	_addAction(id,STRING, (void*)fnAction);
	// if(id<0)
		//DBG("id error @" + String(__LINE__));

	return id;
}
int Webgui::addLED(String name, int x, int y, String title, String classname)
{
	String sid;
	int id;
	name += "," + String(x) + "," + String(y) + "," + title + "," + classname;
	CHECKCONNECTION(-1);
	
	_print("ADD_MONITOR:");
	id = this->_getId();
	sid = _idtostr(id);
	_print(sid);
	_print(",`");
	_print(name);
	_println("`,boolean");
	// if(id<0)
		//DBG("id error @" + String(__LINE__));

	return id;
}
int Webgui::addNeedleIndicator(String name, float minvalue, float maxvalue, int x, int y, String title, String classname)
{
	String sid;
	int id;
	name += "," + String(x) + "," + String(y) + "," + title + "," + classname;
	CHECKCONNECTION(-1);
	
	_print("ADD_MONITOR:");
	id = this->_getId();
	sid = _idtostr(id);
	_print(sid);
	_print(",`");
	_print(name);
	_print("`,analog,");
	_print(SN(minvalue));
	_print(",");
	_println(SN(maxvalue));
	// if(id<0)
		//DBG("id error @" + String(__LINE__));

	return id;
}
int Webgui::addNumericDisplay(String name, int x, int y, String title, String classname)
{
	String sid;
	int id;
	name += "," + String(x) + "," + String(y) + "," + title + "," + classname;
	CHECKCONNECTION(-1);
	
	_print("ADD_MONITOR:");
	id = this->_getId();
	sid = _idtostr(id);
	_print(sid);
	_print(",`");
	_print(name);
	_println("`,digital");
//	// if(id<0)
		//DBG("id error @" + String(__LINE__));

	return id;
}
int Webgui::addStringDisplay(String name, int x, int y, String title, String classname)
{
	String sid;
	int id;
	name += "," + String(x) + "," + String(y) + "," + title + "," + classname;
	CHECKCONNECTION(-1);
	
	_print("ADD_MONITOR:");
	id = this->_getId();
	sid = _idtostr(id);
	_print(sid);
	_print(",`");
	_print(name);
	_println("`,string");
//	// if(id<0)
		//DBG("id error @" + String(__LINE__));

	return id;
}

void Webgui::setMonitor(int id, float value)
{
	CHECKCONNECTION();
	
	_print("SET_MONITOR:");
	String sid;
	sid = _idtostr(id);
	_print(sid);
	_print(",");
	_println(String(value));
}

void Webgui::setMonitor(int id, String value)
{
	// if (lastout[id] == value)
	// {
	// 	//DBG(id);
	// 	return;
	// }
	// lastout[id] = value;
	if (id < 0 || id > idcounter)
	{
		//DBG("invalid ID " + String(id));
		return;
	}
	CHECKCONNECTION();
	
	_print("SET_MONITOR:");
	String sid;
	sid = _idtostr(id);
	_print(sid);
	_print(",`");
	_print(value);
	_println("`");
}
void Webgui::setMonitorT(int id, String value, short type)
{
	// if (lastout[id] == value)
	// {
	// 	//DBG(id);
	// 	return;
	// }
	// lastout[id] = value;
	if (id < 0 || id > idcounter)
	{
		//DBG("invalid ID " + String(id));
		return;
	}
	CHECKCONNECTION();
	
	if (type == 0)
		_print("SET_MONITOR:");
	else
		_print("SET_MONITOR" + String(type) + ":");
	String sid;
	sid = _idtostr(id);
	_print(sid);
	_print(",`");
	_print(value);
	_println("`");
}

// void Webgui::setMonitor(int id, bool value)
// {
// 	CHECKCONNECTION();
// 	
// 	_print("SET_MONITOR:");
// 	String sid;
// 	sid = _idtostr(id);
// 	_print(sid);
// 	_print(",");
// 	if (value)
// 	{
// 		_println("on");
// 	}
// 	else
// 	{
// 		_println("off");
// 	}
// }
void Webgui::remove(int id)
{
	CHECKCONNECTION();
	if (id < 0 || id > idcounter)
		return;
	_print("REMOVE:");
	String sid;
	sid = _idtostr(id);
	if (id > -1)
		_println(sid);
	// delete from action lists
	int i, j;
	for (i = 0; i < numactions; i++)
	{
		if (act[actionsList[i]].id == id)
		{
			for (j = i; j < numactions - 1; j++)
			{ // shift all the pointers to left
				actionsList[j] = actionsList[j + 1];
			}
			actionsList[numactions - 1] = -1; // delete last, that is repeated
			numactions--;
		}
	}
	for (i = 0; i < nummousedownactions; i++)
	{
		if (mousedownActionsList[i]->id == id)
		{
			free(mousedownActionsList[i]);
			for (j = i; j < nummousedownactions - 1; j++)
			{ // shift all the pointers to left
				mousedownActionsList[j] = mousedownActionsList[j + 1];
			}
			mousedownActionsList[nummousedownactions - 1] = NULL; // delete last, that is repeated
			nummousedownactions--;
		}
	}
}
void Webgui::reset()
{
	CHECKCONNECTION();
	_println("RESET");
	numactions = 0;
	//	idcounter = 1;
}
