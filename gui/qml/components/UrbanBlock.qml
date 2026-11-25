import QtQuick

// Entorno estático. No añade agentes ni vehículos a la simulación.
Rectangle { antialiasing: true;
    id: block
    required property int kind
    color: "#c8cfbf"; radius: 8
    border.color: "#aeb8ad"; border.width: 1
    clip: true
    component Tree: Item {
        width: 32; height: 32
        Rectangle { antialiasing: true; x: 4; y: 5; width: 29; height: 29; radius: 15; color: "#204033"; opacity: 0.12 }
        Rectangle { antialiasing: true; x: 1; y: 1; width: 28; height: 28; radius: 14; color: "#6c8668" }
        Rectangle { antialiasing: true; x: 3; y: 2; width: 22; height: 22; radius: 11; color: "#829a76" }
        Rectangle { antialiasing: true; x: 6; y: 4; width: 12; height: 12; radius: 7; color: "#94aa85" }
    }
    component Building: Rectangle { antialiasing: true;
        property color roof: "#bcc2bc"
        color: "#a5afa7"; radius: 2
        Rectangle { antialiasing: true; x: 5; y: 6; width: parent.width; height: parent.height; color: "#42584b"; opacity: 0.12; z: -1 }
        Rectangle { antialiasing: true; anchors.fill: parent; anchors.margins: 4; color: parent.roof; border.color: "#d9ddd5" }
        Rectangle { antialiasing: true; x: 12; y: 12; width: parent.width-24; height: parent.height-24; color: "transparent"; border.color: "#aeb6ad" }
        Rectangle { antialiasing: true; x: parent.width-36; y: 15; width: 20; height: 13; color: "#89978f"; border.color: "#a5b0a7"
            Rectangle { antialiasing: true; x: 3; y: 3; width: 14; height: 2; color: "#bfc8bf" }
            Rectangle { antialiasing: true; x: 3; y: 7; width: 14; height: 2; color: "#bfc8bf" }
        }
        Repeater { model: Math.max(1,Math.floor((parent.width-20)/25)); Rectangle { antialiasing: true; required property int index; x: 12+index*25; y: parent.height-5; width: 12; height: 3; color: "#75837c" } }
    }
    Rectangle { antialiasing: true;
        id: inner
        anchors.fill: parent; anchors.margins: 12; radius: 5
        color: block.kind===2 ? "#d7d5c6" : "#b6c6a8"
        border.color: block.kind===2 ? "#c2c5b7" : "#adbea1"
        // Comercio y estacionamiento vacío.
        Item {
            anchors.fill: parent; visible: block.kind===0
            Building { x: 16; y: 15; width: Math.max(75,parent.width*0.56); height: Math.max(55,parent.height*0.50); roof: "#b8bbae" }
            Building { x: parent.width*0.66; y: 15; width: Math.max(55,parent.width*0.29-10); height: Math.max(55,parent.height*0.50); roof: "#c2c9c3" }
            Rectangle { antialiasing: true;
                x: 18; y: parent.height-58; width: parent.width-72; height: 40; color: "#87958c"; radius: 2
                Repeater { model: Math.max(1,Math.floor((parent.width-16)/25)); Rectangle { antialiasing: true; required property int index; x: 10+index*25; y: 5; width: 2; height: 24; color: "#cbd1be" } }
                Rectangle { antialiasing: true; x: 10; y: 29; height: 2; width: parent.width-20; color: "#cbd1be" }
            }
            Tree { x: parent.width-42; y: parent.height-49 }
        }
        // Parque: sendero, arbolado y una pequeña fuente.
        Item {
            anchors.fill: parent; visible: block.kind===1
            Rectangle { antialiasing: true; x: 0; y: parent.height*0.52-8; width: parent.width; height: 16; color: "#d7d2ba" }
            Rectangle { antialiasing: true; x: parent.width*0.55-8; y: 0; width: 16; height: parent.height; color: "#d7d2ba" }
            Rectangle { antialiasing: true;
                x: parent.width*0.55-30; y: parent.height*0.52-30; width: 60; height: 60; radius: 30; color: "#dddbcc"; border.color: "#bbc2b1"
                Rectangle { antialiasing: true; anchors.centerIn: parent; width: 43; height: 43; radius: 22; color: "#83a7a3"; border.color: "#739790" }
                Rectangle { antialiasing: true; anchors.centerIn: parent; width: 12; height: 12; radius: 6; color: "#c4d8cb" }
            }
            Repeater {
                model: [[0.08,0.12],[0.30,0.20],[0.68,0.10],[0.85,0.24],[0.10,0.70],[0.32,0.65],[0.70,0.72],[0.88,0.65]]
                Tree { required property var modelData; x: (parent.width-32)*modelData[0]; y: (parent.height-32)*modelData[1] }
            }
            Rectangle { antialiasing: true; x: parent.width*0.70; y: parent.height*0.52-20; width: 26; height: 5; color: "#968c72"; radius: 1 }
            Rectangle { antialiasing: true; x: parent.width*0.22; y: parent.height*0.52+16; width: 26; height: 5; color: "#968c72"; radius: 1 }
        }
        // Plaza pavimentada con parterre central.
        Item {
            anchors.fill: parent; visible: block.kind===2
            Repeater { model: Math.max(0,Math.ceil(parent.width/24)); Rectangle { antialiasing: true; required property int index; x: index*24; height: parent.height; width: 2; color: "#cbcdbf" } }
            Repeater { model: Math.max(0,Math.ceil(parent.height/24)); Rectangle { antialiasing: true; required property int index; y: index*24; width: parent.width; height: 2; color: "#cbcdbf" } }
            Rectangle { antialiasing: true; x: parent.width*0.29; y: parent.height*0.22; width: parent.width*0.42; height: parent.height*0.53; radius: 12; color: "#a2b390"; border.color: "#dfe0d1"
                Rectangle { antialiasing: true; anchors.centerIn: parent; width: Math.min(62,parent.width*0.6); height: width; radius: width/2; color: "#c6cdb7"; border.color: "#a1b18f" }
            }
            Repeater {
                model: [[0.08,0.1],[0.80,0.1],[0.08,0.70],[0.80,0.70]]
                Tree { required property var modelData; x: (parent.width-32)*modelData[0]; y: (parent.height-32)*modelData[1] }
            }
        }
        // Dos bloques residenciales con acceso interior.
        Item {
            anchors.fill: parent; visible: block.kind===3
            Rectangle { antialiasing: true; x: 0; y: parent.height*0.52; width: parent.width; height: 12; color: "#d0cdb8" }
            Building { x: 16; y: 14; width: Math.max(60,parent.width*0.49-22); height: Math.max(45,parent.height*0.40); roof: "#c1b5a3" }
            Building { x: parent.width*0.56; y: 14; width: Math.max(60,parent.width*0.44-16); height: Math.max(45,parent.height*0.40); roof: "#b7bdb7" }
            Building { x: 16; y: parent.height*0.66; width: Math.max(60,parent.width*0.49-22); height: Math.max(30,parent.height*0.24); roof: "#b7c2bd" }
            Tree { x: parent.width*0.57; y: parent.height*0.68 }
            Tree { x: parent.width*0.82; y: parent.height*0.70 }
        }
    }
    // Juntas de la acera, sin redibujado por fotograma.
    Repeater { model: Math.max(0,Math.ceil(block.width/26)); Rectangle { antialiasing: true; required property int index; x: index*26; y: block.height-11; width: 2; height: 10; color: "#b6c0b3" } }
    Repeater { model: Math.max(0,Math.ceil(block.height/26)); Rectangle { antialiasing: true; required property int index; x: block.width-11; y: index*26; width: 10; height: 2; color: "#b6c0b3" } }
}
