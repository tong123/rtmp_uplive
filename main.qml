import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2

Window {
    title: qsTr("Hello World")
    width: 800
    height: 480
    visible: true
    ImageDisplay {
        anchors.fill: parent
    }

}
