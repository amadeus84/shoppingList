#ifndef _SHOPPINGLISTIO_H_
#define _SHOPPINGLISTIO_H_

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>

#define WhereMacro __FILE__ << ": line: " << __LINE__ << ": " << __FUNCTION__

using namespace std;

typedef map<string,bool> lst_t;  // use ordered map, to preserve order

lst_t readList(const char * fname=0, bool replWhiteSp=0, char delim='\t')
{
    string item, restOfLine;
    int value=0;
    lst_t lst;
    istream * In=&cin;

    if(fname) In=new ifstream(fname,ios::in);

    // This uses delim to distinguish between item and value.
    while(getline(*In,item,delim) >> value) {
	if(replWhiteSp) replace(item.begin(),item.end(),' ','_');
	lst[item]=value;
	getline(*In,restOfLine,'\n');   // read rest of line
    }
    
    if(In!=&cin) delete In;

    cerr << WhereMacro << ": read list from " << (fname ? fname : "stdin")
	 << endl;
    
    return move(lst);
}

void writeList(const lst_t & lst, const char * fname=0)
{
    ostream * Out=&cout;
    if(fname) Out=new ofstream(fname,ios::out);
	
    for(auto& [key, value] : lst)
	*Out << key << "\t" << value << endl;

    if(Out!=&cout) delete Out;
}

string makeLedIdFromText(string text,string prefix="led_")
{
    replace(text.begin(),text.end(),' ','_');
    text=prefix+text;  
    return move(text);
}



#endif
