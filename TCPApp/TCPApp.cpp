#include "TCPApp.h"
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
    if (senderSocket) {
        QByteArray data = senderSocket->readAll();
        QString msg = QString::fromUtf8(data);
        ui->messagesText->append("<span style='color:red'>" + senderSocket->peerAddress().toString() + "</span>: " + msg);
    
        if (server && senderSocket != socket) {
            for (QTcpSocket* client : clients) {
                if (client != senderSocket && client->state() == QAbstractSocket::ConnectedState) {
                    client->write(data);
                }
            }
        }
    }
}