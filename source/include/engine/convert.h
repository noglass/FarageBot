#ifndef _FARAGE_ENGINE_CONVERT_
#define _FARAGE_ENGINE_CONVERT_

namespace Farage
{
    User convertUser(SleepyDiscord::User user)
    {
        return User{
            user.ID,
            user.username,
            user.discriminator,
            user.avatar,
            user.bot,
            user.mfa_enabled,
            user.verified,
            user.email,
        };
    }
    
    Channel convertChannel(SleepyDiscord::Channel channel)
    {
        Channel fchannel = {
            channel.ID,
            channel.type,
            channel.serverID,
            channel.position,
            channel.name,
            channel.topic,
            channel.isNSFW,
            channel.lastMessageID,
            channel.bitrate,
            channel.userLimit,
            std::vector<User>(channel.recipients.size()),
            channel.icon,
            channel.ownerID,
            channel.parentID,
            channel.lastPinTimestamp
        };
        auto itt = fchannel.recipients.begin();
        for (auto it = channel.recipients.begin(), ite = channel.recipients.end();it != ite;++it,++itt)
            *itt = convertUser(*it);
        return std::move(fchannel);
    }
    
    Role convertRole(SleepyDiscord::Role role)
    {
        return Role{
            role.ID,
            role.name,
            role.color,
            role.hoist,
            role.position,
            role.permissions,
            role.managed,
            role.mentionable
        };
    }
    
    ServerMember convertServerMember(SleepyDiscord::ServerMember member)
    {
        ServerMember fmember = {
            convertUser(member.user),
            member.nick,
            std::vector<std::string>(member.roles.size()),
            member.joinedAt,
            member.deaf,
            member.mute
        };
        fmember.roles.assign(member.roles.begin(),member.roles.end());
        return std::move(fmember);
    }
    
    Server convertServer(SleepyDiscord::Server server)
    {
        Server fserver = {
            server.ID,
            server.name,
            server.icon,
            server.splash,
            server.ownerID,
            server.permissions,
            server.region,
            server.AFKchannelID,
            server.AFKTimeout,
            server.embedEnable,
            server.embedChannelID,
            server.verificationLevel,
            server.defaultMessageNotifications,
            std::vector<Role>(server.roles.size()),
            server.MFALevel,
            server.joinedAt,
            server.large,
            server.unavailable,
            std::vector<ServerMember>(server.members.size()),
            std::vector<Channel>(server.channels.size())
        };
        {
            auto itt = fserver.roles.begin();
            for (auto it = server.roles.begin(), ite = server.roles.end();it != ite;++it,++itt)
                *itt = std::move(convertRole(*it));
        }
        {
            auto itt = fserver.members.begin();
            for (auto it = server.members.begin(), ite = server.members.end();it != ite;++it,++itt)
                *itt = std::move(convertServerMember(*it));
        }
        {
            auto itt = fserver.channels.begin();
            for (auto it = server.channels.begin(), ite = server.channels.end();it != ite;++it,++itt)
                *itt = std::move(convertChannel(*it));
        }
        return std::move(fserver);
    }
    
    Message convertMessage(SleepyDiscord::Message message)
    {
        Message fmessage = {
            message.ID,
            message.channelID,
            message.serverID,
            convertUser(message.author),
            message.content,
            message.timestamp,
            message.editedTimestamp,
            message.tts,
            message.mentionEveryone,
            std::vector<User>(message.mentions.size()),
            std::vector<std::string>(message.mentionRoles.size()),
            message.pinned,
            message.type
        };
        auto itt = fmessage.mentions.begin();
        for (auto it = message.mentions.begin(), ite = message.mentions.end();it != ite;++it,++itt)
            *itt = std::move(convertUser(*it));
        fmessage.mention_roles.assign(message.mentionRoles.begin(),message.mentionRoles.end());
        return std::move(fmessage);
    }
    
    Ready convertReady(SleepyDiscord::Ready readyData)
    {
        Ready fready = {
            readyData.v,
            convertUser(readyData.user),
            std::vector<Channel>(readyData.privateChannels.size()),
            std::vector<std::string>(readyData.servers.size()),
            readyData.sessionID
        };
        {
            auto itt = fready.private_channels.begin();
            for (auto it = readyData.privateChannels.begin(), ite = readyData.privateChannels.end();it != ite;++it,++itt)
                *itt = std::move(convertChannel(*it));
        }
        {
            auto itt = fready.guilds.begin();
            for (auto it = readyData.servers.begin(), ite = readyData.servers.end();it != ite;++it,++itt)
                itt->assign(it->ID);
        }
        //fready.guilds.assign(readyData.servers.begin(),readyData.servers.end());
        return std::move(fready);
    }
    
    Emoji convertEmoji(SleepyDiscord::Emoji emoji)
    {
        Emoji femoji = {
            emoji.ID,
            emoji.name,
            std::vector<Role>(emoji.roles.size()),
            convertUser(emoji.user),
            emoji.requireColons,
            emoji.managed,
            false
        };
        auto itt = femoji.roles.begin();
        for (auto it = emoji.roles.begin(), ite = emoji.roles.end();it != ite;++it,++itt)
            *itt = std::move(convertRole(*it));
        return std::move(femoji);
    }
    
