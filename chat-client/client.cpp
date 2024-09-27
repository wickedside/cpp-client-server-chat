#include "client.h"
#include <iostream>
#include <thread>
#include <QDataStream>
#include <QCoreApplication>

Client::Client() {
    socket = new QTcpSocket(this); // создаём новый TCP сокет
    connect(socket, &QTcpSocket::readyRead, this, &Client::processServerResponse); // связываем сигнал readyRead с нашим слотом
}

void Client::connectToServer() {
    socket->connectToHost("127.0.0.1", 1234); // пытаемся подключиться к серверу на localhost и порту 1234
    if (!socket->waitForConnected(3000)) {    // ждём до 3 секунд
        std::cerr << "Failed to connect to the server" << std::endl; // если не получилось, ругаемся
        exit(1); // и выходим
    }
}

void Client::run() {
    connectToServer(); // подключаемся к серверу

    int choice = 0; // переменная для выбора в меню
    while (choice != 3) { // пока не выбрали выход
        std::cout << "1. Register\n2. Login\n3. Exit\nSelect an option: ";
        std::cin >> choice; // считываем выбор

        switch (choice) {
        case 1:
            registerUser(); // регистрация нового пользователя
            break;
        case 2:
            loginUser(); // вход под существующим аккаунтом
            break;
        case 3:
            std::cout << "Exiting the program" << std::endl; // выходим из программы
            socket->disconnectFromHost(); // отключаемся от сервера
            break;
        default:
            std::cout << "Invalid choice, please try again" << std::endl; // если ввели что-то не то
            break;
        }
    }
}

void Client::registerUser() {
    std::string login, password, name;
    std::cout << "Enter login: ";
    std::cin >> login; // вводим логин
    std::cout << "Enter password: ";
    std::cin >> password; // вводим пароль
    std::cout << "Enter name: ";
    std::cin >> name; // вводим имя

    QDataStream out(socket); // создаём поток данных для отправки
    out.setVersion(QDataStream::Qt_5_15); // устанавливаем версию потока
    out << QString("REGISTER") << QString::fromStdString(login) << QString::fromStdString(password) << QString::fromStdString(name); // отправляем команду регистрации и данные

    if (!socket->waitForReadyRead(3000)) { // ждём ответ от сервера
        std::cerr << "No response from the server" << std::endl; // если не дождались, выводим ошибку
        return;
    }

    // обрабатываем ответ сервера
    processServerResponse();
}

void Client::loginUser() {
    std::string login, password;
    std::cout << "Enter login: ";
    std::cin >> login; // вводим логин
    std::cout << "Enter password: ";
    std::cin >> password; // вводим пароль

    QDataStream out(socket);
    out.setVersion(QDataStream::Qt_5_15);
    out << QString("LOGIN") << QString::fromStdString(login) << QString::fromStdString(password); // отправляем команду входа и данные

    if (!socket->waitForReadyRead(3000)) { // ждём ответ от сервера
        std::cerr << "No response from the server" << std::endl; // если не дождались, выводим ошибку
        return;
    }

    // обрабатываем ответ сервера
    processServerResponse();
    // mainMenu() вызывается внутри processServerResponse(), если вход успешен
}

void Client::mainMenu() {
    int choice = 0;

    while (choice != 4) { // пока не выбрали выход из меню
        // проверяем, есть ли входящие сообщения
        if (socket->waitForReadyRead(100)) {
            processServerResponse(); // обрабатываем их
        }

        std::cout << "\n1. Send message to all\n2. Send private message\n3. Refresh messages\n4. Logout\nSelect an option: ";
        std::cin >> choice; // считываем выбор

        switch (choice) {
        case 1:
            sendMessage(); // отправляем сообщение всем
            break;
        case 2:
            sendPrivateMessage(); // отправляем личное сообщение
            break;
        case 3:
            // обновляем сообщения
            while (socket->bytesAvailable()) {
                processServerResponse();
            }
            break;
        case 4:
            std::cout << "Logging out" << std::endl; // выходим из аккаунта
            currentUser = User(); // сбрасываем текущего пользователя
            return; // выходим из меню
        default:
            std::cout << "Invalid choice, please try again" << std::endl; // если ввели что-то не то
            break;
        }
    }
}

