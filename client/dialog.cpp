/**
 * File     : dialog.cpp
 * Purpose  : The file contains various implementations of the GUI all related to the dialog boxes
 *            which will be generated during execution
 *
 * @author : Ayush Choubey(ayush.choubey@gmail.com)
 * @version: 1.0
 */
#include "dialog.h"
dialog::dialog(QWidget *parent) :
    QDialog(parent)
{

}
/**
 * The dialog box to be generated, when the Server>Add is selected from the menu
 *
 * @return ReturnAddServer
 */
ReturnAddServer dialog::addServer(){
    this->setWindowTitle("Add Server");

    QLineEdit serverip(this);
    QLineEdit servername(this);
    QPushButton ok("Ok",this);

    QLabel serverip_label("Server IP", this);
    QLabel serverName_label("Server Name", this);

    serverip.move(120, 50);
    serverip_label.move(25,50);

    servername.move(120, 100);
    serverName_label.move(25,100);

    ok.move(135,150);
    connect(&ok,SIGNAL(clicked()),(&ok)->parent(),SLOT(close()));

    this->setMinimumSize(300,200);
    this->setWindowFlags(Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    this->setModal(true);

    this->show();
    this->exec();
    ReturnAddServer returnvalue;
    returnvalue.serverip = serverip.text();
    returnvalue.servername = servername.text();

    return returnvalue;
}

QString dialog::removeServer(QComboBox *serverlist)
{
    QString serverString = serverlist->currentText();
    serverlist->removeItem(serverlist->currentIndex());
    return serverString;
}

/**
 * This dialog box is used to set the way the log is read.
 * Currently, how many lines should be read is implemented.
 *
 * @return ReturnLogSetting
 */
ReturnLogSetting dialog :: setLogOption(ReturnLogSetting setting_in)
{
    this->setWindowTitle("Log Settings");

    QRadioButton entirelog("Entire log", this);
    QRadioButton lastsomeline("last some line",this);
    QLineEdit lines_qty(this);

    QPushButton ok("Ok",this);

    entirelog.move(50, 50);
    lastsomeline.move(50,100);
    lines_qty.move(175,95);

    ok.move(120, 150);
    connect(&ok,SIGNAL(clicked()),(&ok)->parent(),SLOT(close()));
    connect(&lastsomeline,SIGNAL(toggled(bool)), &lines_qty,SLOT(setEnabled(bool)));

    this->setMinimumSize(300,200);
    this->setWindowFlags(Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    this->setModal(true);

    //Showing Pevious Log Setting
    if(setting_in.checkbox == 1){
        lines_qty.setEnabled(false);
        entirelog.setChecked(true);

    }else {
        lines_qty.setEnabled(true);
        lines_qty.setText(setting_in.logLines.c_str());
        lastsomeline.setChecked(true);
    }

    this->show();

    this->exec();
    ReturnLogSetting setting;
    if(entirelog.isChecked()){
        setting.checkbox = 1;
        setting.logLines = '0';
    } else {
        setting.checkbox = 0;
        if(lines_qty.text().length() > 0) {
            setting.logLines = lines_qty.text().toStdString();
        } else {
            setting.logLines = "0";
        }
    }
    return setting;
}
/**
  * When client Successfully pings the server
  * @param pingval Gives ping value

  */
void dialog :: showPingMsg(int pingval){
    switch(pingval){
        case 200:{
            QMessageBox::information(NULL, "Ping Successful!",
                                 "Server is live you can continue");
        }
    }
}
