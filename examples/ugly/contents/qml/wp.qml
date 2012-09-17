import QtQuick 1.1

Rectangle
{
    gradient: Gradient {
         GradientStop { position: 0.0; color: "red" }
         GradientStop { position: 1.0; color: "blue" }
     }
    
    Timer {
         interval: 1000; running: true; repeat: true
         onTriggered: time.text = Date().toString()
     }

     Text { anchors.centerIn: parent; id: time }
}