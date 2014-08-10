/**
 * File     : workerthread.cpp
 * Purpose  : The file contains various implementations to manage the threads used to watch the servers.
 *
 * @author : Ayush Choubey(ayush.choubey@gmail.com)
 * @version: 1.0
 */
#include "workerthread.h"

/**
 * Construtor for ThreadWrap. Used to initialise the threads
 *
 * @param servername Takes in servername
 *
 */
ThreadWrap :: ThreadWrap(std::string servername){
    _servername = servername;
    _runstate = new int(THREAD_RUN);
}

/**
 * Returns the Pointer to the current running state of the thread
 *
 * @return int* pointer to the thread running state variable
 */
int* ThreadWrap :: getRunState(){
    return _runstate;
}

/**
 * Get the servername that is attached to that partiular thread
 *
 * @return string name of the server
 */
std::string ThreadWrap :: getServername(){
    return _servername;
}

/**
 * Sets the type of running state for that particular thread
 *
 * @param int state new running state value
 *
 * @return void
 */
void ThreadWrap :: setState(int state){
    *_runstate = state;
}

/**
  * @param filename      Name of the file to be watched
  * @param ip            Ip Address of the server that we have to connect to
  * @param friendlyname  Friendly name for that particular watch.(Used during notification)
  * @param servername    Server name(Used during notification)
  *
  * @return int          returns 1. OR -100 if connection goes down
  */
int WorkerThread :: startNewThread(std::string filename, std::string ip, std::string friendlyname, std::string servername, int ssl){

    try{
        boost::shared_ptr<TTransport> transport;

        if(!ssl) {
            boost::shared_ptr<TSocket> socket(new TSocket(ip.data(), 59999));
            transport = boost::shared_ptr<TTransport>(new TFramedTransport(socket));
        } else {
            shared_ptr <TSSLSocketFactory> socketFactory = shared_ptr<TSSLSocketFactory>(new TSSLSocketFactory());
            socketFactory->loadPrivateKey("client-key.pem");
            socketFactory->loadCertificate("client-cert.pem");
            socketFactory->loadTrustedCertificates("server-cert.pem");
            socketFactory->authenticate(true);
            socketFactory->ciphers("HIGH:!DSS:!aNULL@STRENGTH");

            shared_ptr <TSSLSocket>socket = socketFactory->createSocket(ip, 59999);
            transport = boost::shared_ptr<TTransport>(new TFramedTransport(socket));
        }

        boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));

        transport->open();

        workerHandlerClient client(protocol);
        ThreadWrap *thread = new ThreadWrap(servername);
        thread->_thread = new boost::thread(&WorkerThread :: startWatch, thread->_runstate, filename, friendlyname, servername, model, transport, client);
        threadList.push_back(*thread);
        return 1;
    } catch(std::exception e) {
        return -100;  //if connection is not present return -100
    }
}

/**
 * This function is called on the double liking of status column when the cell shows Connection error
 * in the event if a open connection gets closed due to server going down. It tries to create a new
 * connection and starts a new thread to watch the file
 *
 * @param filename     std::string filename to be watched
 * @param ip           std::string IP address to connect to
 * @param friendlyname std::string Friendlyname for that particular watch
 * @param servername   std::string servername for that server
 * @param threadnum    int         thread position number. It will always math with that of table
 *
 * @return int on success returns 1 else returns -100
 */
