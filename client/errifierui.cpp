/**
 * File     : errifierui.cpp
 * Purpose  : The file contains various implementations of the GUI such as when ping button
 *            IS CLICKED, or when "start watch" button is CLICKED, when we want to
 *            change the status of error reporting(Active/Paused) by double clicking, or seeing
 *            the last logged errors.
 *
 * @author : Ayush Choubey(ayush.choubey@gmail.com)
 * @version: 1.0
 */

#include "errifierui.h"
#include "ui_errifierui.h"
#include <QDebug>
errifierui::errifierui(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::errifierui)
{
    ui->setupUi(this);

    //Setting up menu

    settings = menuBar()->addMenu("Settings");

    logaction = new QAction("&Log", this);
    serveraction = new QMenu("&ServerRecord", this);
    addserver = new QAction("Add Server", this);
    removeserver = new QAction("Remove Server", this);
    sslaction = new QAction("SSL", this);
    sslaction->setCheckable(true);


    settings->addAction(logaction);
    settings->addMenu(serveraction);

    serveraction->addAction(addserver);
    serveraction->addAction(removeserver);
    settings->addSeparator();
    settings->addAction(sslaction);


    //Adding onclick event for menu items
    connect(addserver, SIGNAL(triggered()), this, SLOT(addServerHandler()));
    connect(removeserver,SIGNAL(triggered()),this,SLOT(removeServerHandler()));
    connect(logaction, SIGNAL(triggered()),this,SLOT(logSettingHandler()));

    //By default setting to read log
    setting.checkbox = 1;
    setting.logLines = '0';

    //Start setting TableView, where data can be displayed.
    model = new QStandardItemModel(this);
    //Initialising three headers of the table.

    model->setHorizontalHeaderItem(0, new QStandardItem(QString("Server Name")));
    model->setHorizontalHeaderItem(1, new QStandardItem(QString("File Location")));
    model->setHorizontalHeaderItem(2, new QStandardItem(QString("Friendly Name")));
    model->setHorizontalHeaderItem(3, new QStandardItem(QString("Status")));

    ui->tableView->setModel(model);
    ui->tableView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);

    // Makes the window of standard size, So that it can't be maximised or resized
    setWindowFlags( (windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowMaximizeButtonHint);
    setFixedSize(width(), height());

    //Adding HOST data to combo box
    ui->serverlist->addItem("HOST(127.0.0.1)");

    //Initialising serv(WorkerThread)
    serv.model = ui->tableView->model();
}

/** When log is clicked from seetings in menubar**/
void errifierui :: logSettingHandler()
{
    setting = dialog1.setLogOption(setting);
}

/**
  * When add server is clicked from menu bar, then this method is called.
  * When the data is entered in the dialog box, the data returns and is added
  * to combo box.
 */
void errifierui :: addServerHandler()
{
    ReturnAddServer addserverResult = dialog1.addServer();
    if(addserverResult.servername.length() > 0 && addserverResult.serverip.length() > 0) {
        this->ui->serverlist->addItem(addserverResult.servername + "(" + addserverResult.serverip + ")");
    }
}

/**
  * When remove server is clicked from the menu bar.It removes the data from the
  * combo box.
  * To do : It should also stop any thread that runs to watch that sever
  */
void errifierui ::removeServerHandler()
{
    QString server_addr = dialog1.removeServer(this->ui->serverlist);
    int lastindex = server_addr.lastIndexOf("(");
    QString servername, serverip;
    int length = server_addr.length();
    for(int i = 0;i < length - 1;i++){
        if(i > lastindex){
            serverip.append(server_addr.at(i));
        }else if(i < lastindex){
            servername.append(server_addr.at(i));
        }
    }
    serv.removeServer(servername.toStdString());


    QAbstractItemModel *model = ui->tableView->model();
    int i = 0;
    while(i < model->rowCount()){
        std::string tbl_server  = model->index(i,0).data().toString().toStdString();
        if(tbl_server.compare(servername.toStdString()) == 0){
            model->removeRow(i);
        } else{
            i++;
        }
    }

}

errifierui::~errifierui()
{
    delete ui;
}

/**
  * when ping button is clicked. It opens a client and conects to server. If return value
  * is 200, display the successful message box.
  */
