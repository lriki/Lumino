
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtQuick/QQuickView>

#include "Squircle.h"

class ViewModel : public QObject
{
    Q_OBJECT
public:
    //Q_INVOKABLE QDateTime getCurrentDateTime() const {
    //    return QDateTime::currentDateTime();
    //}

    Q_INVOKABLE void clicked()
    {
        printf("\n");
    }
};

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<Squircle>("OpenGLUnderQML", 1, 0, "Squircle");

    //QCoreApplication::addLibraryPath("./");

    //QQmlApplicationEngine engine;
    //engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    ////QObject* obj = engine.rootObjects().front();
    //
    //view.setResizeMode(QQuickView::SizeRootObjectToView);
    //ViewModel vm;
    //engine.rootContext()->setContextProperty("vm", &vm);


    QQuickView view;
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.setSource(QUrl("qrc:/main.qml"));
    view.show();

	    
    return app.exec();
}

#include "main.moc"

