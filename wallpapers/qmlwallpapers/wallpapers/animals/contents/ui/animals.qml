import QtQuick 1.1

Rectangle
{
    id: root
    Animal {
        id: anim
        function move() {
            anim.x=Math.min(Math.random()*root.width, root.width-width)
            anim.y=Math.min(Math.random()*root.height, root.height-height)
        }
    }
    
    function randomColor() {
        root.color = Qt.hsla(Math.random(), 1,0.5,1)
    }
    
    function randomAction() {
        var a = [anim.move, anim.fetchAnimal, randomColor]
        var i = Math.floor(Math.random()*a.length)
        a[i]()
    }
    
    Timer {
        running: true
        repeat: true
        interval: 1000*600 //10 minutes
        onTriggered: randomAction()
    }
    
    Component.onCompleted: {
        anim.fetchAnimal()
        anim.move()
        randomColor()
    }
    
    MouseArea {
        anchors.fill: parent
        onClicked: randomAction()
    }

    Behavior on color { ColorAnimation { duration: 1000; easing.type: Easing.InQuad } }
}