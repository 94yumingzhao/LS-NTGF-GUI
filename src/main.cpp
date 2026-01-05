// main.cpp - Qt6 GUI Application Entry Point
// Production Planning Optimizer GUI

#include <QApplication>
#include <QStyleFactory>
#include "main_window.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Application metadata
    QApplication::setApplicationName("LS-NTGF-RR Optimizer");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("YM-Code");

    // Use Fusion style for consistent look
    app.setStyle(QStyleFactory::create("Fusion"));

    // Create and show main window
    MainWindow window;
    window.show();

    return app.exec();
}
