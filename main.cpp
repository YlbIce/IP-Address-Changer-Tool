#include <QApplication>
#include <QTextCodec>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set codec for Chinese character support
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
#endif

    app.setApplicationName("ChangeIPTool");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("IPTool");

    MainWindow window;
    window.resize(600, 500);
    window.show();

    return app.exec();
}
