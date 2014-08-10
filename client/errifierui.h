#ifndef ERRIFIERUI_H
#define ERRIFIERUI_H

#include <QMainWindow>
#include "dialog.h"
#include <QtGui/QStandardItemModel>
#include "core/workerHandler.h"
#include <transport/TSocket.h>
#include <transport/TBufferTransports.h>
#include <protocol/TBinaryProtocol.h>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TNonblockingServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/transport/TSSLSocket.h>
#include <thrift/transport/TSSLServerSocket.h>

#include <boost/thread.hpp>

#include "core/workerthread.h"

namespace Ui {
class errifierui;
}

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace ::apache::thrift::server;

using boost::shared_ptr;


using namespace workercontroller;
class errifierui : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit errifierui(QWidget *parent = 0);
    ~errifierui();
private:
    Ui::errifierui *ui;
    QMenu *settings;
    QMenu *about;

    QAction *logaction;
    QMenu *serveraction;
    QAction *addserver;
    QAction *removeserver;
    QAction *sslaction;
    dialog dialog1;
    QStandardItemModel *model;

    struct ReturnLogSetting setting;
    std::map <QString, QString> ipnamemap;
    WorkerThread serv;

public Q_SLOTS:
    void addServerHandler();
    void removeServerHandler();
    void logSettingHandler();
private Q_SLOTS:
    void on_ping_clicked();
    void on_pushButton_clicked();
    void on_tableView_doubleClicked(const QModelIndex &index);
};

#endif // ERRIFIERUI_H
