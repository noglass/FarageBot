#include <string>
#include "api/events.h"

std::string Farage::eventName(Event event)
{
    switch (event)
    {
        case ONSTART:                return FARAGE_ONSTART_FUNC;
        case ONSTOP:                 return FARAGE_ONSTOP_FUNC;
        case ONSHUTDOWN:             return FARAGE_ONSHUTDOWN_FUNC;
        case ONLOADED:               return FARAGE_ONLOADED_FUNC;
        case ONREADY:                return FARAGE_ONREADY_FUNC;
        case ONRESUMED:              return FARAGE_ONRESUMED_FUNC;
        case ONDELETESERVER:         return FARAGE_ONDELETESERVER_FUNC;
        case ONEDITSERVER:           return FARAGE_ONEDITSERVER_FUNC;
        case ONBAN:                  return FARAGE_ONBAN_FUNC;
        case ONUNBAN:                return FARAGE_ONUNBAN_FUNC;
        case ONMEMBER:               return FARAGE_ONMEMBER_FUNC;
        case ONREMOVEMEMBER:         return FARAGE_ONREMOVEMEMBER_FUNC;
        case ONEDITMEMBER:           return FARAGE_ONEDITMEMBER_FUNC;
        case ONROLE:                 return FARAGE_ONROLE_FUNC;
        case ONDELETEROLE:           return FARAGE_ONDELETEROLE_FUNC;
        case ONEDITROLE:             return FARAGE_ONEDITROLE_FUNC;
        case ONEDITEMOJIS:           return FARAGE_ONEDITEMOJIS_FUNC;
        case ONMEMBERCHUNK:          return FARAGE_ONMEMBERCHUNK_FUNC;
        case ONDELETECHANNEL:        return FARAGE_ONDELETECHANNEL_FUNC;
        case ONEDITCHANNEL:          return FARAGE_ONEDITCHANNEL_FUNC;
        case ONPINMESSAGE:           return FARAGE_ONPINMESSAGE_FUNC;
        case ONPRESENCEUPDATE:       return FARAGE_ONPRESENCEUPDATE_FUNC;
        case ONEDITUSER:             return FARAGE_ONEDITUSER_FUNC;
        case ONEDITUSERNOTE:         return FARAGE_ONEDITUSERNOTE_FUNC;
        case ONEDITUSERSETTINGS:     return FARAGE_ONEDITUSERSETTINGS_FUNC;
        case ONEDITVOICESTATE:       return FARAGE_ONEDITVOICESTATE_FUNC;
        case ONTYPING:               return FARAGE_ONTYPING_FUNC;
        case ONDELETEMESSAGES:       return FARAGE_ONDELETEMESSAGES_FUNC;
        case ONEDITMESSAGE:          return FARAGE_ONEDITMESSAGE_FUNC;
        case ONEDITVOICESERVER:      return FARAGE_ONEDITVOICESERVER_FUNC;
        case ONSERVERSYNC:           return FARAGE_ONSERVERSYNC_FUNC;
        case ONRELATIONSHIP:         return FARAGE_ONRELATIONSHIP_FUNC;
        case ONDELETERELATIONSHIP:   return FARAGE_ONDELETERELATIONSHIP_FUNC;
        case ONREACTION:             return FARAGE_ONREACTION_FUNC;
        case ONDELETEREACTION:       return FARAGE_ONDELETEREACTION_FUNC;
        case ONDELETEALLREACTION:    return FARAGE_ONDELETEALLREACTION_FUNC;
        case ONMESSAGE:              return FARAGE_ONMESSAGE_FUNC;
        case ONMESSAGE_PRE:          return FARAGE_ONMESSAGE_PRE_FUNC;
        case ONDMESSAGE:             return FARAGE_ONDMESSAGE_FUNC;
        case ONDMESSAGE_PRE:         return FARAGE_ONDMESSAGE_PRE_FUNC;
        case ONCMESSAGE:             return FARAGE_ONCMESSAGE_FUNC;
        case ONCMESSAGE_PRE:         return FARAGE_ONCMESSAGE_PRE_FUNC;
        case ONGMESSAGE:             return FARAGE_ONGMESSAGE_FUNC;
        case ONGMESSAGE_PRE:         return FARAGE_ONGMESSAGE_PRE_FUNC;
        case ONSERVER:               return FARAGE_ONSERVER_FUNC;
        case ONCHANNEL:              return FARAGE_ONCHANNEL_FUNC;
        case ONDISPATCH:             return FARAGE_ONDISPATCH_FUNC;
        case ONHEARTBEAT:            return FARAGE_ONHEARTBEAT_FUNC;
        case ONHEARTBEATACK:         return FARAGE_ONHEARTBEATACK_FUNC;
        case ONINVALIDSESSION:       return FARAGE_ONINVALIDSESSION_FUNC;
        case ONDISCONNECT:           return FARAGE_ONDISCONNECT_FUNC;
        case ONRESUME:               return FARAGE_ONRESUME_FUNC;
        case ONQUIT:                 return FARAGE_ONQUIT_FUNC;
        case ONRESTART:              return FARAGE_ONRESTART_FUNC;
        case ONRESPONSE:             return FARAGE_ONRESPONSE_FUNC;
        case ONERROR:                return FARAGE_ONERROR_FUNC;
        default:                     return "";
    }
}