    ActivityAssets convertActivityAssets(SleepyDiscord::ActivityAssets assets)
    {
        return ActivityAssets{ assets.largeImage, assets.largeText, assets.smallImage, assets.smallText };
    }
    
    ActivitySecrets convertActivitySecrets(SleepyDiscord::ActivitySecrets secrets)
    {
        return ActivitySecrets{ secrets.join, secrets.spectate, secrets.match };
    }
    
    Activity convertActivity(SleepyDiscord::Activity activity)
    {
        return Activity{
            activity.name,
            activity.type,
            activity.url,
            std::pair<std::time_t,std::time_t>(activity.timestamps.start,activity.timestamps.end),
            activity.applicationID,
            activity.details,
            activity.state,
            convertActivityAssets(activity.assets),
            convertActivitySecrets(activity.secrets),
            activity.instance,
            activity.flags
        };
    }
    
    PresenceUpdate convertPresenceUpdate(SleepyDiscord::PresenceUpdate presence)
    {
        PresenceUpdate fpresence = {
            convertUser(presence.user),
            std::vector<std::string>(presence.roleIDs.size()),
            convertActivity(presence.currentActivity),
            presence.serverID,
            presence.status,
            std::vector<Activity>(presence.activities.size())
        };
        fpresence.roles.assign(presence.roleIDs.begin(),presence.roleIDs.end());
        /*{
            auto itt = fpresence.roles.begin();
            for (auto it = presence.roleIDs.begin(), ite = presence.roleIDs.end();it != ite;++it,++itt)
                itt->assign(*it);
        }*/
        {
            auto itt = fpresence.activities.begin();
            for (auto it = presence.activities.begin(), ite = presence.activities.end();it != ite;++it,++itt)
                *itt = std::move(convertActivity(*it));
        }
        return std::move(fpresence);
    }
    
    Response convertResponse(SleepyDiscord::Response response)
    {
        return Response(response.statusCode,response.text,response.header);
    }
    
    VoiceState convertVoiceState(SleepyDiscord::VoiceState state)
    {
        return VoiceState{ state.serverID, state.channelID, state.userID, state.sessionID, state.deaf, state.mute, state.selfDeaf, state.selfMute, state.suppress };
    }
    
    VoiceServerUpdate convertVoiceServerUpdate(SleepyDiscord::VoiceServerUpdate update)
    {
        return VoiceServerUpdate{ update.token, update.serverID, update.endpoint };
    }
    
    Reaction convertReaction(SleepyDiscord::Reaction reaction)
    {
        return Reaction{ reaction.count, reaction.me, convertEmoji(reaction.emoji) };
    }
    
    Invite convertInvite(SleepyDiscord::Invite invite)
    {
        return Invite{ invite.code, convertServer(invite.server), convertChannel(invite.channel) };
    }
                                                                                
    User                convertObject(SleepyDiscord::User user)                 { return convertUser(std::move(user)); }
    Channel             convertObject(SleepyDiscord::Channel channel)           { return convertChannel(std::move(channel)); }
    Role                convertObject(SleepyDiscord::Role role)                 { return convertRole(std::move(role)); }
    ServerMember        convertObject(SleepyDiscord::ServerMember member)       { return convertServerMember(std::move(member)); }
    Server              convertObject(SleepyDiscord::Server server)             { return convertServer(std::move(server)); }
    Message             convertObject(SleepyDiscord::Message message)           { return convertMessage(std::move(message)); }
    Ready               convertObject(SleepyDiscord::Ready readyData)           { return convertReady(std::move(readyData)); }
    Emoji               convertObject(SleepyDiscord::Emoji emoji)               { return convertEmoji(std::move(emoji)); }
    ActivityAssets      convertObject(SleepyDiscord::ActivityAssets assets)     { return convertActivityAssets(std::move(assets)); }
    ActivitySecrets     convertObject(SleepyDiscord::ActivitySecrets secrets)   { return convertActivitySecrets(std::move(secrets)); }
    Activity            convertObject(SleepyDiscord::Activity activity)         { return convertActivity(std::move(activity)); }
    PresenceUpdate      convertObject(SleepyDiscord::PresenceUpdate presence)   { return convertPresenceUpdate(std::move(presence)); }
    Response            convertObject(SleepyDiscord::Response response)         { return convertResponse(std::move(response)); }
    VoiceState          convertObject(SleepyDiscord::VoiceState state)          { return convertVoiceState(std::move(state)); }
    VoiceServerUpdate   convertObject(SleepyDiscord::VoiceServerUpdate update)  { return convertVoiceServerUpdate(std::move(update)); }
    Reaction            convertObject(SleepyDiscord::Reaction reaction)         { return convertReaction(std::move(reaction)); }
    Invite              convertObject(SleepyDiscord::Invite invite)             { return convertInvite(std::move(invite)); }
    
    template<class FromType, class ToType>
    ArrayResponse<ToType> convertArrayResponse(SleepyDiscord::ArrayResponse<FromType> response)
    {
        std::vector<FromType> array = response.vector();
        std::vector<ToType> list;
        list.reserve(array.size());
        for (auto it = array.begin(), ite = array.end();it != ite;++it)
            list.push_back(convertObject(*it));
        return ArrayResponse<ToType>(std::move(convertResponse(response)),std::move(list));
    }
};

#endif

