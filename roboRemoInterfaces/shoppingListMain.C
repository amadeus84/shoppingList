//
// Generate the RoboRemo shopping list interface.
//
// g++ -std=c++17 -g -Wall -o shoppingListMain shoppingListMain.C
//
// Get RoboRemo on an android (iPhone?) device from the app store.
// Then build an interace with a menu button and a table of buttons with
// an led on each button, representing the shopping items.
// 
// Normally, the RoboRemo interface can be built by drag-and-drop, but with
// a comprehensive list of items (buttons) that becomes tedious. 
//
// This program automatically generates the buttons for a list of items
// read from file. The generated list can then be transfered on the smartphone
// and used with RoboRemo.
// 

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

#include <unistd.h>
#include <getopt.h>
#include <cmath>
#include <cctype>
#include <sys/types.h>


#include "roboRemo.H"
#include "../shoppingListIO.H"

using namespace std;


void usage(char * prog)
{
    cerr << endl << "\e[1;31mUsage: \e[0m" 
	 << prog << " [Options]" << endl 
	 << endl;
    
    cerr << "\e[1;33mOptions: \e[0m" << endl
	 << "\t-s syn        send syn string upon connect" << endl
	 // << "\t-k keepalive  send keepalive string to keep alive" << endl
	 // << "\t-t kTime      repeat keepalive every kTime seconds" << endl
	 << "\t-c color      led color (r/g/b/y)" << endl
	 << "\t-r            output in roboremo format" << endl
	 << "\t-h            help" << endl
	 << endl;

    cerr << "\e[1;34mPurpose: \e[0m" << endl
	 << "\tGiven a list of (shopping) items, create a RoboRemo interface"
	 << endl
	 << "\tconsisting of a table of buttons with LEDs on them, for toggling"
	 << endl
	 << "\tthe items." << endl
	 << endl;

    cerr << "\e[1;32mExample usage: \e[0m" << endl
	 << "\tcat ../savedShoppingList.txt | " << prog << " -r > rrShoppingListInterface" << endl
	 << endl;

    cerr << "\e[1;35mPre-requisites: \e[0m" << endl
	 << endl;

    cerr << "\e[1;35mSource file: \e[0m" << endl
	 << "\t" << __FILE__ << endl
	 << endl;

    exit(-1);    
}


// ../shoppingListIO.H already has a readList() function to read the list items,
// but that returns a std::map, and so we lose the ordering of the items on
// file. We do want to preserve that ordering, so here we read the items in a
// vector.
vector<string> readVector(const char * fname=0, bool replWhiteSp=0, char delim='\t')
{
    string item, restOfLine;
    int value=0;
    istream * In=&cin;
    vector<string> lst;
	
    if(fname) In=new ifstream(fname,ios::in);

    // This uses delim to distinguish between item and value.
    while(getline(*In,item,delim) >> value) {
	if(replWhiteSp) replace(item.begin(),item.end(),' ','_');
	lst.push_back(item);
	getline(*In,restOfLine,'\n');   // read rest of line
    }
    
    if(In!=&cin) delete In;

    cerr << WhereMacro << ": read list from " << (fname ? fname : "stdin")
	 << endl;
    
    return move(lst);
}


int main(int argc, char * argv[])
{
    int opt=0;
    
    Widget W;
    vector<Widget> WVec;        // the list of RR widgets

    float ledFactor=2.5;
    float x=0, y=0, dx=0.248, dy=0.1, dxLed=0.14/ledFactor, dyLed=0.08/ledFactor;
    string ww, hh, xx, yy;
    size_t wid=1;
    string text, ledId, ledColor="r";
    string syn;
    string keepalive;
    // int kTime=1000;             // in ms.

    bool rrformat=0;
    string lstFname;
    
    while((opt=getopt(argc, argv, "s:k:t:c:l:rh"))!=-1){
	switch(opt){
	case 's':
	    syn=optarg;
	    break;
	case 'k':
	    keepalive=optarg;
	    break;
	// case 't':
	//     kTime=strtol(optarg,NULL,10)*1000;
	//     break;
	case 'c':
	    ledColor=*optarg;
	    break;
	case 'r':
	    rrformat=1;
	    break;
	case 'l':
	    lstFname=optarg;
	    break;
	case 'h':
	default:
	    usage(argv[0]);
	    break;
	}
    }

    // keepalive=syn + " " + keepalive;

    vector<string> lst;    
    if(lstFname.empty())
	lst=readVector(0,1);           // replace ' ' with '_'
    else
	lst=readVector(lstFname.c_str(), 1);
    

    Widget Menu=Widget::createMenuButton();
    
    // // Make the heartbeat widget:
    // W=Widget::createHeartbeat(wid++,keepalive,kTime,0.5,0.02,0.02,0.02);
    // WVec.push_back(W);    // otherwise interface has wrong byte size.

    // The buttons and the LEDs:
    
    // sort(lst.begin(),lst.end());
    float offset=0.01;
    float yMenu=strtod(Menu["y"].value.c_str(),NULL);
    float hMenu=strtod(Menu["h"].value.c_str(),NULL);
    float yBase=yMenu+hMenu+offset;

    dy=hMenu/2;    // fit 2 buttons to the right of the menu
    x=0;
    y=yBase;

    for(auto item : lst) {

	// text=item.first;  // has blanks
	text=item;  // has blanks
	W=Widget::createButton(wid++,"",text,"",x,y,dx,dy); // no blanks 
	WVec.push_back(W);
	
	ledId=makeLedIdFromText(text);
	ledFactor=3;
	dxLed=0.14/ledFactor;
	dyLed=0.06/ledFactor;
	W=Widget::createLED(wid++,ledId,text,ledColor,  
			    x+dx/2-dxLed/2,y+0.02,dxLed,dyLed);
	WVec.push_back(W);

	cerr << WhereMacro << ": " << x << "\t" << y << " " << text << endl;
	
	// Increase y as long as y+dy<1; When y+dy>=1, go to the next column:
	// y=0; x+=dx;
	y+=dy+0.0001;
	if(y+dy>1) {
	    y=yBase-2*dy;
	    x+=dx+0.001;
	}
	
    }

    
    // Finally, the interface:
    Interface I(WVec.size(),"",syn);

    if(rrformat){
	cout << (string) I << (string) Menu << " ";
	for(size_t i=0; i<WVec.size(); i++)
	    cout << (string) WVec[i] << " ";
    }
    else{
	cout << I << endl << endl;
	cout << Menu << endl << endl;
	cout << "--------------------------------------------------" << endl;
	for(size_t i=0; i<WVec.size(); i++)
	    cout << WVec[i] << endl << endl;
    }
    
    
    return 0;
}
