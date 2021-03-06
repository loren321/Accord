import QtQuick 2.0
import QtQuick.Controls 2.2
import QtQuick.Window 2.3
import QtQuick.Dialogs 1.2
import accord.types 1.0

Window {
    visible: true
    title: qsTr("Accord")
    color: "#4f4b4b"
    id: main
    objectName: "main"
    width: screenGeometry.width
    height: screenGeometry.height

    function onAuthenticated() {
        stack.replace(client, {}, StackView.Immediate);
    }

    function onFailedAuthenticated() {
       stack.replace(login, {"failed": "true"}, StackView.Immediate)
    }

    function onFailedRegistered() {
        stack.replace(login, {"failed": "true", "registering": "true"}, StackView.Immediate);
    }

    function onAlreadyInCommunity() {
        alreadyInCommunityDialog.open();
    }

    function onCommunityReady(table) {
        stack.currentItem.listView.model.append({});
    }

    function onInviteReady(id, invite) {
        inviteDialog.invite = invite;
        inviteDialog.communityId = id;
        inviteDialog.open();
    }

    StackView {
        id: stack
        initialItem: login
        width: parent.width
        height: parent.height

        Component {
            id: login
            Login {
                width: parent.width
                height: parent.height
            }
        }

        Component {
            id: client
            Client {
                width: parent.width
                height: parent.height
            }
        }
    }

    MessageDialog {
        property string communityId
        property string invite
        id: inviteDialog
        title: "Invite is ready"
        text: {
            "Your invite for "+ communityId + " is " + invite + ".
Pressing OK will copy the invite to the clipboard."
        }
        standardButtons: StandardButton.Ok | StandardButton.Cancel
        width: 200
        height: 200
        onAccepted: {
            backend.stringToClipboard(invite);
        }
    }

    MessageDialog {
        id: alreadyInCommunityDialog
        title: "Error"
        text: {
            "You cannot accept an invite to a community you are already in!"
        }
        width: 200
        height: 200
    }
}
