#include "message.h"

Message::Message(int id, int senderId, int receiverId, const QString& content, const QDateTime& timestamp)
    : id(id), senderId(senderId), receiverId(receiverId), content(content), timestamp(timestamp) {
    // конструктор сообщения с параметрами
}

int Message::getId() const {
    return id; // возвращаем ID сообщения
}

int Message::getSenderId() const {
    return senderId; // возвращаем ID отправителя
}

int Message::getReceiverId() const {
    return receiverId; // возвращаем ID получателя
}

QString Message::getContent() const {
    return content; // возвращаем текст сообщения
}

QDateTime Message::getTimestamp() const {
    return timestamp; // возвращаем временную метку
}