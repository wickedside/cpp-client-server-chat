#ifndef CLIENT_H
#define CLIENT_H

#include "user.h"
#include "message.h"
#include <QTcpSocket>
#include <string>
#include <QObject>

class Client : public QObject {
    Q_OBJECT

public:
    Client();      // конструктор клиента
    void run();    // запускаем клиента

private:
    QTcpSocket* socket; // сокет для связи с сервером
    User currentUser;   // текущий пользователь

    void registerUser();       // метод регистрации
    void loginUser();          // метод входа
    void sendMessage();        // отправка сообщения всем
    void sendPrivateMessage(); // отправка личного сообщения
    void mainMenu();           // главное меню

    void connectToServer();    // подключение к серверу

private slots:
    void processServerResponse(); // обработка ответа от сервера
};

#endif // CLIENT_H