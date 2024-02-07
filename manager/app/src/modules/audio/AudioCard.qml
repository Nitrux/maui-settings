
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

import org.mauikit.controls 1.3 as Maui

import org.maui.settings 1.0 as Man
import org.maui.settings.lib 1.0 as ManLib

import org.mauicore.audio as MauiCore


Maui.SectionItem
{
    id: control

    readonly property alias slide : _slider

    readonly property int volume: model.Volume
    property bool ignoreValueChange: true
    readonly property var currentPort: model.Ports[model.ActivePortIndex]
    readonly property int percent: (volume * 100) / MauiCore.PulseAudio.NormalVolume

    label1.text:
    {
        if (currentPort && currentPort.description) {
            if (!model.Description) {
                return currentPort.description;
            } else {
                return i18nc("label of device items", "%1 (%2)", currentPort.description, model.Description);
            }
        }
        if (model.Description) {
            return model.Description;
        }
        if (model.Name) {
            return model.Name;
        }
        return i18n("Device name not found");
    }

    label2.text: model.Description
    label3.text: percent

    onVolumeChanged:
    {
        var oldIgnoreValueChange = ignoreValueChange;
        ignoreValueChange = true;
        _slider.value = model.Volume;
        ignoreValueChange = oldIgnoreValueChange;
    }

    MauiCore.VolumeMonitor
    {
        id: meter
        target: _slider.visible && model.PulseObject ? model.PulseObject : null
    }

    Timer
    {
        id: updateTimer
        interval: 200
        triggeredOnStart: true
        onTriggered: updateSliderValue(control.volume)
    }

    Component.onCompleted:
    {
        ignoreValueChange = false;
        updateTimer.start()
    }

    SliderTemplate
    {
        id: _slider
        Layout.fillWidth: true

        from: MauiCore.PulseAudio.MinimalVolume
        to: MauiCore.PulseAudio.MaximalVolume
        value: model.Volume
        stepSize: _slider.to / (_slider.to / MauiCore.PulseAudio.NormalVolume * 100.0)
        iconSource: "audio-volume-high"

        opacity: model.Muted ? 0.5 : 1
        //slider.value: 0.5
        animatedRec.width: (_slider.handle.x * meter.volume) + Maui.Style.space.medium
    }

    function updateSliderValue(value)
    {
        _slider.value = value
    }
}
