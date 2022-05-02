// Copyright 2018-2020 Camilo Higuita <milo.h@aol.com>
// Copyright 2018-2020 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later


import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import org.mauikit.controls 1.3 as Maui

Maui.Page
{
    id: control

    headBar.middleContent: Maui.ToolActions
    {
        id: _gestureOptions
        expanded: isWide
        autoExclusive: true
        currentIndex: _swipeView.currentIndex

        Action
        {
            text: i18n("Swipe")
        }

        Action
        {
            text: i18n("Pinch")

        }

        Action
        {
            text: i18n("Tap")

        }
    }

    SwipeView
    {
        id: _swipeView
        anchors.fill: parent
        currentIndex: _gestureOptions.currentIndex

        ScrollView
        {
            contentWidth: availableWidth
            contentHeight: _layout.implicitHeight

            Flickable
            {

                ColumnLayout
                {
                    id: _layout
                    width: parent.width
                    Maui.SettingsSection
                    {
                        Layout.fillWidth: true
                        title: i18n("Swipe with 2 fingers")

                        Maui.SettingTemplate
                        {
                            label1.text: i18n("Up")

                            Maui.ComboBox
                            {
                                Layout.fillWidth: true
                            }
                        }

                        Maui.SettingTemplate
                        {
                            label1.text: i18n("Down")

                            Maui.ComboBox
                            {
                                Layout.fillWidth: true
                            }
                        }

                        Maui.SettingTemplate
                        {
                            label1.text: i18n("Left")

                            Maui.ComboBox
                            {
                                Layout.fillWidth: true
                            }
                        }

                        Maui.SettingTemplate
                        {
                            label1.text: i18n("Right")

                            Maui.ComboBox
                            {
                                Layout.fillWidth: true
                            }
                        }
                    }


                    Maui.SettingsSection
                    {
                        Layout.fillWidth: true
                        title: i18n("Swipe with 3 fingers")

                        Maui.SettingTemplate
                        {
                            label1.text: i18n("Up")

                            Maui.ComboBox
                            {
                                Layout.fillWidth: true
                            }
                        }


                        Maui.SettingTemplate
                        {
                            label1.text: i18n("Down")

                            Maui.ComboBox
                            {
                                Layout.fillWidth: true
                            }
                        }

                        Maui.SettingTemplate
                        {
                            label1.text: i18n("Left")

                            Maui.ComboBox
                            {
                                Layout.fillWidth: true
                            }
                        }

                        Maui.SettingTemplate
                        {
                            label1.text: i18n("Right")

                            Maui.ComboBox
                            {
                                Layout.fillWidth: true
                            }
                        }
                    }


                    Maui.SettingsSection
                    {
                        Layout.fillWidth: true
                        title: i18n("Swipe with 4 fingers")

                        Maui.SettingTemplate
                        {
                            label1.text: i18n("Up")

                            Maui.ComboBox
                            {
                                Layout.fillWidth: true
                            }
                        }


                        Maui.SettingTemplate
                        {
                            label1.text: i18n("Down")

                            Maui.ComboBox
                            {
                                Layout.fillWidth: true
                            }
                        }

                        Maui.SettingTemplate
                        {
                            label1.text: i18n("Left")

                            Maui.ComboBox
                            {
                                Layout.fillWidth: true
                            }
                        }

                        Maui.SettingTemplate
                        {
                            label1.text: i18n("Right")

                            Maui.ComboBox
                            {
                                Layout.fillWidth: true
                            }
                        }
                    }
                }
            }

        }



        Item {}

        Item {}
    }
}
