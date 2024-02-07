// Copyright 2018-2020 Camilo Higuita <milo.h@aol.com>
// Copyright 2018-2020 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

import org.mauikit.controls 1.3 as Maui

import org.maui.settings 1.0 as Man
import org.maui.settings.lib 1.0 as ManLib

import org.mauicore.audio as MauiCore

ManLib.SettingsPage
{
    id: control

    property var module
    readonly property var manager : control.module.manager

    property var defaultSinkValue: defaultSink ? defaultSink.volume / MauiCore.PulseAudio.MaximalVolume * 100.0 : -1
    property var defaultSink: paSinkModel.defaultSink


    title: module.name

    MauiCore.SinkModel
    {
        id: paSinkModel

        onDefaultSinkChanged:
        {
            if (!defaultSink) {
                return
            }
        }
    }

    // Input devices
    MauiCore.SourceModel
    {
        id: paSourceModel
    }
    // Confusingly, Sink Input is what PulseAudio calls streams that send audio to an output device
    MauiCore.SinkInputModel { id: paSinkInputModel }

    // Confusingly, Source Output is what PulseAudio calls streams that take audio from an input device
    MauiCore.SourceOutputModel { id: paSourceOutputModel }

    MauiCore.PulseObjectFilterModel
    {
        id: paSinkFilterModel
        sortRole: "SortByDefault"
        sortOrder: Qt.DescendingOrder
        //        filterOutInactiveDevices: true
        sourceModel: paSinkModel
    }

    // non-virtual streams going to output devices
    MauiCore.PulseObjectFilterModel {
        id: paSinkInputFilterModel
        filters: [ { role: "VirtualStream", value: false } ]
        sourceModel: paSinkInputModel
    }

    // non-virtual streams coming from input devices
    MauiCore.PulseObjectFilterModel {
        id: paSourceOutputFilterModel
        filters: [ { role: "VirtualStream", value: false } ]
        sourceModel: paSourceOutputModel
    }

       // active input devices
       MauiCore.PulseObjectFilterModel
       {
           id: paSourceFilterModel
           sortRole: "SortByDefault"
           sortOrder: Qt.DescendingOrder
           filterOutInactiveDevices: true
           sourceModel: paSourceModel
       }

    MauiCore.CardModel
    {
        id: paCardModel
    }

    MauiCore.VolumeFeedback
    {
        id: feedback
    }

    Maui.SectionGroup
    {
        title: i18n("Playback")
        description: i18n("Playback devices.")

        Repeater
        {
            model:paSinkFilterModel

            AudioCard {}
        }
    }
    Maui.SectionGroup
    {
        title: i18n("Apps")
        description: i18n("Playback devices.")

        Repeater
        {
            model: paSinkInputModel

            AudioCard
            {

            }
        }
    }

    Maui.SectionGroup
    {
        title: i18n("Recording")
        description: i18n("Recording devices.")

        Repeater
        {
            model: paSourceFilterModel

            AudioCard
            {

            }
        }
    }
}
