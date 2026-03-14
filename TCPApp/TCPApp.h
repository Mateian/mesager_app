#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_TCPApp.h"
#include <QTcpSocket>
#include <QTcpServer>
#include <QHostAddress>
#include <qlist.h>
#include <QFileDialog>
#include <QFile>

class TCPApp : public QMainWindow
{
    Q_OBJECT

public:
    TCPApp(QWidget *parent = nullptr);
    ~TCPApp();

private slots:
    void NewApp();
    void ClearChat();
    void QuitApp();
    void ShowHow();
    void ShowInfo();
    void SendMessage();
    void ConnectToPeer();
    void StartServer();
    void HandleNewConnection();
    void ReadMessage();
    void SendFile();
    void SaveReceivedFile(const QString &filename, const QByteArray &filedata);

private:
    Ui::TCPAppClass* ui;
    QTcpSocket* socket = nullptr; // conexiunea client
    QTcpServer* server = nullptr; // server
    QList<QTcpSocket*> clients;
    QFile* receivingFile = nullptr;
    qint64 totalBytes = 0;
    qint64 bytesReceived = 0;
};

