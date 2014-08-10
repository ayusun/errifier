#include <boost/thread.hpp>
#include <sys/inotify.h>
#include <libnotify/notify.h>
#include <QtGui>

#include <transport/TSocket.h>
#include <transport/TBufferTransports.h>
#include <protocol/TBinaryProtocol.h>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TNonblockingServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/transport/TSSLSocket.h>
#include <thrift/transport/TSSLServerSocket.h>
#include "workerHandler.h"

using namespace workercontroller;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using boost::shared_ptr;

#define MAX_THREAD 25
#define THREAD_LOCK 0
#define THREAD_STOP 2

#define THREAD_RUN 1
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

/**
 * This class wraps over the thread with some other essential fields
 */
class ThreadWrap {
    public:
        std::string _servername;
        boost::thread *_thread;
        int *_runstate;

        ThreadWrap(std::string servername);
        int *getRunState();
        void setState(int state);
        std::string getServername();
};

/** This class is used by main thread to create watches over remote files
  * by creating various threads
  */
class WorkerThread {
    private:
        std::vector <ThreadWrap> threadList;
    public :
        QAbstractItemModel *model;

        int startNewThread(std::string filename, std::string ip, std::string friendlyname, std::string servername, int ssl);
        int startTimedoutThread(std::string filename, std::string ip, std::string friendlyname, std::string servername, int threadNum, int ssl);
        void setThreadLock(int threadnumber);
        void setThreadRun(int threadnumber);
        static void startWatch(int *runstate, std::string filename, std::string friendlyname, std::string servername, QAbstractItemModel *model, boost::shared_ptr<TTransport> transport,workerHandlerClient client);
        void removeServer(std::string servername);
        int getThreadCount();

};
