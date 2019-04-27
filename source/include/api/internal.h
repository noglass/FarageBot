#ifndef _FARAGE_INTERNAL_
#define _FARAGE_INTERNAL_

#include "api/containers.h"

namespace Farage
{
    struct Internals
    {
        ObjectResponse<Message> (*sendMessage)(const std::string&, const std::string&, const std::string&, bool);
        ObjectResponse<Reaction> (*reactToID)(const std::string&, const std::string&, const std::string&);
        ObjectResponse<Channel> (*getChannel)(const std::string&);
        ObjectResponse<Channel> (*getDirectMessageChannel)(const std::string&);
        ObjectResponse<User> (*getUser)(const std::string&);
        ObjectResponse<User> (*getSelf)();
        BoolResponse (*sendTyping)(const std::string&);
        ObjectResponse<Message> (*sendFile)(const std::string&, const std::string&, const std::string&);
        Server (*getGuildCache)(const std::string&);
        ServerMember (*getServerMember)(const std::string&, const std::string&);
        Channel (*getChannelCache)(const std::string&, const std::string&);
        ObjectResponse<Channel> (*editChannel)(const std::string&, const std::string&, const std::string&);
        ObjectResponse<Channel> (*editChannelName)(const std::string&, const std::string&);
        ObjectResponse<Channel> (*editChannelTopic)(const std::string&, const std::string&);
        ObjectResponse<Channel> (*deleteChannel)(const std::string&);
        ArrayResponse<Message> (*getMessages)(const std::string&, GetMessagesKey, const std::string&, uint8_t);
        ObjectResponse<Message> (*getMessage)(const std::string&, const std::string&);
        BoolResponse (*removeReaction)(const std::string&, const std::string&, const std::string&, const std::string&);
        ArrayResponse<Reaction> (*getReactions)(const std::string&, const std::string&, const std::string&);
        Response (*removeAllReactions)(const std::string&, const std::string&);
        ObjectResponse<Message> (*editMessage)(const std::string&, const std::string&, const std::string&);
        BoolResponse (*deleteMessage)(const std::string&, const std::string&);
        BoolResponse (*bulkDeleteMessages)(const std::string&, const std::vector<std::string>&);
        BoolResponse (*editChannelPermissions)(const std::string&, const std::string&, int, int, const std::string&);
        ArrayResponse<Invite> (*getChannelInvites)(const std::string&);
        ObjectResponse<Invite> (*createChannelInvite)(const std::string&, const uint64_t, const uint64_t, const bool, const bool);
        BoolResponse (*removeChannelPermission)(const std::string&,const std::string&);
        ArrayResponse<Message> (*getPinnedMessages)(const std::string&);
        BoolResponse (*pinMessage)(const std::string&, const std::string&);
        BoolResponse (*unpinMessage)(const std::string&, const std::string&);
        Response (*addRecipient)(const std::string&, const std::string&);
        Response (*removeRecipient)(const std::string&, const std::string&);
    };
};

#endif

