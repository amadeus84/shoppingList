//
// g++ --std=c++17 -g -Wall -o shoppingListServer shoppingListServer.C
// 
// Server for the shoppingList roboremo app.
//
// 

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cctype>
#include <unordered_map>
#include <algorithm>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/ioctl.h>

#include <unistd.h>
#include <getopt.h>

#include "shoppingListIO.H"


using namespace std;

// fixed length buffer for this application:
const size_t bufLen=1024;


void usage(char * prog)
{
    cerr << endl << "\e[1;31mUsage: \e[0m" 
	 << prog << " [Options] " << endl 
	 << endl;
    
    cerr << "\e[1;33mOptions: \e[0m" << endl
	 << "\t-p port\t\t\tport to listen on (default 22910)" << endl
	 << "\t-l\t\t\twrite list and exit (for debugging)" << endl
	 << "\t-i inputList.txt" << endl
	 << "\t-h\t\t\thelp" << endl
	 << endl;

    cerr << "\e[1;34mPurpose: \e[0m" << endl
	 << "\tShopping list server: maintain a shopping list. " << endl
	 << "\tThe server listens to multiple clients and maintains a " << endl
	 << "\tshopping list based on client commands, common across clients."
	 << endl
	 << "\tAny change by any of the clients is immediately broadcast"
	 << endl
	 << "\tto all clients."
	 << endl << endl
	 << "\t" << prog << " reads a shopping list from stdin of from file (-i)" << endl
	 << "\tand saves it from memory to stdout upon exit (so use redirection)."
	 << endl
	 << "\tThe list files consist of a sequence of key/value pairs" << endl
	 << "\twhere the key is a string and value is 0=don't buy or 1=buy."
	 << endl
	 << endl
	 << "\tExample: " << endl << endl
	 << "\teggs\t0" << endl
	 << "\ttomatoes\t0" << endl
	 << "\tnutella\t1" 
	 << endl << endl;

    cerr << "\e[1;32mExample usage: \e[0m" << endl
	 << "\tcat inputList.txt | " << prog << " -p 22910 > savedList.txt"
	 << "\t" << prog << " -i savedShoppingList.txt -p 22910 > savedList.txt"
	 << endl
	 << endl;

    cerr << "\e[1;35mPre-requisites: \e[0m" << endl
	 << endl;

    cerr << "\e[1;35mTO DO: \e[0m" << endl
	 << "\tAuthentication and encryption." << endl
	 << "\tAutomatically extract lists from RoboRemo shopping list interfaces." << endl
	 << endl;
    
    cerr << "\e[1;35mSource file: \e[0m" << endl
	 << "\t" << __FILE__ << endl
	 << endl;

    exit(-1);    
}


bool done=0;

// Catch ctrl-C
void ctrlC(int sig)
{
    cerr << "Got SIGINT." << endl;
    done=1;
}


void sendList(int fd, const lst_t & lst)
{
    string outBuf;
    for(const auto & [button, status] : lst) {
	outBuf=makeLedIdFromText(button) + (status ? " 1\n" : " 0\n"); // as is
	write(fd,outBuf.c_str(),outBuf.size());
    }
}

