#ifndef DATABASE_H
#define DATABASE_H

#include <QtSql>
#include "user.h"
#include "message.h"

class Database {
public:
    Database(); // конструктор
    bool connect(); // подключение к базе данных
    void disconnect(); // отключение

    QList<User> getAllUsers(); // получить всех пользователей
    void banUser(int userId); // забанить пользователя
    void unbanUser(int userId); // разбанить пользователя

    bool registerUser(const QString& login, const QString& password, const QString& name); // регистрация пользователя
    User authenticateUser(const QString& login, const QString& password); // аутентификация пользователя
    User getUserById(int userId); // получить пользователя по ID
    void saveMessage(int senderId, int receiverId, const QString& content); // сохранить сообщение

    QList<Message> getUnreadMessagesForUser(int userId); // получить непрочитанные сообщения для пользователя
    QList<Message> getAllMessages(); // получить все сообщения
    void markMessagesAsRead(int userId); // отметить сообщения как прочитанные

private:
    QSqlDatabase db; // объект базы данных
};

#endif // DATABASE_H