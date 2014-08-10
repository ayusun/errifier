#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QtGui>
struct ReturnAddServer
{
    QString serverip;
    QString servername;
};

struct ReturnLogSetting {
    int checkbox; //1 for all 0 for some lines
    std::string logLines; //0 for all lines else the number of lines mentioned
};

class dialog : public QDialog
{
    Q_OBJECT
public:
    explicit dialog(QWidget *parent = 0);
    QString removeServer(QComboBox *serverlist);
    ReturnLogSetting setLogOption(ReturnLogSetting setting_in);
    void showPingMsg(int pingval);
    
    
public Q_SLOTS:
    ReturnAddServer addServer();

};

#endif // DIALOG_H
