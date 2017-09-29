import QtQuick 2.0
import ImageItem 1.0
import QtQuick.Controls 1.2
import QtQuick.Dialogs 1.2
import QtQuick.Window 2.2
Rectangle {
    id: image_display
    color: "gray"
//    width: 640
//    height: 480

    ImageItem {
        id: image_item
//        anchors.fill: parent
        anchors.centerIn: parent
        focus: true;
//        rotation: 90
        width: 640
        height: 480
        Connections{
            target: ir_view
//            ignoreUnknownSignals: true
            onImage_changed:{
                image_item.img_trigged_slot( puc_out_buf, n_data_len );
            }
        }

    }
    Text {
        id: interval_time
        text: qsTr("FR")
        anchors.left: parent.left
        anchors.leftMargin: 60
        anchors.top: parent.top
        anchors.topMargin: 60
        width: 60
        height: 60
        Connections {
            target: image_encode
            onShow_frame_rate_info: {
                interval_time.text = s_frame_rate
            }
        }

    }
//    Rectangle {
//        id: main_btn
//        anchors.left: parent.left
//        anchors.leftMargin: 60
//        anchors.top: interval_time.bottom
//        anchors.topMargin: 20
//        width: 70
//        height: 70
//        radius: 12
//        border.width: 0.5
//        color: "transparent"
//        MouseArea {
//            anchors.fill: parent
//            onClicked: {
//                console.log("main_btn clicked")
//                menu_list.popup()
//            }
//        }
//    }
//    Text {
//        id: interval_time
////        text: qsTr("FR")
//        rotation: 90
//        anchors.right: parent.right
//        anchors.rightMargin: 60
//        anchors.top: parent.top
//        anchors.topMargin: 60
//        width: 60
//        height: 60
//        Connections {
//            target: image_item
//            onShow_frame_rate_info: {
//                interval_time.text = s_frame_rate
//            }
//        }

//    }
//    Rectangle {
//        id: main_btn
//        anchors.right: interval_time.left
//        anchors.rightMargin: 30
//        anchors.top: parent.top
//        anchors.topMargin: 60
//        width: 70
//        height: 70
//        radius: 12
//        border.width: 0.5
//        color: "transparent"
//        MouseArea {
//            anchors.fill: parent
//            onClicked: {
//                console.log("main_btn clicked")
//                menu_list.popup()
//            }
//        }
//    }
//    Menu {
//        id: menu_list

//        MenuItem {
//            id: unlink_net
////            Rotation:90
//            text: qsTr("断开连接")
//            onTriggered: {
//                wifi_client.disconnect_current_server( )
//                link_btn.b_unlinked_status = true
//                link_txt.text = qsTr("连  接")
//                image_display.visible = false
//            }
//        }
//    }

}