int main(int argc, char * argv[])
{
    int opt=-1;
    int port=22910;
    int server_sockfd, client_sockfd;
    socklen_t server_len, client_len;
    sockaddr_in server_address;
    sockaddr_in client_address;
    int result;
    fd_set readfds, testfds;

    char buf[bufLen];

    lst_t lst;
    bool writeListAndExit=0;

    char addrBuf[INET6_ADDRSTRLEN];   // string representation of the client IP address

    string listName;
    
    while((opt=getopt(argc,argv,"p:i:lh"))!=-1) {
	switch(opt){
	case 'p':
	    port=strtol(optarg,NULL,10);
	    break;
	case 'i':
	    listName=optarg;
	    break;
	case 'l':
	    writeListAndExit=1;
	    break;
	case 'h':
	default:
	    usage(argv[0]);
	    break;
	}
    }


    signal(SIGINT, ctrlC);

    if(listName.empty())
	lst=readList(0,1);           // replace ' ' with '_'
    else
	lst=readList(listName.c_str(), 1);
    
    if(lst.empty()) usage(argv[0]);

    if(writeListAndExit) {
	writeList(lst);
	exit(0);
    }
    

    // Create server socket:
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // Hack to close the connection immediately if need be:
    // set SO_LINGER with timeout=0. Otherwise, the socket enters the
    // TIME_WAIT state upon exit and clients can't connect while in that state.
    // See
    // http://serverfault.com/questions/329845/how-to-forcibly-close-a-socket-in-time-wait
    linger lngr;
    lngr.l_onoff=1;
    lngr.l_linger=0;
    setsockopt(server_sockfd, SOL_SOCKET, SO_LINGER, &lngr, sizeof(lngr));
	       
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(port);
    server_len = sizeof(server_address);

    // Bind it:
    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);

    // Listen to incoming connections:
    listen(server_sockfd, 5);

    // Set up select(), in order to handle multiple clients: we
    // process the one that needs attention, as signaled by select().
    FD_ZERO(&readfds);
    FD_SET(server_sockfd, &readfds);

    // timeval timeout;   // see below
    while(!done) {

        testfds = readfds;

	// Which one needs attention? 
        result = select(FD_SETSIZE, &testfds, (fd_set *)0, 
			(fd_set *)0, (struct timeval *) 0);

	// Could use select() with timeout and non blocking accept().
	// timeout.tv_sec = 2;
	// timeout.tv_usec = 0;
        // result = select(FD_SETSIZE, &testfds, (fd_set *)0, 
	// 		(fd_set *)0, &timeout);

        if(result < 1) {              // happens upon Ctrl-C
            perror("select() returned 0.");
	    done=1;
	    break;
	}

	// look to see which one we need to attend to: 
	for(int fd = 0; fd < FD_SETSIZE; fd++) {

            if(FD_ISSET(fd,&testfds)) {

		// If it's the server 
                if(fd == server_sockfd) {

		    // accept the connection and add new client to watch list:
                    client_sockfd = accept(server_sockfd, 
					   (sockaddr *)&client_address,
					   &client_len);

		    // string tStr=datestr(utc2local(time(0)));
		    // memset(addrBuf,0,INET6_ADDRSTRLEN);
		    inet_ntop(AF_INET,
			      (unsigned char*) &client_address.sin_addr.s_addr,
			      addrBuf, INET6_ADDRSTRLEN);

		    time_t t=time(0);
		    cerr << WhereMacro << ": connection request from "
			 << addrBuf << " at " << ctime(&t) << endl;

		    // Check if the client is legitimate, by reading a secret token:
		    bool isOk=1;
		    
		    if(isOk){
			cerr << "Adding client on fd " << client_sockfd << endl;
			FD_SET(client_sockfd, &readfds);

			// Send the list to the client:
			sendList(client_sockfd,lst);
		    }
		    else{
			cerr << WhereMacro << ": intrusion attempt from "
			     << addrBuf << endl;
			shutdown(fd,SHUT_RDWR);
                        close(fd);
                        FD_CLR(fd, &readfds);
			cerr << "Removing client on fd " << fd << endl;
		    }
                }
                else {

		    // It's one of the clients, sending us data.
		    // We only know it by the file descriptor, nothing else.
		    // In particular, no ip. 

		    // see how many bytes are in the buffer to read
		    int nread=0;
                    ioctl(fd, FIONREAD, &nread);

		    // This is how we check the client is still alive:
		    // if nothing to read, client closed the connection.
		    // Remove it.
                    if(nread == 0) {
			shutdown(fd,SHUT_RDWR);
                        close(fd);
                        FD_CLR(fd, &readfds);
			cerr << "Removing client on fd " << fd << endl;
                    }
                    else {
			memset(buf,0,bufLen);
                        nread=read(fd, buf, bufLen);
			replace(buf,buf+bufLen,'\n','\0');  // in std::string
			string button(buf);
			
			// check if it's in the list, otherwise panic:
			auto bit=lst.find(button);
			if(bit!=lst.end()){
			    bit->second=bit->second ? 0 : 1;  // flip value
			    string outBuf=string("led_")+button+" "+
				(bit->second ? "1" : "0") + "\n";
			
			    // Send this to all connected clients:
			    for(int ff = 0; ff<FD_SETSIZE; ff++) {
				if(FD_ISSET(ff,&readfds) && ff!=server_sockfd) {
				    write(ff, outBuf.c_str(), outBuf.size());
				}
			    }
			}
			else{
			    cerr << WhereMacro << ": "
				 << "unexpected item " << button << " from "
				 << addrBuf << endl;
			    shutdown(fd,SHUT_RDWR);
			    close(fd);
			    FD_CLR(fd, &readfds);
			    cerr << "Removing client on fd " << fd << endl;
			}
			
                    }
                }
            }
        }
    }

    // Clean exit:
    cerr << WhereMacro << ": closing port " << port << endl;
    shutdown(server_sockfd,SHUT_RDWR);
    close(server_sockfd);    // this only closes fd, not connection

    writeList(lst);          // to stdout

    return 0;
}
