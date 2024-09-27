#include "database.h"
#include <QCryptographicHash>

Database::Database() {
    db = QSqlDatabase::addDatabase("QPSQL"); // используем драйвер PostgreSQL
    db.setHostName("localhost");             // хост базы данных (не меняем)
    db.setDatabaseName("chat");              // имя базы данных в PostgreSQL
    db.setUserName("login");                 // имя пользователя БД (имя пользователя в PostgreSQL)
    db.setPassword("pass");                  // не забудьте заменить на свой пароль (пароль от пользователя в PGSQL)
}

bool Database::connect() {
    return db.open(); // открываем соединение с базой данных
}

void Database::disconnect() {
    db.close(); // закрываем соединение
}

QList<User> Database::getAllUsers() {
    QList<User> users;
    QSqlQuery query("SELECT id, login, name, is_banned FROM users"); // запрос всех пользователей

    while (query.next()) { // перебираем результаты
        int id = query.value(0).toInt();
        QString login = query.value(1).toString();
        QString name = query.value(2).toString();
        bool isBanned = query.value(3).toBool();

        users.append(User(id, login, name, isBanned)); // добавляем в список
    }

    return users; // возвращаем список пользователей
}

void Database::banUser(int userId) {
    QSqlQuery query;
    query.prepare("UPDATE users SET is_banned = TRUE WHERE id = :id"); // готовим запрос на бан
    query.bindValue(":id", userId);
    query.exec(); // выполняем
}

void Database::unbanUser(int userId) {
    QSqlQuery query;
    query.prepare("UPDATE users SET is_banned = FALSE WHERE id = :id"); // готовим запрос на разбан
    query.bindValue(":id", userId);
    query.exec(); // выполняем
}

bool Database::registerUser(const QString& login, const QString& password, const QString& name) {
    QSqlQuery query;
    query.prepare("INSERT INTO users (login, password, name) VALUES (:login, :password, :name)");
    query.bindValue(":login", login);
    query.bindValue(":password", QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex()); // хэшируем пароль
    query.bindValue(":name", name);
    return query.exec(); // выполняем и возвращаем результат
}

User Database::authenticateUser(const QString& login, const QString& password) {
    QSqlQuery query;
    query.prepare("SELECT id, name, is_banned FROM users WHERE login = :login AND password = :password");
    query.bindValue(":login", login);
    query.bindValue(":password", QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex()); // хэшируем пароль
    query.exec();

    if (query.next()) { // если нашли пользователя
        int id = query.value(0).toInt();
        QString name = query.value(1).toString();
        bool isBanned = query.value(2).toBool();
        return User(id, login, name, isBanned); // возвращаем его
    }
    else {
        return User(); // иначе возвращаем пустого
    }
}

User Database::getUserById(int userId) {
    QSqlQuery query;
    query.prepare("SELECT id, login, name, is_banned FROM users WHERE id = :id");
    query.bindValue(":id", userId);
    query.exec();

    if (query.next()) {
        int id = query.value("id").toInt();
        QString login = query.value("login").toString();
        QString name = query.value("name").toString();
        bool isBanned = query.value("is_banned").toBool();
        return User(id, login, name, isBanned);
    }
    else {
        return User();
    }
}

void Database::saveMessage(int senderId, int receiverId, const QString& content) {
    QSqlQuery query;
    query.prepare("INSERT INTO messages (sender_id, receiver_id, content, is_read) VALUES (:sender_id, :receiver_id, :content, FALSE)");
    query.bindValue(":sender_id", senderId);
    query.bindValue(":receiver_id", receiverId == 0 ? QVariant() : receiverId); // если receiverId == 0, сохраняем NULL
    query.bindValue(":content", content);
    query.exec();
}

QList<Message> Database::getUnreadMessagesForUser(int userId) {
    QList<Message> messages;
    QSqlQuery query;
    query.prepare("SELECT id, sender_id, receiver_id, content, timestamp FROM messages WHERE receiver_id = :receiver_id AND is_read = FALSE");
    query.bindValue(":receiver_id", userId);
    query.exec();

    while (query.next()) {
        int id = query.value("id").toInt();
        int senderId = query.value("sender_id").toInt();
        int receiverId = query.value("receiver_id").toInt();
        QString content = query.value("content").toString();
        QDateTime timestamp = query.value("timestamp").toDateTime();

        messages.append(Message(id, senderId, receiverId, content, timestamp)); // добавляем в список
    }

    return messages; // возвращаем список непрочитанных сообщений
}

QList<Message> Database::getAllMessages() {
    QList<Message> messages;
    QSqlQuery query("SELECT id, sender_id, receiver_id, content, timestamp FROM messages ORDER BY timestamp ASC");

    while (query.next()) {
        int id = query.value("id").toInt();
        int senderId = query.value("sender_id").toInt();
        int receiverId = query.value("receiver_id").isNull() ? 0 : query.value("receiver_id").toInt();
        QString content = query.value("content").toString();
        QDateTime timestamp = query.value("timestamp").toDateTime();

        messages.append(Message(id, senderId, receiverId, content, timestamp)); // добавляем в список
    }

    return messages; // возвращаем все сообщения
}

void Database::markMessagesAsRead(int userId) {
    QSqlQuery query;
    query.prepare("UPDATE messages SET is_read = TRUE WHERE receiver_id = :receiver_id AND is_read = FALSE");
    query.bindValue(":receiver_id", userId);
    query.exec(); // отмечаем сообщения как прочитанные
}