pragma Singleton
import QtQuick

QtObject {
    property color backgroundColor_1: "#141414"
    property color backgroundColor_2: "#232323"
    property color backgroundColor_3: "#262E3C"
    property color tabColor: "#400080"

    property color borderColor_1: "#141414"

    /*
    property color color_1: "#002346"
    property color color_2: "#052D55"
    property color color_3: "#00326E"
    property color color_4: "#305A8A"
    property color color_5: "#145FA5"
    property color color_6: "#50A0F0"
    */

    property int textSize: 10
    property font font
    font.family: "Segoe UI"
    property color textColor: "white"
    property color textHighlightColor: "#FF80FF"

    property real buttonBorderWidth: 2
    property real buttonCornerRadius: 8
    property color buttonBkgColor: "#400080"
    property color buttonBkgHoverColor: "#5A3597"
    property color buttonBkgPressedColor: "#7346BE"
    property color buttonBorderColor: "#5A3597"
    property color buttonBorderHoverColor: "#7346BE"
    property color buttonBorderPressedColor: "#7346BE"
    property color buttonTextColor: textColor
    property color buttonTextHoverColor: textHighlightColor
    property color buttonTextPressedColor: textHighlightColor

    property real radioButtonBorderWidth: 1
    property color radioButtonBkgColor: "black"
    property color radioButtonBkgHoverColor: radioButtonBkgColor
    property color radioButtonBkgPressedColor: radioButtonBkgColor
    property color radioButtonBorderColor: "#808080"
    property color radioButtonBorderHoverColor: "#B0B0B0"
    property color radioButtonBorderPressedColor: radioButtonBorderHoverColor
    property color radioButtonTextColor: textColor
    property color radioButtonTextHoverColor: textColor
    property color radioButtonTextPressedColor: textColor

    property real comboBoxBorderWidth: 1
    property real comboBoxCornerRadius: 8
    property color comboBoxBkgColor: "#400080"
    property color comboBoxBkgHoverColor: comboBoxBkgColor
    property color comboBoxBkgPressedColor: comboBoxBkgColor
    property color comboBoxBorderColor: "#7346BE"
    property color comboBoxBorderHoverColor: "#FF80FF"
    property color comboBoxBorderPressedColor: comboBoxBorderHoverColor
    property color comboBoxTextColor: textColor
    property color comboBoxTextHoverColor: textColor
    property color comboBoxTextPressedColor: textColor

    property real popupBorderWidth: 1
    property real popupCornerRadius: 0
    property color popupBkgColor: "#400080"
    property color popupTextColor: textColor
    property color popupBkgHighlightColor: "#7346BE"

    property real textFieldBorderWidth: 1
    property real textFieldCornerRadius: 12
    property color textFieldBkgColor: "#400080"
    property color textFieldBkgHoverColor: textFieldBkgColor
    property color textFieldBkgPressedColor: textFieldBkgColor
    property color textFieldBorderColor: "#7346BE"
    property color textFieldBorderHoverColor: "#FF80FF"
    property color textFieldBorderPressedColor: textFieldBorderHoverColor
    property color textFieldTextColor: textColor
    property color textFieldTextHoverColor: textColor
    property color textFieldTextPressedColor: textColor
    property color textFieldTextSelectionColor: "#FF80FF"

    property real scrollBarCornerRadius: 4
    property real scrollBarWidth: 8
    property color scrollBarHandleColor: "white"

    property color sliderHandleColor: "#7346BE"
    property color sliderHandleBorderColor: "#232323"
    property color sliderFillColor: "#5A3597"
    property color sliderVoidColor: "#400080"
    property color sliderBorderColor: "#232323"

    property real toolTipCornerRadius: 4
    property color toolTipBkgColor: "#400080"
    property color toolTipBorderColor: "#7346BE"

    property color listViewHeaderColor: tabColor
    property color listViewBkgColor: backgroundColor_3
    property color listViewTextColor: textColor
    property color listViewBkgHighlightColor: "#7346BE"

    property real joystickBorderWidth: 3
    property color joystickBkgColor: "#400080"
    property color joystickBkgHoverColor: "#5A3597"
    property color joystickBkgPressedColor: "#7346BE"
    property color joystickBorderColor: "#7346BE"
    property color joystickTextColor: textColor
    property color joystickTextHoverColor: "#FF80FF"
    property color joystickTextPressedColor: "#FF80FF"
}
