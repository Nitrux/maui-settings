
/*
 * Copyright 2018 Alexis Lopes Zubeta <contact@azubieta.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

import org.nomad.ufw 1.0 as UFW

ColumnLayout {
    spacing: 12
    GlobalRules {
    }

    FocusScope {
        id: rulesViewRoot

        Layout.topMargin: 6
        Layout.fillWidth: true
        Layout.fillHeight: true

        clip: true

        Component {
            id: sectionHeading
            PlasmaComponents.ListItem {
                sectionDelegate: true
                Item {
                    height: 32
                    width: 200
                    PlasmaExtras.Heading {
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        level: 4
                        text: section == "true" ? "IPv6" : "IPv4"
                    }
                }
            }
        }

        PlasmaExtras.ScrollArea {
            id: listScrollArea
            anchors.fill: parent

            ListView {
                id: listView
                model: ufwClient.rules()
                bottomMargin: 48 * 2
                delegate: RuleListItem {
                    dropAreasVisible: true
                    width: listView.width
                    onMove: function (from, to) {
                        //                    print("moving ", from, " to ", to)
                        if (from < to)
                            to = to - 1

                        // Force valid positions
                        to = Math.max(0, to)
                        to = Math.min(listView.model.rowCount(), to)

                        if (from !== to) {
                            //                        listView.model.move(from, to)
                            ufwClient.moveRule(from, to)
                        }
                    }

                    onEdit: function (index) {
                        var rule = ufwClient.getRule(index)
                        ruleDetailsLoader.setSource("RuleEdit.qml", {
                                                        rule: rule,
                                                        newRule: false,
                                                        x: 0,
                                                        y: 0,
                                                        height: mainWindow.height,
                                                        width: mainWindow.width
                                                    })
                    }

                    onRemove: function (index) {
                        ufwClient.removeRule(index)
                    }
                }

                section.property: "ipv6"
                section.criteria: ViewSection.FullString
                section.delegate: sectionHeading
            }
        }

        PlasmaComponents.ToolButton {
            height: 48
            iconSource: "list-add"
            text: i18n("New Rule")

            onClicked: {
                ruleDetailsLoader.setSource("RuleEdit.qml", {
                                                newRule: true,
                                                x: 0,
                                                y: 0,
                                                height: mainWindow.height,
                                                width: mainWindow.width
                                            })
            }

            anchors.bottom: parent.bottom
            anchors.bottomMargin: 12
            anchors.right: parent.right
//            anchors.rightMargin: 12
        }
    }
}
