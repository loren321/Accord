import QtQuick 2.0
import QtQuick.Controls 2.2
import QtQuick.Window 2.3

Window {
    visible: true
    visibility: "Maximized"
    title: qsTr("Accord")
    color: "#5d5959"
    id: main
    objectName: "main"

    function onAuthenticated() {
        stack.push(client, {}, StackView.Immediate);
    }

    function onFailedAuthenticated() {
       stack.replace(login, {"failed": "true"}, StackView.Immediate)
    }

    function onFailedRegistered() {
        stack.replace(login, {"failed": "true", "registering": "true"}, StackView.Immediate);
    }

    StackView {
        id: stack
        initialItem: login

        Component {
            id: login
            Login { }
        }

        Component {
            id: client
            Client { }
        }
    }
}