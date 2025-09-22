#include <QApplication>
#include "SimpleVideoEditor.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    SimpleVideoEditor editor;
    editor.show();
    return app.exec();
}