#ifndef _ROBOREMO_H_
#define _ROBOREMO_H_

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <typeinfo>
#include <utility>
#include <algorithm>

#include <unordered_map>

#define WhereMacro \
    __FILE__ << ": line: " << __LINE__ << ": " << __FUNCTION__


using namespace std;

typedef unordered_map<string,string> map_t;
typedef pair<string,string> strPair_t;

#define typeidStr(x) M[typeid(x).name()]
#define typeidSize(x) M[typeid(x).name()].size()
#define typeidPrint(x) M[typeid(x).name()].size() << " " << M[typeid(x).name()]
// #define typeidPair(x) strPair_t(typeid(x).name(),sizeof(typeid(x).name()))



template <class Type>
string num2str(Type x,int precision=0)
{
    ostringstream Out;
    if(precision) Out << setprecision(precision);
    Out << x;
    return Out.str();
}



class Token
{
public:

    string name;
    string type;
    string value;

    Token(const string nm="", const string tp="", const string val="")
	: name(nm), type(tp), value(val) {}

    string tostring() const {
	ostringstream Out;
	Out << *this << " ";
	return Out.str();
    }

    // Cast to string, ready for output, so we can do
    // cout << (string) token;
    explicit operator string() const {
	ostringstream Out;
	Out << name.size() << " " << name << " "
	    << type.size() << " " << type << " "
	    << value.size() << " " << value << " ";
	string str=Out.str();      // for debugging
	return Out.str();
    }

    size_t strlen() const { return ((string) *this).size(); }

    Token & convert(const map_t & M) {
	map_t::const_iterator iter;
	if((iter=M.find(type))!=M.end()) type=iter->second;
	return *this;
    }
  
    Token & operator=(const string & val) {
	value=val;
	return *this;
    }
    

    bool operator==(const Token & y) const {
	return type==y.type && name==y.name;
    }

    bool operator==(const string & tokenName) const { return name==tokenName; }
    
    
    friend istream & operator>>(istream & In, Token & x) {

	size_t count=0;

	x=Token();
	
	// Token::value might have white spaces (if String), so read it whole:
	In >> count >> x.name >> count >> x.type >> count;
	In.ignore(); // skip the white space after count.
	for(size_t i=0; i<count; i++) x.value.push_back(In.get());

	return In;
    }

    // Easy read. Cast to string to output it for roboremo.
    friend ostream & operator<<(ostream & Out, const Token & x) {
	Out << x.type << " " << x.name << " " << x.value;  // easy read
	return Out;
    }

    
};


class Interface
{
public:
    Token itemCount, name, commandEnding, connectAction;

    Interface(size_t n=0, string Name="", string ConnectAction="",
	      string CommandEnding="\n")
	: itemCount("itemCount","int",num2str(n+1)),
	  name("name","String",Name),
	  commandEnding("commandEnding","String",CommandEnding),
	  connectAction("connectAction","String",ConnectAction) {}


    explicit operator string() const {

	string out;
	out+=(string) itemCount;
	out+=(string) name;
	out+=(string) commandEnding;
	out+=(string) connectAction;
	// out+=(string) itemCount + " ";
	// out+=(string) name + " ";
	// out+=(string) commandEnding + " ";
	// out+=(string) connectAction + " ";
	return out;
    }

    
    Interface & convert(const map_t M) {
	itemCount.convert(M);
	name.convert(M);
	commandEnding.convert(M);
	connectAction.convert(M);
	return *this;
    }
    
    friend istream & operator>>(istream & In, Interface & x) {
	In >> x.itemCount >> x.name >> x.commandEnding >> x.connectAction;
	return In;
    }

    friend ostream & operator<<(ostream & Out, const Interface & x) {
	// Out << x.itemCount << " " << x.name << " " << x.commandEnding << " "
	//     << x.connectAction;
	Out << (string) x.itemCount << " " << (string) x.name << " "
	    << (string) x.commandEnding << " " << (string) x.connectAction;
	return Out;
    }
};


class Widget
{
public:

    string id;
    string type;
    size_t nchar;
    vector<Token> items;

    size_t size() const {
	size_t sz=0;
	for(size_t i=0; i<items.size(); i++) sz+=items[i].strlen();
	return sz;
    }


    explicit operator string() const {

	ostringstream OutHeader, OutBody;

	for(size_t i=0; i<items.size(); i++)
	    OutBody << (string) items[i];
	
	OutHeader << id.size() << " " << id << " "
		  << type.size() << " " << type << " "
		  << OutBody.str().size() << " ";

	string str=OutHeader.str() + OutBody.str();
	
	return str;
    }

