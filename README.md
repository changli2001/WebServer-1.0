# WebServer-1.0
This Project Creat a Complete TCP  server . acting as a HTTP server;
The HTTP vrsion used as a Reference is : 1.0

//Step 1 :
    The first step of creating our server is to build the server architecture
    and make it listening for incomin requests;
    
        socket() -> This step aim to creat a socket That the server
                    use it to listen for incoming requests;
        
                 -> Make the socket reusable;
        bind()   -> Link to socket with a specific port and address;
        listen()   -> Now our socket is listening for new client requests;

//Step 2
    After setting Up our server , Now we are going to use the Select() sys call
    The select() sys call job is To keep track and monitoring the server conneted
    FDs and listening socket;