#ifndef SERVER_H
#define SERVER_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTextEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QTabWidget>
#include <QMap>

#include "database.h"
#include "user.h"
#include "message.h"

class Server : public QWidget {
    Q_OBJECT

public:
    Server(QWidget* parent = nullptr); // конструктор сервера
    ~Server(); // деструктор

private slots:
    void newConnection();      // обработка нового подключения
    void readData();           // чтение данных от клиента
    void clientDisconnected(); // обработка отключения клиента
    void banUser();            // забанить пользователя
    void unbanUser();          // разбанить пользователя

private:
    void setupUI();            // настройка интерфейса
    void loadUsers();          // загрузка списка пользователей
    void loadMessageHistory(); // загрузка истории сообщений

    QTcpServer* tcpServer;             // TCP сервер
    QMap<int, QTcpSocket*> clients;    // мапа clientId -> QTcpSocket*
    QMap<int, QTcpSocket*> userSockets; // мапа userId -> QTcpSocket*

    Database db; // база данных

    // элементы интерфейса
    QTabWidget* tabWidget;

    // первая вкладка - чаты
    QWidget* chatsTab;
    QTextEdit* publicChatArea;
    QTextEdit* privateChatArea;

    // вторая вкладка - пользователи
    QWidget* usersTab;
    QTableWidget* userTable;
    QPushButton* banButton;
    QPushButton* unbanButton;

    // третья вкладка - лог
    QWidget* logTab;
    QTextEdit* logArea;
};

#endif // SERVER_H