#include <QApplication>
#include "server.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv); // создаём приложение Qt

    Server server; // создаём сервер
    server.show(); // показываем его окно

    return app.exec(); // запускаем главный цикл приложения
}