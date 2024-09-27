#ifndef USER_H
#define USER_H

#include <QString>

class User {
public:
    User(); // конструктор по умолчанию
    User(int id, const QString& login, const QString& name, bool isBanned); // конструктор с параметрами

    int getId() const;          // получить ID пользователя
    QString getLogin() const;   // получить логин
    QString getName() const;    // получить имя
    bool isBanned() const;      // проверка бана

private:
    int id;            // ID пользователя
    QString login;     // логин
    QString name;      // имя
    bool banned;       // статус бана
};

#endif // USER_H