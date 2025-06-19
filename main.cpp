#include "mainwindow.h"

#include <QApplication>
#include<QToolTip>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // set organizationName
    QCoreApplication::setOrganizationName("abc");
    QCoreApplication::setApplicationName("def");

    // Tooltip style
    QToolTip::setFont(QFont("Consolas", 11));

    qApp->setStyleSheet(R"(
    QToolTip {
        background-color: #2d2d30;
        color: #ffffff;
        border: 1px solid #555;
        padding: 6px;
        font-family: Consolas;
        font-size: 11pt;
    }
)");

    MainWindow w;
    w.show();
    a.setWindowIcon(QIcon(":/mainIcon.png"));
    w.setWindowTitle("CodeNova");
    return a.exec();
}
