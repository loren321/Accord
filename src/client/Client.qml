import QtQuick 2.4
import QtQuick.Window 2.3
import QtQuick.Controls 2.2
import QtGraphicalEffects 1.0

ClientForm {
    background: Rectangle {
        color: "#5d5959"
    }
    button2.onClicked: {
        channelList.model.append({})
    }

    addCommunityButton.onClicked: {
        addCommunityPopup.open();
    }

    listView.onCountChanged: {
        addCommunityButton.anchors.topMargin = (65 * listView.count)
    }
        Component {
            id: communityDelegate
            RoundButton {
                background: Image {
                    source: "image://communityImageProvider/" + modelData.id
                    sourceSize.width: 65
                    sourceSize.height: 65
                    layer.enabled: true
                    layer.effect: OpacityMask {
                        maskSource: Rectangle {
                            radius: 50
                            width: 65
                            height: 65
                            visible: false
                        }
                    }
                }
                onClicked: {
                    var id = modelData.id;
                    selectedCommunity = id;
                    backend.loadChannels(id);
                }

                text: modelData.name
                radius: 50
                width: 65
                height: 65
            }
        }
        Component {
            id: channelDelegate
            Rectangle {
                Label {
                    text: modelData.name
                    width: 50
                    height: 20
                }
                color: "red"
                width: 50
                height: 20
            }
        }

        Popup {
            id: addCommunityPopup
            x: Math.round((parent.width - width) / 2)
            y: Math.round((parent.height - height) / 2)
            width: 640
            height: 480
            modal: true
            focus: true
            closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
            AddCommunity {}
        }
    }
