#ifndef _FARAGE_LINK_
#define _FARAGE_LINK_

#include "api/basics.h"

namespace Farage
{
    inline ObjectResponse<Message> sendMessage(const std::string &channelID, const std::string &message, const std::string &json = "", bool tts = false)
    {
        return recallGlobal()->callbacks.sendMessage(channelID,message,json,tts);
    }
    inline ObjectResponse<Message> messageChannel(const Channel &channel, const std::string &message, const std::string &json = "", bool tts = false)
    {
        return recallGlobal()->callbacks.sendMessage(channel.id,message,json,tts);
    }
    inline ObjectResponse<Message> messageChannelID(const std::string &channelID, const std::string &message, const std::string &json = "", bool tts = false)
    {
        return recallGlobal()->callbacks.sendMessage(channelID,message,json,tts);
    }
    inline ObjectResponse<Message> messageReply(const Message &msg, const std::string &message, const std::string &json = "", bool tts = false)
    {
        return recallGlobal()->callbacks.sendMessage(msg.channel_id,message,json,tts);
    }
    inline ObjectResponse<Reaction> reaction(const Message &msg, const std::string &emoji)
    {
        return recallGlobal()->callbacks.reactToID(msg.channel_id,msg.id,emoji);
    }
    inline ObjectResponse<Reaction> reactToID(const std::string &channelID, const std::string &messageID, const std::string &emoji)
    {
        return recallGlobal()->callbacks.reactToID(channelID,messageID,emoji);
    }
    inline ObjectResponse<Channel> getChannel(const std::string &channelID)
    {
        return recallGlobal()->callbacks.getChannel(channelID);
    }
    inline ObjectResponse<Channel> getDirectMessageChannel(const std::string &userID)
    {
        return recallGlobal()->callbacks.getDirectMessageChannel(userID);
    }
    inline ObjectResponse<User> getUser(const std::string &userID)
    {
        return recallGlobal()->callbacks.getUser(userID);
    }
    inline ObjectResponse<User> getSelf()
    {
        return recallGlobal()->callbacks.getSelf();
    }
    inline BoolResponse sendTyping(const std::string &channelID)
    {
        return recallGlobal()->callbacks.sendTyping(channelID);
    }
    inline ObjectResponse<Message> sendEmbed(const std::string &channelID, const std::string &json, const std::string &message = "", bool tts = false)
    {
        return recallGlobal()->callbacks.sendMessage(channelID,message,json,tts);
    }
    inline ObjectResponse<Message> sendFile(const std::string &channelID, const std::string &filepath, const std::string &message = "")
    {
        return recallGlobal()->callbacks.sendFile(channelID,filepath,message);
    }
    inline Server getGuildCache(const std::string &guildID)
    {
        return recallGlobal()->callbacks.getGuildCache(guildID);
    }
    inline ServerMember getServerMember(const std::string &guildID, const std::string &userID)
    {
        return recallGlobal()->callbacks.getServerMember(guildID,userID);
    }
    inline Channel getChannelCache(const std::string &guildID, const std::string &channelID)
    {
        return recallGlobal()->callbacks.getChannelCache(guildID,channelID);
    }
    inline ObjectResponse<Channel> editChannel(const std::string &channelID, const std::string &name = "", const std::string &topic = "")
    {
        return recallGlobal()->callbacks.editChannel(channelID,name,topic);
    }
    inline ObjectResponse<Channel> editChannelName(const std::string &channelID, const std::string &name)
    {
        return recallGlobal()->callbacks.editChannelName(channelID,name);
    }
    inline ObjectResponse<Channel> editChannelTopic(const std::string &channelID, const std::string &topic)
    {
        return recallGlobal()->callbacks.editChannelTopic(channelID,topic);
    }
    inline ObjectResponse<Channel> deleteChannel(const std::string &channelID)
    {
        return recallGlobal()->callbacks.deleteChannel(channelID);
    }
    inline ArrayResponse<Message> getMessages(const std::string &channelID, GetMessagesKey when, const std::string &messageID, uint8_t limit = 0)
    {
        return recallGlobal()->callbacks.getMessages(channelID,when,messageID,limit);
    }
    inline ObjectResponse<Message> getMessage(const std::string &channelID, const std::string &messageID)
    {
        return recallGlobal()->callbacks.getMessage(channelID,messageID);
    }
    inline BoolResponse removeReaction(const std::string &channelID, const std::string &messageID, const std::string &emoji, const std::string &userID = "@me")
    {
        return recallGlobal()->callbacks.removeReaction(channelID,messageID,emoji,userID);
    }
    inline ArrayResponse<Reaction> getReactions(const std::string &channelID, const std::string &messageID, const std::string &emoji)
    {
        return recallGlobal()->callbacks.getReactions(channelID,messageID,emoji);
    }
    inline Response removeAllReactions(const std::string &channelID, const std::string &messageID)
    {
        return recallGlobal()->callbacks.removeAllReactions(channelID,messageID);
    }
    inline ObjectResponse<Message> editMessage(const std::string &channelID, const std::string &messageID, const std::string &newMessage)
    {
        return recallGlobal()->callbacks.editMessage(channelID,messageID,newMessage);
    }
    inline BoolResponse deleteMessage(const std::string &channelID, const std::string &messageID)
    {
        return recallGlobal()->callbacks.deleteMessage(channelID,messageID);
    }
    inline BoolResponse bulkDeleteMessages(const std::string &channelID, const std::vector<std::string> &messageIDs)
    {
        return recallGlobal()->callbacks.bulkDeleteMessages(channelID,messageIDs);
    }
    inline BoolResponse editChannelPermissions(const std::string &channelID, const std::string &overwriteID, int allow, int deny, const std::string &type)
    {
        return recallGlobal()->callbacks.editChannelPermissions(channelID,overwriteID,allow,deny,type);
    }
    inline ArrayResponse<Invite> getChannelInvites(const std::string &channelID)
    {
        return recallGlobal()->callbacks.getChannelInvites(channelID);
    }
    inline ObjectResponse<Invite> createChannelInvite(const std::string &channelID, const uint64_t maxAge = 0, const uint64_t maxUses = 0, const bool temporary = false, const bool unique = false)
    {
        return recallGlobal()->callbacks.createChannelInvite(channelID,maxAge,maxUses,temporary,unique);
    }
    inline BoolResponse removeChannelPermission(const std::string &channelID,const std::string &ID)
    {
        return recallGlobal()->callbacks.removeChannelPermission(channelID,ID);
    }
    inline ArrayResponse<Message> getPinnedMessages(const std::string &channelID)
    {
        return recallGlobal()->callbacks.getPinnedMessages(channelID);
    }
    inline BoolResponse pinMessage(const std::string &channelID, const std::string &messageID)
    {
        return recallGlobal()->callbacks.pinMessage(channelID,messageID);
    }
    inline BoolResponse unpinMessage(const std::string &channelID, const std::string &messageID)
    {
        return recallGlobal()->callbacks.unpinMessage(channelID,messageID);
    }
    inline Response addRecipient(const std::string &channelID, const std::string &userID)
    {
        return recallGlobal()->callbacks.addRecipient(channelID,userID);
    }
    inline Response removeRecipient(const std::string &channelID, const std::string &userID)
    {
        return recallGlobal()->callbacks.removeRecipient(channelID,userID);
    }
    inline std::string serverCommand(const std::string &command)
    {
        return recallGlobal()->callbacks.serverCommand(command);
    }
};

#endif