void errifierui::on_ping_clicked()
{
    QString server_addr = this->ui->serverlist->currentText();
    int length = server_addr.length();
    int i;
    int lastindex = server_addr.lastIndexOf("(");
    QString servername, serverip;
    // Finding servername and server ip from the text of combo box
    for(i = 0;i < length - 1;i++){
        if(i > lastindex){
            serverip.append(server_addr.at(i));
        }else if(i < lastindex){
            servername.append(server_addr.at(i));
        }
    }

    try{

        boost::shared_ptr<TTransport> transport;

        if(!this->sslaction->isChecked()) {
            boost::shared_ptr<TSocket> socket(new TSocket(serverip.toUtf8().data(), 59999));
            transport = boost::shared_ptr<TTransport>(new TFramedTransport(socket));
        } else {
            shared_ptr <TSSLSocketFactory> socketFactory = shared_ptr<TSSLSocketFactory>(new TSSLSocketFactory());
            socketFactory->loadPrivateKey("client-key.pem");
            socketFactory->loadCertificate("client-cert.pem");
            socketFactory->loadTrustedCertificates("server-cert.pem");
            socketFactory->authenticate(true);
            socketFactory->ciphers("HIGH:!DSS:!aNULL@STRENGTH");

            shared_ptr <TSSLSocket>socket = socketFactory->createSocket(serverip.toStdString(), 59999);
            transport = boost::shared_ptr<TTransport>(new TFramedTransport(socket));
        }
        boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));

        workerHandlerClient client(protocol);
        transport->open();
        int pingValue = client.ping();
        transport->close();
        dialog1.showPingMsg(pingValue);
    }catch(TSSLException e){
        QMessageBox::warning(NULL, "Connection Error",
                                     "Cannot Authenticate. Please Check your certificates.");
    }
    catch(TTransportException exception){
        QMessageBox::warning(NULL, "Connection Error",
                                     "Seems like the server is not yet activated or is being blocked down by firewall. Please check it.");
    }
}

/**
 *  Checks whether the credentials entered are already present or not
 *
 *  @param servername   Name of the server
 *  @param fileloc      Location of the File in char *
 *  @param friendlyName Friendly Name used to differentiate between services
 *  @param ui           Pointer to UI
 *
 *  @return int
 */
int fileAlreadyMonitored(const char *servername,const char *fileloc, const char*friendlyName, Ui::errifierui *ui)
{
    QAbstractItemModel *model = ui->tableView->model();
    int rowCount = model->rowCount();
    int i = 0;
    int ispresent = 0;
    for(i=0;i<rowCount;i++){
        const char *tbl_server  = model->index(i,0).data().toString().toUtf8().constData();
        const char *tbl_fileloc = model->index(i,1).data().toString().toUtf8().constData();
        const char *tbl_friendly= model->index(i,2).data().toString().toUtf8().constData();

        if((strcmp(tbl_fileloc,fileloc)==0 && strcmp(tbl_server, servername) == 0) || (strcmp(tbl_friendly, friendlyName) == 0)) {
            ispresent = 1;
            break;
        }
    }
    return ispresent;
}

/**
 * When "Start Watch" button is clicked. Check if the file is already being
 * monitored, if not, then a thread is started which montors the file
 */
void errifierui::on_pushButton_clicked()
{
        QString server_addr = this->ui->serverlist->currentText();
        QString fileloc = this->ui->txt_log->text();
        QString friendlyName = this->ui->txt_friendlyname->text();

        if(fileloc.length() > 0 && friendlyName.length() > 0) {
            int length = server_addr.length();
            std::string filename = fileloc.toStdString();
            int i;
            int lastindex = server_addr.lastIndexOf("(");
            QString servername, serverip;
            for(i = 0;i < length - 1;i++){
                if(i > lastindex){
                    serverip.append(server_addr.at(i));
                }else if(i < lastindex){
                    servername.append(server_addr.at(i));
                }
            }
            if(!fileAlreadyMonitored(servername.toStdString().c_str(),filename.c_str(),friendlyName.toStdString().c_str(),this->ui)){

                //boost::shared_ptr<TSocket> socket(new TSocket(serverip.toUtf8().data(), 59999));
                //boost::shared_ptr<TTransport> transport(new TFramedTransport(socket));
                //boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));

                try{
                    boost::shared_ptr<TTransport> transport;

                    if(!this->sslaction->isChecked()) {
                        boost::shared_ptr<TSocket> socket(new TSocket(serverip.toUtf8().data(), 59999));
                        transport = boost::shared_ptr<TTransport>(new TFramedTransport(socket));
                    } else {
                        shared_ptr <TSSLSocketFactory> socketFactory = shared_ptr<TSSLSocketFactory>(new TSSLSocketFactory());
                        socketFactory->loadPrivateKey("client-key.pem");
                        socketFactory->loadCertificate("client-cert.pem");
                        socketFactory->loadTrustedCertificates("server-cert.pem");
                        socketFactory->authenticate(true);
                        socketFactory->ciphers("HIGH:!DSS:!aNULL@STRENGTH");

                        shared_ptr <TSSLSocket>socket = socketFactory->createSocket(serverip.toStdString(), 59999);
                        transport = boost::shared_ptr<TTransport>(new TFramedTransport(socket));
                    }

                    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
                    transport->open();

                    workerHandlerClient client(protocol);
                    if(client.isFileExist(fileloc.toStdString())) {
                        ipnamemap.insert(std::pair<QString,QString>(servername,serverip));
                        int threadnum = serv.startNewThread(filename, serverip.toStdString(), friendlyName.toStdString(),servername.toStdString(),sslaction->isChecked());
                        if(threadnum >= 0){
                            QList<QStandardItem*> newRow;
                            newRow.append(new QStandardItem(servername));
                            newRow.append(new QStandardItem(fileloc));
                            newRow.append(new QStandardItem(friendlyName));
                            newRow.append(new QStandardItem(QString("Active")));
                            model->appendRow(newRow);
                        } else if(threadnum == -100){
                            QMessageBox::warning(NULL, "Watch Error",
                                                 "Cannot Watch File, Seems like Connection went down");
                        }
                    } else {
                        QMessageBox::warning(NULL, "File Name Error",
                                             "Are you sure that's a correct filename. Because it probably doesn't exist");
                    }
                    transport->close();

                }catch(TSSLException e){
                        QMessageBox::warning(NULL, "Connection Error",
                                                     "Cannot Authenticate. Please Check your certificates.");
                }catch(std::exception e){

                    QMessageBox::warning(NULL, "Connection Error",
                                         "Seems like the server is not yet activated or is being blocked down by firewall. Please check it.");
                }
            } else {
                QMessageBox::warning(NULL, "File Name Error",
                                     "Seems like the file has already been monitored or you are re-using a friendly name");
            }
        } else{
            QMessageBox::warning(NULL, "Blank Input",
                                 "You just can't enter a Blank field. Enter some value!!");
        }

}

