#ifndef _FARAGE_EVENT_
#define _FARAGE_EVENT_

#define FARAGE_ONSTART_FUNC                 "onModuleStart"
#define FARAGE_ONSTOP_FUNC                  "onModuleStop"
#define FARAGE_ONSHUTDOWN_FUNC              "onShutdown"
#define FARAGE_ONLOADED_FUNC                "onModulesLoaded"
#define FARAGE_ONREADY_FUNC                 "onReady"
#define FARAGE_ONRESUMED_FUNC               "onResumed"
#define FARAGE_ONDELETESERVER_FUNC          "onDeleteServer"
#define FARAGE_ONEDITSERVER_FUNC            "onEditServer"
#define FARAGE_ONBAN_FUNC                   "onBan"
#define FARAGE_ONUNBAN_FUNC                 "onUnban"
#define FARAGE_ONMEMBER_FUNC                "onMember"
#define FARAGE_ONREMOVEMEMBER_FUNC          "onRemoveMember"
#define FARAGE_ONEDITMEMBER_FUNC            "onEditMember"
#define FARAGE_ONROLE_FUNC                  "onRole"
#define FARAGE_ONDELETEROLE_FUNC            "onDeleteRole"
#define FARAGE_ONEDITROLE_FUNC              "onEditRole"
#define FARAGE_ONEDITEMOJIS_FUNC            "onEditEmojis"
#define FARAGE_ONMEMBERCHUNK_FUNC           "onMemberChunk"
#define FARAGE_ONDELETECHANNEL_FUNC         "onDeleteChannel"
#define FARAGE_ONEDITCHANNEL_FUNC           "onEditChannel"
#define FARAGE_ONPINMESSAGE_FUNC            "onPinMessage"
#define FARAGE_ONPRESENCEUPDATE_FUNC        "onPresenceUpdate"
#define FARAGE_ONEDITUSER_FUNC              "onEditUser"
#define FARAGE_ONEDITUSERNOTE_FUNC          "onEditUserNote"
#define FARAGE_ONEDITUSERSETTINGS_FUNC      "onEditUserSettings"
#define FARAGE_ONEDITVOICESTATE_FUNC        "onEditVoiceState"
#define FARAGE_ONTYPING_FUNC                "onTyping"
#define FARAGE_ONDELETEMESSAGES_FUNC        "onDeleteMessages"
#define FARAGE_ONEDITMESSAGE_FUNC           "onEditMessage"
#define FARAGE_ONEDITVOICESERVER_FUNC       "onEditVoiceServer"
#define FARAGE_ONSERVERSYNC_FUNC            "onServerSync"
#define FARAGE_ONRELATIONSHIP_FUNC          "onRelationship"
#define FARAGE_ONDELETERELATIONSHIP_FUNC    "onDeleteRelationship"
#define FARAGE_ONREACTION_FUNC              "onReaction"
#define FARAGE_ONDELETEREACTION_FUNC        "onDeleteReaction"
#define FARAGE_ONDELETEALLREACTION_FUNC     "onDeleteAllReaction"
#define FARAGE_ONMESSAGE_FUNC               "onMessage"
#define FARAGE_ONMESSAGE_PRE_FUNC           "onMessagePrefixed"
#define FARAGE_ONDMESSAGE_FUNC              "onDirectMessage"
#define FARAGE_ONDMESSAGE_PRE_FUNC          "onDirectMessagePrefixed"
#define FARAGE_ONCMESSAGE_FUNC              "onChannelMessage"
#define FARAGE_ONCMESSAGE_PRE_FUNC          "onChannelMessagePrefixed"
#define FARAGE_ONGMESSAGE_FUNC              "onGroupMessage"
#define FARAGE_ONGMESSAGE_PRE_FUNC          "onGroupMessagePrefixed"
#define FARAGE_ONSERVER_FUNC                "onServer"
#define FARAGE_ONCHANNEL_FUNC               "onChannel"
#define FARAGE_ONDISPATCH_FUNC              "onDispatch"
#define FARAGE_ONHEARTBEAT_FUNC             "onHeartbeat"
#define FARAGE_ONHEARTBEATACK_FUNC          "onHeartbearAck"
#define FARAGE_ONINVALIDSESSION_FUNC        "onInvalidSession"
#define FARAGE_ONDISCONNECT_FUNC            "onDisconnect"
#define FARAGE_ONRESUME_FUNC                "onResume"
#define FARAGE_ONQUIT_FUNC                  "onQuit"
#define FARAGE_ONRESTART_FUNC               "onRestart"
#define FARAGE_ONRESPONSE_FUNC              "onResponse"
#define FARAGE_ONERROR_FUNC                 "onError"

#include <string>

namespace Farage
{
    class Handle;
    
    enum Event
    {
        ONSTART = 8,
        ONSTOP,
        ONSHUTDOWN,
        ONLOADED,
        ONREADY,
        ONRESUMED,
        ONDELETESERVER,
        ONEDITSERVER,
        ONBAN,
        ONUNBAN,
        ONMEMBER,
        ONREMOVEMEMBER,
        ONEDITMEMBER,
        ONROLE,
        ONDELETEROLE,
        ONEDITROLE,
        ONEDITEMOJIS,
        ONMEMBERCHUNK,
        ONDELETECHANNEL,
        ONEDITCHANNEL,
        ONPINMESSAGE,
        ONPRESENCEUPDATE,
        ONEDITUSER,
        ONEDITUSERNOTE,
        ONEDITUSERSETTINGS,
        ONEDITVOICESTATE,
        ONTYPING,
        ONDELETEMESSAGES,
        ONEDITMESSAGE,
        ONEDITVOICESERVER,
        ONSERVERSYNC,
        ONRELATIONSHIP,
        ONDELETERELATIONSHIP,
        ONREACTION,
        ONDELETEREACTION,
        ONDELETEALLREACTION,
        ONMESSAGE,
        ONMESSAGE_PRE,
        ONDMESSAGE,
        ONDMESSAGE_PRE,
        ONCMESSAGE,
        ONCMESSAGE_PRE,
        ONGMESSAGE,
        ONGMESSAGE_PRE,
        ONSERVER,
        ONCHANNEL,
        ONDISPATCH,
        ONHEARTBEAT,
        ONHEARTBEATACK,
        ONINVALIDSESSION,
        ONDISCONNECT,
        ONRESUME,
        ONQUIT,
        ONRESTART,
        ONRESPONSE,
        ONERROR
    };
    
    class Handle;
    
    typedef int (*EventCallback)(Handle&,Event,void*,void*,void*,void*);
    
    struct EventWrap
    {
        Event event;
        EventCallback func;
    };
    
    std::string eventName(Event event);
};

#endif

