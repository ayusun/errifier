/**
 * File     : workHandler-server.cpp
 * Purpose  : The file contains Server implementation. Generated by thrift. Refer .thrift file for IDL defination
 *
 * @author : Ayush Choubey(ayush.choubey@gmail.com)
 * @version: 1.0
 */
#include "workerHandler.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TNonblockingServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/PosixThreadFactory.h>
#include <thrift/concurrency/PlatformThreadFactory.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/transport/TSSLSocket.h>
#include <thrift/transport/TSSLServerSocket.h>

#include <boost/thread.hpp>
#include <sys/inotify.h>
#include <libnotify/notify.h>
#include <sys/stat.h>

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )
#define SERVER_THREAD 50

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace ::apache::thrift::concurrency;

using boost::shared_ptr;

using namespace  ::workercontroller;

class workerHandlerHandler : virtual public workerHandlerIf {

public:
	workerHandlerHandler() {
    }
    
    /**
     *  When Client Calls Ping, means the client is able to reach the server, return 200
     */
    int32_t ping() {
		printf("ping");
		return 200;
    }
    
    /**
     * Client requests to read the log
     * 
     * @param _return  Out Read log is assigned to this variable
     * @param filename IN  File to be watched
     * @param type     IN  The type of reading(last certain lines or all the lines)
     * @param lines    IN  No. Of lines to be actually read
     * 
     */ 
    void getLog(std::string& _return, const std::string& filename, const int32_t type, const std::string& lines) {
        
        std::string command;  
        std::string totalBuf;
        char *buffer = new char[2048];
        
	    if(type == 0) {
		    command = "tail -n "; //use tail command to read last some lines
   		    command = command + lines + " " + filename;

	    } else {
		    command = "cat "; //use cat command to get the entire log output
		    command = command + filename;
		}
		
		FILE* file = popen(command.c_str(), "r");
			
		//start reading the output line by line into buffer variable 
		//and concatinate them in totalBuf
		while(fgets(buffer, 2048, file)){
		    totalBuf = totalBuf + std::string(buffer) + "\n\r\n\r"; 
		}
		
		pclose(file);
		free(buffer);
			
		_return = totalBuf;
    }
    
    /**
     * Tells whether the filename entered by user on client side, actually exist or not
     * 
     * @param filename
     * 
     * @return int
     */
    int32_t isFileExist(const std::string& filename) {
	    struct stat buf;
		if(stat(filename.c_str(), &buf) == 0) {
		    return 1;
		} else {
		    return 0;
		}
    }
    
    /**
     * Watches the file for any changes using inotify
     * 
     * @param filename File to be watched
     * 
     * @return int
     */
    int32_t startNotifier(const std::string& filename) {
	    int fd;
		int wd;
		
		fd = inotify_init();
		if ( fd < 0 ) {
			perror( "inotify_init" );
		}
	
		//Use Inotify to watch the file for any modifications
	
		wd = inotify_add_watch( fd, filename.c_str(), IN_MODIFY );
		
		int length, i = 0;
		char buffer[BUF_LEN];
		while(1) {
	            length = read( fd, buffer, BUF_LEN );
	            while(length > 0 && i < length){
		            struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
		            if ( event->mask & IN_MODIFY ) {
			            return 1;
				    }
			        i += EVENT_SIZE + event->len;
		        }
		        i = 0;
	     }
	     close(fd);
    }

};

// Main function that drives the program 
int main(int argc, char **argv) {
  int port = 59999;
  int opt, sslselected = 0;
  while ((opt = getopt(argc,argv,"s")) != EOF){
      switch(opt){
	      case 's':
	          sslselected = 1;
	          printf("Server is started with SSL option\n");
	          fflush(stdout);
	          break;
	      case '?':
	          printf("Select correct value\n");
	          exit(0);
	          break;
	  }
  }
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
  shared_ptr<workerHandlerHandler> handler(new workerHandlerHandler());
  shared_ptr<TProcessor> processor(new workerHandlerProcessor(handler));
  
  //signal(SIGPIPE, SIG_IGN); // See README.SSL
  shared_ptr<TServerSocket> socket;
  if(sslselected){
      shared_ptr<TSSLSocketFactory> sslSocketFactory(new TSSLSocketFactory());
      sslSocketFactory->loadPrivateKey("server-key.pem");
      sslSocketFactory->loadCertificate("server-cert.pem");
      sslSocketFactory->authenticate(true);
      sslSocketFactory->loadTrustedCertificates("client-cert.pem");
      sslSocketFactory->ciphers("HIGH:!DSS:!aNULL@STRENGTH");
      socket = shared_ptr<TServerSocket>(new TSSLServerSocket(port, sslSocketFactory));
  }
  else
      socket = shared_ptr<TServerSocket>(new TServerSocket(port));
  
  shared_ptr<TTransportFactory> transportFactory(new TFramedTransportFactory());
  shared_ptr<ThreadManager> threadManager = ThreadManager::newSimpleThreadManager(SERVER_THREAD);
  shared_ptr<PlatformThreadFactory> threadFactory = shared_ptr<PlatformThreadFactory>(new PlatformThreadFactory());
  threadManager->threadFactory(threadFactory);
  threadManager->start();
  
  TThreadPoolServer server(processor, socket, transportFactory, protocolFactory, threadManager);
  server.serve();
    
  return 0;
}