void Client::sendMessage() {
    std::string content;
    std::cout << "Enter message: ";
    std::cin.ignore(); // чистим буфер
    std::getline(std::cin, content); // считываем сообщение

    QDataStream out(socket);
    out.setVersion(QDataStream::Qt_5_15);
    out << QString("MESSAGE") << currentUser.getId() << 0 << QString::fromStdString(content); // отправляем команду сообщения всем

    // можно обработать немедленные ответы
    if (socket->waitForReadyRead(300)) {
        processServerResponse();
    }
}

void Client::sendPrivateMessage() {
    int receiverId;
    std::string content;
    std::cout << "Enter recipient ID: ";
    std::cin >> receiverId; // вводим ID получателя
    std::cout << "Enter message: ";
    std::cin.ignore(); // чистим буфер
    std::getline(std::cin, content); // считываем сообщение

    QDataStream out(socket);
    out.setVersion(QDataStream::Qt_5_15);
    out << QString("MESSAGE") << currentUser.getId() << receiverId << QString::fromStdString(content); // отправляем команду личного сообщения

    // можно обработать немедленные ответы
    if (socket->waitForReadyRead(300)) {
        processServerResponse();
    }
}

void Client::processServerResponse() {
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_15);

    bool loginSuccess = false; // флаг успешного входа
    bool endOfMessages = false; // флаг окончания сообщений

    while (!endOfMessages) {
        while (socket->bytesAvailable()) {
            QString response;
            in >> response; // читаем ответ от сервера
            // qDebug() << "Received response:" << response; // можно раскомментировать для отладки

            if (response == "REGISTER_SUCCESS") {
                std::cout << "Registration successful" << std::endl; // регистрация успешна
            }
            else if (response == "REGISTER_FAIL") {
                std::cout << "Registration failed" << std::endl; // регистрация не удалась
            }
            else if (response == "LOGIN_SUCCESS") {
                int userId;
                QString name;
                in >> userId >> name; // получаем данные пользователя
                currentUser = User(userId, "", name, false); // сохраняем текущего пользователя
                std::cout << "Login successful, welcome, " << name.toUtf8().constData() << std::endl;
                loginSuccess = true; // вход успешен
            }
            else if (response == "LOGIN_FAIL") {
                std::cout << "Invalid login or password" << std::endl; // неверный логин или пароль
            }
            else if (response == "LOGIN_BANNED") {
                std::cout << "You are banned on the server" << std::endl; // вы забанены на сервере
            }
            else if (response == "USER_BANNED") {
                std::cout << "You have been banned from the server" << std::endl; // вас забанили
                socket->disconnectFromHost(); // отключаемся
                return;
            }
            else if (response == "NEW_MESSAGE") {
                int senderId, receiverId;
                QString content;
                in >> senderId >> receiverId >> content; // получаем данные сообщения
                std::cout << "\n[Message from " << senderId << "]: " << content.toUtf8().constData() << std::endl; // выводим сообщение
            }
            else if (response == "END_OF_MESSAGES") {
                // получили сигнал окончания сообщений
                endOfMessages = true;
                break; // выходим из внутреннего цикла
            }
            else {
                std::cout << "Unknown response from server: " << response.toUtf8().constData() << std::endl; // неизвестный ответ
                in.skipRawData(socket->bytesAvailable()); // пропускаем оставшиеся данные
            }
        }

        if (endOfMessages) {
            break; // выходим из внешнего цикла
        }

        if (!socket->waitForReadyRead(500)) {
            break; // если нет больше данных, выходим
        }
    }

    // если вход успешен, переходим в главное меню
    if (loginSuccess) {
        mainMenu();
    }
}
