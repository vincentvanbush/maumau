import QtQuick 2.0


//Rectangle {
//    width: 100
//    height: 100
//    color: "red"

Item {
    //property alias loader1: loader1
    //property alias loader2: loader2

    Component {
        id: square

        Rectangle {
            id: rect
            x: 0
            y: 30
            width: 100
            height: 145

//            //color: "#00ff00"

            Image {
                id: image1
                x: 0
                y: 0
                width: 100
                height: 140
                //source: "qrc:/qtquickplugin/images/template_image.png"
                source: "images/ace_of_spades.png"
            }
            states: [
                State {
                    name: "moved"

                    PropertyChanges {
                        target: image1
                        x: 0
                        y: 0
                    }

                    PropertyChanges {
                        target: rect
                        y: 0
                        height: 145
                    }
                }
            ]
            MouseArea {
                anchors.fill: parent
                onClicked: rect.state = 'moved'
            }

            transitions:  [
                Transition {

                    NumberAnimation {
                        properties: "x,y"
                        duration: 500
                    }
                }

            ]

        }

    }

    Loader { id: loader1; sourceComponent: square;}
    //Loader { id: loader2; sourceComponent: square; x: 100 }

}


