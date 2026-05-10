import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.FormLayout {
    id: page

    property alias cfg_imagePath: imagePathField.text
    property alias cfg_clickUrl:  clickUrlField.text
    property alias cfg_showLabel: showLabelCheck.checked

    TextField {
        id: imagePathField
        Kirigami.FormData.label: "Ad image path:"
        Layout.fillWidth: true
        placeholderText: "/path/to/your/ad.png (or .jpg, .gif, .svg)"
    }

    TextField {
        id: clickUrlField
        Kirigami.FormData.label: "Click URL:"
        Layout.fillWidth: true
        placeholderText: "https://example.com"
    }

    CheckBox {
        id: showLabelCheck
        Kirigami.FormData.label: "Show 'Sponsored' label:"
        text: "Yes, remind me that I'm advertising to myself"
    }

    Kirigami.Separator {
        Kirigami.FormData.isSection: true
        Kirigami.FormData.label: "Tips"
    }

    Label {
        Kirigami.FormData.label: " "
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        text: "Paste the full path to your ad image above.\n" +
              "For a truly enshittified experience, pick something\n" +
              "completely irrelevant to what you're doing.\n\n" +
              "Recommended: a banner ad for Windows 11."
        color: Kirigami.Theme.disabledTextColor
        font.italic: true
    }
}
