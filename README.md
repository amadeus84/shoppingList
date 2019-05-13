Broadcast server:
	  Suppose we have a list of binary variables (think LEDs) that we
	  want to maintain across a network of clients (appliances).

	  When a client toggles one of the variables, we want the other clients
	  to be notified instantly. We do so by implementing a broadcast
	  server, that all clients talk to. The clients and the server have
	  the same list of variables. The server maintains the state of
	  each variable and when one client toggles one of the variables
	  (e.g. by pressing a button), the server updates the state and
	  broadcasts the updated value to all clients connected to it.


Application: shopping list client/server.

         This is a smart version of a pencil and paper shopping list. Any
	 member of a household can add items to the list, except that instead
	 of dynamically adding items (on a white board or on paper), they
	 toggle (1 = buy, 0 = don't buy) items in a predefined list of
	 things to buy. This list might look like

	 > cat shoppingList.txt
	 milk	1
	 butter	0
	 ham	1
	 eggs	1
	 meat	0
	 fruit	0	 

	 Everybody should be able to toggle an item on the list and the 
	 updated list should be pushed to everybody else's device.

	 When someone in the family is actually shopping, they can (should)
	 toggle the items to off, as they shop them. This is the equivalent
	 of crossing something out on the paper list.
	 


Server: shoppingListServer.C: server

	cat shoppingList.txt | shoppingListServer -p 12345


Client: For testing purposes, just connect to the server port on localhost:

	nc localhost <port>


	Graphical client: a RoboRemo interface created with

        roboRemoInterfaces/shoppingListMain.C

RoboRemo: is an android (iPhone?) widget kit app that can be used to create
	(by drag-and-drop) interfaces that control robots or other appliances.
	It has widgets for buttons, accelerometer, sliders, LEDs, etc.
	An action can be associated with each widget. For instance, when
	a button is clicked, we can send a string of characters (representing
	a command) to a specific IP address. 

	For this project, RoboRemo can be used to create a table with
	buttons, each with a label representing the item to be bought and with
	an LED on it, showing the state of the item. When the button is
	pushed, the item on the button is sent to the server, which updates
	its status and notifies all other connected clients.

ConnectBot: optionally, this app can be used to encrypt the traffic between
	a smartphone and the shoppingListServer (or any server). If the
	server is running, say, on port 1234 (on some home computer), then
	in ConnectBot define a connection from some port on the localhost
	(the smartphone), e.g. the same 1234 port and then connect RoboRemo
	to port 1234 on the localhost. This establishes an encrypted channel

	127.0.0.1:1234 (smartphone) <---------> home_computer:1234 (server)
	

Compilation instructions are at the top of each source file.


Start the server like this:

      cat defaultShoppingList.txt | ./shoppingListServer > savedShoppingList.txt


Create a roboremo client interface like this:

       cat defaultShoppingList.txt | roboRemoInterfaces/shopinglistMain -r 