    Token & operator[](const string & tokenName) {
	
	vector<Token>::iterator it=find(items.begin(), items.end(), tokenName);
	if(it==items.end()){
	    cerr << WhereMacro << ": Can't find item " << tokenName << " "
		 << "in widget " << type << " " << id << endl;
	    exit(-1);
	}
	return *it;
    }

    
    friend istream & operator>>(istream & In, Widget & W) {

	W.items.clear();
	
	// Header: id and widget type:
	size_t count=0;
	In >> count >> W.id >> count >> W.type >> W.nchar;
	// In.ignore();
	
	string tmp(W.nchar+1,0);
	char * buf=&tmp[0];
	In.read(buf,W.nchar);

	istringstream InStr(tmp);
	Token tok;
	while(InStr>>tok) W.items.push_back(tok);
	
	return In;
    }

    friend ostream & operator<<(ostream & Out, const Widget & W) {
	// Out << W.hdr << " ";
	Out << W.id.size() << " " << W.id << endl
	    << W.type.size() << " " << W.type << " "
	    << W.nchar << endl;
	for(size_t i=0; i<W.items.size(); i++) Out << W.items[i] << endl;
	return Out;
    }



    // Widget factory

    static Widget createButton(int id=1, string text="Button",
			       string pressAction="",
			       string releaseAction="",
			       float x=0.5, float y=0.5,
			       float w=0.3, float h=0.1,
			       int precision=8) {
	Widget W;

	string xx=num2str(x,precision);
	string yy=num2str(y,precision);
	string ww=num2str(w,precision);
	string hh=num2str(h,precision);
	
	// string pressAction=text;    // TODO: random string
	replace(pressAction.begin(),pressAction.end(),' ','_');
	replace(releaseAction.begin(),releaseAction.end(),' ','_');
	
	W.id=num2str(id);
	W.type="UiButton";
	W.items.push_back(Token("x","float",xx));
	W.items.push_back(Token("y","float",yy));
	W.items.push_back(Token("w","float",ww));
	W.items.push_back(Token("h","float",hh));
	W.items.push_back(Token("isEditLocked","boolean","false"));
	W.items.push_back(Token("text","String",text));
	W.items.push_back(Token("pressed","boolean","false"));
	W.items.push_back(Token("pressAction","String",pressAction));
	W.items.push_back(Token("releaseAction","String",releaseAction));
	W.items.push_back(Token("repeatDelay","int","0"));
	W.items.push_back(Token("repeatPeriod","int","0"));
	W.items.push_back(Token("alwaysPressed","boolean","false"));
	W.items.push_back(Token("local","boolean","false"));

	string Wstr=(string) W;
	W.nchar=Wstr.size();
	
	return W;
    }


    static Widget createLED(int id=1, string ledId="",string label="",
			    string color="g",
			    float x=0.5, float y=0.5,
			    float w=0.3, float h=0.1,
			    int precision=8) {
	Widget W;

	string xx=num2str(x,precision);
	string yy=num2str(y,precision);
	string ww=num2str(w,precision);
	string hh=num2str(h,precision);

	W.id=num2str(id);
	W.type="UiLed";
	W.items.push_back(Token("x","float",xx));
	W.items.push_back(Token("y","float",yy));
	W.items.push_back(Token("w","float",ww));
	W.items.push_back(Token("h","float",hh));
	W.items.push_back(Token("isEditLocked","boolean","false"));
	W.items.push_back(Token("id","String",ledId));
	W.items.push_back(Token("onCommand","String","1"));
	W.items.push_back(Token("offCommand","String","0"));
	W.items.push_back(Token("label","String",label));
	W.items.push_back(Token("on","boolean","false"));
	W.items.push_back(Token("color","String",color));
	W.items.push_back(Token("onTimeout","int","0"));
	W.items.push_back(Token("offTimeout","int","0"));
    
	string Wstr=(string) W;
	W.nchar=Wstr.size();

	return W;
    }



    static Widget createHeartbeat(int id=1,
				  string heartId="",
				  int repeatPeriod=1000,
				  float x=0.5, float y=0.5,
				  float w=0.3, float h=0.1,
				  int precision=8) {
	Widget W;

	string xx=num2str(x,precision);
	string yy=num2str(y,precision);
	string ww=num2str(w,precision);
	string hh=num2str(h,precision);
	string rp=num2str(repeatPeriod,0);
	
	W.id=num2str(id);
	W.type="UiHeartbeatSender";
	W.items.push_back(Token("x","float",xx));
	W.items.push_back(Token("y","float",yy));
	W.items.push_back(Token("w","float",ww));
	W.items.push_back(Token("h","float",hh));
	W.items.push_back(Token("isEditLocked","boolean","false"));
	W.items.push_back(Token("repeatPeriod","int",rp));
	W.items.push_back(Token("id","String",heartId));
	W.items.push_back(Token("label","String",""));

	string Wstr=(string) W;
	W.nchar=Wstr.size();
	
	return W;
    }

    static Widget createMenuButton() {
	Widget Menu=createButton(0,"menu","","menu",0.02,0.02,0.2,0.16);
	Menu["local"].value="true";
	return Menu;
    }

};






    

#endif
