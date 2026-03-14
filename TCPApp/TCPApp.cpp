#include "TCPApp.h"
#include <qthread.h>
#include "QMessageBox.h"

TCPApp::TCPApp(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::TCPAppClass)
{
    ui->setupUi(this);
    connect(ui->actionNew, &QAction::triggered, this, &TCPApp::NewApp);
    connect(ui->actionClear_Chat, &QAction::triggered, this, &TCPApp::ClearChat);
    connect(ui->actionQuit, &QAction::triggered, this, &TCPApp::QuitApp);
    connect(ui->actionHow, &QAction::triggered, this, &TCPApp::ShowHow);
    connect(ui->actionInfo, &QAction::triggered, this, &TCPApp::ShowInfo);
    connect(ui->sendButton, &QPushButton::clicked, this, &TCPApp::SendMessage);
    connect(ui->messageLineEdit, &QLineEdit::returnPressed, this, &TCPApp::SendMessage);
    connect(ui->connectButton, &QPushButton::clicked, this, &TCPApp::ConnectToPeer);
    connect(ui->startServerButton, &QPushButton::clicked, this, &TCPApp::StartServer);
    connect(ui->sendFileButton, &QPushButton::clicked, this, &TCPApp::SendFile);
}

TCPApp::~TCPApp()
{
    delete ui;
}

void TCPApp::ClearChat() {
    ui->messagesText->clear();
}

void TCPApp::SendMessage() {
    QString message = ui->messageLineEdit->text();
    if (!message.isEmpty()) {
        ui->messagesText->append("<span style='color:blue'>Me</span>: " + message);

        QByteArray data = message.toUtf8();

        if (socket && socket->state() == QAbstractSocket::ConnectedState) {
            socket->write(data);
        }

        for (QTcpSocket* client : clients) {
            if (client->state() == QAbstractSocket::ConnectedState) {
                client->write(data);
            }
        }

        ui->messageLineEdit->clear();
    }
}

void TCPApp::ShowHow() {
    ui->messagesText->append("<span style='color:orange'>Welcome to the app!</span>\n");
    ui->messagesText->append("<span style='color:orange'>Write the port for the server and start it. To connect to a peer, write the IP and the Port of the destination, then press the connect button.</span>");
}

void TCPApp::ShowInfo() {
    ui->messagesText->append("<span style='color:orange'>Creators:<br>Bejenaru Matei-Andrei<br>Boghiu Mihai-Codrin<br>Parasca Raul<br></span>");
}

void TCPApp::NewApp() {
    if (server && server->isListening()) {
        server->close();
        ui->messagesText->append("Server closed.");
    }

    if (socket && socket->state() == QAbstractSocket::ConnectedState) {
        socket->disconnectFromHost();
        socket->waitForDisconnected();
    }
    for (QTcpSocket* client : clients) {
        client->disconnectFromHost();
        client->waitForDisconnected();
    }
    clients.clear();
    ui->messagesText->clear();
    ui->messagesText->append("Client connection closed.");
    ui->messageLineEdit->clear();
    ui->ipText->clear();
    ui->portText->clear();

    ui->messageLineEdit->setFocus();
    this->close();
    TCPApp* app = new TCPApp();
    app->show();
}

