import Ubuntu.Connectivity 1.1

import QtQuick 2.4
import QtQuick.Layouts 1.2

import Ubuntu.Components 1.3
import Ubuntu.Components.ListItems 1.3 as ListItem

MainView {
    width: units.gu(48)
    height: units.gu(60)

    Page {
        title: "Cellular Information"

        ColumnLayout {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: units.gu(5)

            RowLayout {
                Switch {
                    checked: Connectivity.mobileDataEnabled
                    function trigger() {
                        Connectivity.mobileDataEnabled = !checked
                    }
                }                
                Label {
                    text: "Cellular Data Enabled"
                }
            }

            SortFilterModel {
                id: sortedModems
                model: Connectivity.modems
                sort.property: "Index"
                sort.order: Qt.AscendingOrder
            }

            OptionSelector {
                id: modemSelector
                expanded: selectedIndex == -1
                text: "Modems"
                model: sortedModems
                selectedIndex: -1
                delegate: OptionSelectorDelegate {
                    text: {
                        model.Index + ": " + model.Serial
                    }
                }

                property var currentSim
                onSelectedIndexChanged: currentSim = model.get(selectedIndex).Sim
                onCurrentSimChanged: {
                    if (currentSim) {
                        dataRoamingSwitch.checked = Qt.binding(function() {
                            return currentSim.DataRoamingEnabled
                        })
                        console.log(currentSim)
                        console.log(Connectivity.simForMobileData)
                        Connectivity.simForMobileData = currentSim
                    }
                }
                Component.onCompleted: {
                    for (var i = 0; i < sortedModems.count; i++) {
                        if (sortedModems.get(i).Sim == Connectivity.simForMobileData) {
                            modemSelector.selectedIndex = i
                        }
                    }
                }
            }
            ColumnLayout {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.margins: units.gu(5)

                RowLayout {
                    Switch {
                        id: dataRoamingSwitch
                        enabled: modemSelector.selectedIndex >= 0
                        checked: modemSelector.currentSim.DataRoamingEnabled
                        function trigger() {
                            modemSelector.currentSim.DataRoamingEnabled = !checked
                        }
                    }
                    Label {
                        text: "Data roaming"
                    }
                }
            }
            OptionSelector {
                expanded: true
                text: "Sims"
                model: Connectivity.sims
                selectedIndex: -1
                delegate: OptionSelectorDelegate {
                    text: model.Imsi + ": " + model.PrimaryPhoneNumber

                }
                onSelectedIndexChanged: {
                    //console.log(model.get(selectedIndex).Sim)
                    console.log(selectedIndex)
                }
            }
        }

//            Repeater {
//                model: Connectivity.modems
//                delegate: Item {
//                    SimObject {
//                        id: sim
//                        source.path: simPath.path
//                    }
//                }
//            }

//        }
    }
}