/**
  * When table is double clicked
  * Two cases generate :
  * 1 : When a cell in status colum is clicked, then status of that watch should
  *     toggle between active and paused
  * 2 : When Cell in filename is clicked then, the log should be generated and
  *     should be displayed in QPlainText
  */
void errifierui::on_tableView_doubleClicked(const QModelIndex &index)
{
    QString servername = index.model()->index(index.row(),0).data().toString();
    QString serverip = ipnamemap.find(index.model()->index(index.row(),0).data().toString())->second;
    boost::shared_ptr<TTransport> transport;

    if(!this->sslaction->isChecked()) {
        boost::shared_ptr<TSocket> socket(new TSocket(serverip.toUtf8().data(), 59999));
        transport = boost::shared_ptr<TTransport>(new TFramedTransport(socket));
    } else {
        shared_ptr <TSSLSocketFactory> socketFactory = shared_ptr<TSSLSocketFactory>(new TSSLSocketFactory());
        socketFactory->loadPrivateKey("client-key.pem");
        socketFactory->loadCertificate("client-cert.pem");
        socketFactory->loadTrustedCertificates("server-cert.pem");
        socketFactory->authenticate(true);
        socketFactory->ciphers("HIGH:!DSS:!aNULL@STRENGTH");

        shared_ptr <TSSLSocket>socket = socketFactory->createSocket(serverip.toStdString(), 59999);
        transport = boost::shared_ptr<TTransport>(new TFramedTransport(socket));
    }

    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));

    try{

        transport->open();

        workerHandlerClient client(protocol);
        switch(index.column())
        {
        case 1: //If first column was Clicked. Then we need to show the user the log file
        {
            std::string logData;
            std::string filename;

            filename = index.data().toString().toStdString();
            client.getLog(logData, filename, setting.checkbox, setting.logLines);

            QPlainTextEdit *txt = new QPlainTextEdit();
            txt->appendPlainText(QString(logData.data()));

            txt->showMaximized();

            break;
        }
        case 3: //If 4rd column is double clicked, then pause/Activate the thread

        /** The idea is pretty simple. The row in a table matches to the index of the thread in threadList.
         *  So, we take the row id and then use that as an index of the mutexThread in threadlist change the
         *  mutex value, which actually determines, whether notification should appear or not
         */

            int rowid = index.row();

            std::string state;
            state = index.data().toString().toStdString();
            if(state.compare("Active") == 0) { //If its Active, Change it to Paused
                serv.setThreadLock(rowid);
                this->ui->tableView->model()->setData(index, "Paused");
            }
            else if(state.compare("Paused") == 0){ //If its in Paused State, Change it to active
                serv.setThreadRun(rowid);
                this->ui->tableView->model()->setData(index, "Active");
            } else {
                std::string filename = index.model()->index(index.row(),1).data().toString().toStdString();
                std::string friendlyname = index.model()->index(index.row(),2).data().toString().toStdString();
                if(serv.startTimedoutThread(filename,serverip.toStdString(),friendlyname,servername.toStdString(),rowid, this->sslaction->isChecked()) == 1){
                    this->ui->tableView->model()->setData(index, "Active");
                } else {
                    QMessageBox::warning(NULL, "Connection Error",
                                                 "Sorry seems like the server is still not running.");
                }
            }
        }
        transport->close();
    }catch(std::exception e){
        QMessageBox::warning(NULL, "Connection Error",
                                     "Seems like the server is not yet activated or is being blocked down by firewall. Please check it.");
    }
}