int WorkerThread :: startTimedoutThread(std::string filename, std::string ip, std::string friendlyname, std::string servername, int threadNum, int ssl){
    try{
        boost::shared_ptr<TTransport> transport;

        if(!ssl) {
            boost::shared_ptr<TSocket> socket(new TSocket(ip.data(), 59999));
            transport = boost::shared_ptr<TTransport>(new TFramedTransport(socket));
        } else {
            shared_ptr <TSSLSocketFactory> socketFactory = shared_ptr<TSSLSocketFactory>(new TSSLSocketFactory());
            socketFactory->loadPrivateKey("client-key.pem");
            socketFactory->loadCertificate("client-cert.pem");
            socketFactory->loadTrustedCertificates("server-cert.pem");
            socketFactory->authenticate(true);
            socketFactory->ciphers("HIGH:!DSS:!aNULL@STRENGTH");

            shared_ptr <TSSLSocket>socket = socketFactory->createSocket(ip, 59999);
            transport = boost::shared_ptr<TTransport>(new TFramedTransport(socket));
        }

        boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));

        transport->open();

        workerHandlerClient client(protocol);

        threadList.at(threadNum).setState(THREAD_RUN);
        threadList.at(threadNum)._thread = new boost::thread(&WorkerThread :: startWatch, threadList.at(threadNum)._runstate, filename, friendlyname, servername, model, transport, client);

        return 1;
    } catch(std::exception e) {
        return -100;  //if connection is not present return -100
    }
}


/**
 * Used to Pause the notification.
 *
 * @param threadnumber index of the thread that should be paused(always matches to the row number of table)
 *
 * @return void
 */
void WorkerThread :: setThreadLock(int threadnumber){
    threadList.at(threadnumber).setState(THREAD_LOCK);
}

/**
 * Used to Resume the notification.
 *
 * @param threadnumber index of the thread that should be paused(always matches to the row number of table)
 *
 * @return void
 */
void WorkerThread :: setThreadRun(int threadnumber){
    threadList.at(threadnumber).setState(THREAD_RUN);
}

/**
 * Used to run by the thread
 *
 * @param runstate      Pointer to integer, that marks whether notification should be shown or not
 * @param filename      The name of the file that should be watched
 * @param friendlyname  The friendly name given for a watch
 * @param servername    Name of the server
 * @param model         Interface to the table of the ui. Used to show the error message in case connection goes down
 * @param transport     Shared pointer containing TTransport. Used So that connection can be closed safely when thread is over
 * @param client        Client connection which is already open
 *
 * @return void
 */
void WorkerThread :: startWatch(int *runstate, std::string filename, std::string friendlyname, std::string servername, QAbstractItemModel *model, boost::shared_ptr<TTransport> transport, workerHandlerClient client){
    std::string msg;
    notify_init(friendlyname.c_str());
    try{
        while((*runstate) != THREAD_STOP) {
            if(*runstate == THREAD_RUN) {
                msg = "";
                if(client.startNotifier(filename) == 1) {

                    msg = msg + "An error occured in " + friendlyname + "@" + servername;

                    NotifyNotification *notice = notify_notification_new (friendlyname.data(), msg.data(), "dialog-error");

                    notify_notification_show (notice, NULL);
                    g_object_unref(G_OBJECT(notice));

                }
            }
        }
        transport->close();
    } catch(std::exception e) {

        msg = msg + "Seems like the server " + servername + " went down.";

        NotifyNotification *notice = notify_notification_new (friendlyname.c_str(), msg.c_str(), "dialog-error");

        notify_notification_show (notice, NULL);
        g_object_unref(G_OBJECT(notice));
        notify_uninit();

        //Write connection error in the status column for that thread
        int rowCount = model->rowCount();
        int i = 0;
        for(i=0;i<rowCount;i++){
            std::string tbl_friendly= model->index(i,2).data().toString().toStdString();
            if(friendlyname.compare(tbl_friendly) == 0){
                break;
            }
        }
        QModelIndex index = model->index(i,3);
        model->setData(index,"Connection Error");
    }
    notify_uninit();
}

/**
 * removes the threads that belong to a particular server when removeServer is selected
 * @param servername std::string  Name of the server
 *
 * @return void
 */
void WorkerThread :: removeServer(std::string servername){
    int i = 0;
    std::string _servername;
    while(i < threadList.size()){
        _servername = threadList.at(i).getServername();
        if(servername.compare(_servername) == 0){
            threadList.at(i).setState(THREAD_STOP);
            threadList.erase(threadList.begin()+i);
        } else {
            i++;
        }
    }
}

/**
 *  Returns the number of active threads
 *
 * @return int No of threads
 */
int WorkerThread :: getThreadCount(){
    return threadList.size();
}
