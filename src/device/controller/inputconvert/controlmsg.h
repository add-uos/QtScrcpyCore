#ifndef CONTROLMSG_H
#define CONTROLMSG_H

#include <QBuffer>
#include <QRect>
#include <QString>

#include "input.h"
#include "keycodes.h"
#include "qscrcpyevent.h"

#define CONTROL_MSG_MAX_SIZE (1 << 18) // 256k

#define CONTROL_MSG_INJECT_TEXT_MAX_LENGTH 300
// type: 1 byte; sequence: 8 bytes; paste flag: 1 byte; length: 4 bytes
#define CONTROL_MSG_CLIPBOARD_TEXT_MAX_LENGTH \
    (CONTROL_MSG_MAX_SIZE - 14)

#define POINTER_ID_MOUSE static_cast<quint64>(-1)
#define POINTER_ID_GENERIC_FINGER static_cast<quint64>(-2)

// Used for injecting an additional virtual pointer for pinch-to-zoom
#define POINTER_ID_VIRTUAL_MOUSE static_cast<quint64>(-3)
#define POINTER_ID_VIRTUAL_FINGER static_cast<quint64>(-4)

// ControlMsg
class ControlMsg : public QScrcpyEvent
{
public:
    enum ControlMsgType
    {
        CMT_NULL = -1,
        CMT_INJECT_KEYCODE = 0,
        CMT_INJECT_TEXT,
        CMT_INJECT_TOUCH,
        CMT_INJECT_SCROLL,
        CMT_BACK_OR_SCREEN_ON,
        CMT_EXPAND_NOTIFICATION_PANEL,
        CMT_EXPAND_SETTINGS_PANEL,
        CMT_COLLAPSE_PANELS,
        CMT_GET_CLIPBOARD,
        CMT_SET_CLIPBOARD,
        CMT_SET_DISPLAY_POWER,
        CMT_ROTATE_DEVICE
    };

    enum GetClipboardCopyKey {
        GCCK_NONE,
        GCCK_COPY,
        GCCK_CUT,
    };

    ControlMsg(ControlMsgType controlMsgType);
    virtual ~ControlMsg();

    void setInjectKeycodeMsgData(AndroidKeyeventAction action, AndroidKeycode keycode, quint32 repeat, AndroidMetastate metastate);
    void setInjectTextMsgData(QString &text);
    // id 代表一个触摸点，最多支持10个触摸点[0,9]
    // action 只能是AMOTION_EVENT_ACTION_DOWN，AMOTION_EVENT_ACTION_UP，AMOTION_EVENT_ACTION_MOVE
    // position action动作对应的位置
    void setInjectTouchMsgData(
        quint64 id,
        AndroidMotioneventAction action,
        AndroidMotioneventButtons actionButtons,
        AndroidMotioneventButtons buttons,
        QRect position,
        float pressure);
    void setInjectScrollMsgData(QRect position, float hScroll, float vScroll, AndroidMotioneventButtons buttons);
    void setGetClipboardMsgData(ControlMsg::GetClipboardCopyKey copyKey); 
    void setSetClipboardMsgData(QString &text, bool paste);
    void setDisplayPowerData(bool on);
    void setBackOrScreenOnData(bool down);

    QByteArray serializeData();

private:
    void writePosition(QBuffer &buffer, const QRect &value);
    quint16 flostToU16fp(float f);
    qint16 flostToI16fp(float f);

private:
    struct ControlMsgData
    {
        ControlMsgType type = CMT_NULL;
        union
        {
            struct
            {
                AndroidKeyeventAction action;
                AndroidKeycode keycode;
                quint32 repeat;
                AndroidMetastate metastate;
            } injectKeycode;
            struct
            {
                char *text = Q_NULLPTR;
            } injectText;
            struct
            {
                quint64 id;
                AndroidMotioneventAction action;
                AndroidMotioneventButtons actionButtons;
                AndroidMotioneventButtons buttons;
                QRect position;
                float pressure;
            } injectTouch;
            struct
            {
                QRect position;
                float hScroll;
                float vScroll;
                AndroidMotioneventButtons buttons;
            } injectScroll;
            struct
            {
                AndroidKeyeventAction action; // action for the BACK key
                // screen may only be turned on on ACTION_DOWN
            } backOrScreenOn;
            struct
            {
                enum GetClipboardCopyKey copyKey;
            } getClipboard;
            struct
            {
                uint64_t sequence = 0;
                char *text = Q_NULLPTR;
                bool paste = true;
            } setClipboard;
            struct
            {
                bool on;
            } setDisplayPower;
        };

        ControlMsgData() {}
        ~ControlMsgData() {}
    };

    ControlMsgData m_data;
};

#endif // CONTROLMSG_H
