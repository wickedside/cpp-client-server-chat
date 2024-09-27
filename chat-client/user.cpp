#include "user.h"

User::User() : id(0), banned(false) {
    // конструктор по умолчанию, ничего особенного
}

User::User(int id, const QString& login, const QString& name, bool isBanned)
    : id(id), login(login), name(name), banned(isBanned) {
    // конструктор с параметрами
}

int User::getId() const {
    return id; // возвращаем ID пользователя
}

QString User::getLogin() const {
    return login; // возвращаем логин
}

QString User::getName() const {
    return name; // возвращаем имя
}

bool User::isBanned() const {
    return banned; // проверяем, забанен ли пользователь
}