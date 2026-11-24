#include "MainWindow.hpp"
#include <QApplication>
#include <QCoreApplication>
#include <QTimer>
#include <QString>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("SimTrrafic Qt");
    app.setOrganizationName("SCTUII");
    MainWindow window;
    window.show();
    // Opcion util para CI: comprueba que la ventana se crea y que Qt inicia.
    if (app.arguments().contains("--smoke-test")) {
        QTimer::singleShot(200, &app, [] { QCoreApplication::quit(); });
    }
    return app.exec();
}
