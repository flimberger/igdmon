import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.0

import Graph 1.0

Item {
//    title: "Fritz Graph"
    height: 480
    width: 640
    visible: true;

    ColumnLayout {
        id: mainLayout

        anchors.fill: parent
        spacing: 0

        Graph {
            id: downstream
            model: downstreamData

            color: "#ff9900"
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
        Graph {
            id: upstream
            model: upstreamData

            color: "#9900ff"
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
