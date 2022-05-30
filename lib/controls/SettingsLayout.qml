// Copyright 2018-2020 Camilo Higuita <milo.h@aol.com>
// Copyright 2018-2020 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

import org.mauikit.controls 1.3 as Maui

Maui.ScrollView
{
    id: control
    default property alias content: _layout.data

    contentWidth: availableWidth
    contentHeight: _layout.implicitHeight

    Flickable
    {
        id: _flickable
        boundsBehavior: Flickable.StopAtBounds
        boundsMovement: Flickable.StopAtBounds

        ColumnLayout
        {
            id:  _layout
            width: parent.width
        }
    }
}