void TCPApp::QuitApp() {
    QMessageBox::StandardButton ansBtn = QMessageBox::question(this, "Exit",
        "Are you sure you want to exit?",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    if (ansBtn == QMessageBox::Yes) {
        this->close();
    }
}

void TCPApp::ConnectToPeer() {
    if (socket == nullptr) {
        socket = new QTcpSocket(this);
        connect(socket, &QTcpSocket::readyRead, this, &TCPApp::ReadMessage);
        connect(socket, &QTcpSocket::connected, this, [=]() {
            ui->messagesText->append("<span style='color:green'>Connected to peer!</span>");
        });
        connect(socket, &QTcpSocket::errorOccurred, this, [=](QAbstractSocket::SocketError err) {
            ui->messagesText->append("<span style='color:red'>Connection error: " + socket->errorString() + "</span>");
        });

        QString ip = ui->ipText->toPlainText();
        QString portString = ui->portText->toPlainText();
        bool ok;
        quint16 port = portString.toUShort(&ok);
        socket->connectToHost(ip, port);
    }
}

void TCPApp::StartServer() {
    if (server == nullptr) {
        server = new QTcpServer(this);
        connect(server, &QTcpServer::newConnection, this, &TCPApp::HandleNewConnection);
    
        QString portString = ui->portText->toPlainText();
        bool ok;
        quint16 port = portString.toUShort(&ok);
        if (!ok) {
            ui->messagesText->append("<span style='color:red'>Invalid port.</span>");
            return;
        }
    
        if (server->listen(QHostAddress::Any, port)) {
            ui->messagesText->append("<span style='color:orange'>Server started on port: " + QString::number(port) + "</span>");
        }
        else {
            ui->messagesText->append("<span style='color:red'>Failed to start server: " + server->errorString() + "</span>");
        }
    }
}

void TCPApp::HandleNewConnection() {
    QTcpSocket *client = server->nextPendingConnection();
    clients.append(client);

    connect(client, &QTcpSocket::readyRead, this, &TCPApp::ReadMessage);
    connect(client, &QTcpSocket::disconnected, this, [=]() {
        clients.removeOne(client);
        client->deleteLater();
        ui->messagesText->append("<span style='color:orange'>Client disconnected.</span>");
    });
    ui->messagesText->append("<span style='color:orange'>Client connected: " + client->peerAddress().toString() + "</span>");
}

void TCPApp::ReadMessage() {
    QTcpSocket* senderSocket = qobject_cast<QTcpSocket*>(sender());
    if (!senderSocket) return;

    QByteArray data = senderSocket->readAll();

    if (receivingFile && receivingFile->isOpen()) {
        bytesReceived += data.size();
        receivingFile->write(data);

        if (bytesReceived >= totalBytes) {
            receivingFile->close();

            if (receivingFile->open(QIODevice::ReadOnly)) {
                QByteArray fileData = receivingFile->readAll();
                receivingFile->close();
                SaveReceivedFile(receivingFile->fileName(), fileData);
            }

            receivingFile->remove();
            delete receivingFile;
            receivingFile = nullptr;
        }

        if (server && senderSocket != socket) {
            for (QTcpSocket* client : clients) {
                if (client != senderSocket && client->state() == QAbstractSocket::ConnectedState) {
                    client->write(data);
                }
            }
        }
        return;
    }

    if (data.startsWith("FILE")) {
        int newlineIndex = data.indexOf("\n");
        if (newlineIndex != -1) {
            QByteArray headerData = data.left(newlineIndex);
            QByteArray remainingData = data.mid(newlineIndex + 1);

            QString msg = QString::fromUtf8(headerData);
            QStringList parts = msg.split(' ');

            if (parts.size() >= 3) {
                QString fileName = parts[1];
                qint64 fileSize = parts.last().toLongLong();

                receivingFile = new QFile(fileName);
                if (!receivingFile->open(QIODevice::WriteOnly)) {
                    ui->messagesText->append("<span style='color:red'>Error: Save File</span>");
                    return;
                }

                totalBytes = fileSize;
                bytesReceived = 0;
                ui->messagesText->append("<span style='color:green'>Downloading " + fileName + " file...</span>");

                if (remainingData.size() > 0) {
                    bytesReceived += remainingData.size();
                    receivingFile->write(remainingData);

                    if (bytesReceived >= totalBytes) {
                        receivingFile->close();

                        if (receivingFile->open(QIODevice::ReadOnly)) {
                            QByteArray fileData = receivingFile->readAll();
                            receivingFile->close();
                            SaveReceivedFile(receivingFile->fileName(), fileData);
                        }

                        receivingFile->remove();
                        delete receivingFile;
                        receivingFile = nullptr;
                    }
                }
            }
        }
    }
    else {
        QString msg = QString::fromUtf8(data);
        ui->messagesText->append("<span style='color:red'>" + senderSocket->peerAddress().toString() + "</span>: " + msg);
    }

    if (server && senderSocket != socket) {
        for (QTcpSocket* client : clients) {
            if (client != senderSocket && client->state() == QAbstractSocket::ConnectedState) {
                client->write(data);
            }
        }
    }
}

//void TCPApp::ReadMessage() {
//    QTcpSocket* senderSocket = qobject_cast<QTcpSocket*>(sender());
//    if (senderSocket) {
//        QByteArray data = senderSocket->readAll();
//        QString msg = QString::fromUtf8(data);
//
//        if (msg.startsWith("FILE")) {
//            QStringList parts = msg.split(' ');
//            if (parts.size() == 3) {
//                QString fileName = parts[1];
//                qint64 fileSize = parts[2].toLongLong();
//
//                receivingFile = new QFile(fileName);
//                if (!receivingFile->open(QIODevice::WriteOnly)) {
//                    ui->messagesText->append("<span style='color:red'>Error: Save File</span>");
//                    return;
//                }
//
//                totalBytes = fileSize;
//                bytesReceived = 0;
//                ui->messagesText->append("<span style='color:green'>Downloading " + fileName + " file...</span>");
//            }
//        }
//        else {
//            if (receivingFile) {
//                bytesReceived += data.size();
//                receivingFile->write(data);
//
//                if (bytesReceived == totalBytes) {
//                    receivingFile->close();
//
//                    if (receivingFile->open(QIODevice::ReadOnly)) {
//                        QByteArray fileData = receivingFile->readAll();
//                        receivingFile->close();
//                        SaveReceivedFile(receivingFile->fileName(), fileData);
//                    }
//
//                    receivingFile->remove();
//                    delete receivingFile;
//
//                    receivingFile = nullptr;
//
//                    ui->messagesText->append("<span style='color:green'>" + senderSocket->peerAddress().toString() + "File received successfully.</span>: " + msg);
//                }
//            }
//            else {
//                ui->messagesText->append("<span style='color:red'>" + senderSocket->peerAddress().toString() + "</span>: " + msg);
//            }
//        }
//
//        //if (receivingFile && bytesReceived == totalBytes) {
//        //    QByteArray fileData = receivingFile->readAll();
//        //    receivingFile->close();
//        //    SaveReceivedFile(receivingFile->fileName(), fileData);
//        //    receivingFile = nullptr;
//        //}
//    
//        if (server && senderSocket != socket) {
//            for (QTcpSocket* client : clients) {
//                if (client != senderSocket && client->state() == QAbstractSocket::ConnectedState) {
//                    client->write(data);
//                }
//            }
//        }
//    }
//}

void TCPApp::SendFile() {
    QString filePath = QFileDialog::getOpenFileName(this, "Select the file to send");
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        ui->messagesText->append("<span style='color:red'>Error: Open File</span>");
        return;
    }

    QString filename = QFileInfo(filePath).fileName();
    QByteArray fileData = file.readAll();
    QByteArray header = QString("FILE %1 %2\n").arg(filename).arg(fileData.size()).toUtf8();

    socket->write(header);
    socket->flush();

    QThread::msleep(100);

    socket->write(fileData);
    socket->flush();
    ui->messagesText->append("<span style='color:green'>File " + filename + " sent.</span>");
}

void TCPApp::SaveReceivedFile(const QString& filename, const QByteArray& filedata) {
    QString savePath = QFileDialog::getSaveFileName(this, "Save file", filename);
    if (savePath.isEmpty()) return;

    QFile file(savePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(filedata);
        file.close();
        ui->messagesText->append("<span style='color:green'>File " + filename + " saved.</span>");
    }
    else {
        ui->messagesText->append("<span style='color:red'>Error: File " + filename + " not saved.</span>");
    }
}