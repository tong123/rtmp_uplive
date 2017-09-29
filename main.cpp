#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "ir-view.h"
#include "image-item.h"
#include "image-encode.h"
#include "image-send.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;
    IrView *p_ir_view = new IrView;
    ImageEncode *p_image_encode = new ImageEncode(p_ir_view);
    ImageSend *p_image_send = new ImageSend( p_image_encode );
    qmlRegisterType<CImageItem>( "ImageItem", 1, 0, "ImageItem" );
    engine.rootContext()->setContextProperty( "ir_view", p_ir_view );
    engine.rootContext()->setContextProperty( "image_encode", p_image_encode );
//    engine.rootContext()->setContextProperty( "image_send", p_image_send );

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
