#ifndef _FARAGE_ENGINE_CONVERT_
#define _FARAGE_ENGINE_CONVERT_

namespace Farage
{
    struct EditMessage : public SleepyDiscord::IdentifiableDiscordObject<Farage::EditMessage>
    {
        public:
            EditMessage() = default;
            //~EditMessage();
            //Message(const json::Values values);
            //Message(const std::string * rawJson);
            EditMessage(const SleepyDiscord::json::Value& json) : Farage::EditMessage(SleepyDiscord::json::fromJSON<Farage::EditMessage>(json)) {}
            EditMessage(const nonstd::string_view& json) : Farage::EditMessage(SleepyDiscord::json::fromJSON<Farage::EditMessage>(json)) {}
            //using DiscordObject::DiscordObject;
            std::size_t length() { return content.length(); }

            SleepyDiscord::Snowflake<SleepyDiscord::Channel> channelID;
            SleepyDiscord::Snowflake<SleepyDiscord::Server> serverID;
            SleepyDiscord::User author;
            SleepyDiscord::ServerMember member;
            std::string content;
            std::string timestamp;
            std::string editedTimestamp;
            bool tts = false;
            bool mentionEveryone = false;
            std::vector<SleepyDiscord::User> mentions;
            std::vector<SleepyDiscord::Snowflake<SleepyDiscord::User>> mentionRoles;
            std::vector<SleepyDiscord::Attachment> attachments;
            std::vector<SleepyDiscord::Embed> embeds;
            std::vector<SleepyDiscord::Reaction> reactions;
            bool pinned = false;
            SleepyDiscord::Snowflake<SleepyDiscord::Webhook> webhookID;
            enum MessageType {
                DEFAULT                = 0,
                RECIPIENT_ADD          = 1,
                RECIPIENT_REMOVE       = 2,
                CALL                   = 3,
                CHANNEL_NAME_CHANGE    = 4,
                CHANNEL_ICON_CHANGE    = 5,
                CHANNEL_PINNED_MESSAGE = 6,
                GUILD_MEMBER_JOIN      = 7
            } type = DEFAULT;
            SleepyDiscord::MessageReference reference;

            //const static std::initializer_list<const char*const> fields;
            JSONStructStart
                std::make_tuple(
                    SleepyDiscord::json::pair                           (&Farage::EditMessage::ID             , "id"              , SleepyDiscord::json::OPTIONAL_FIELD        ),
                    SleepyDiscord::json::pair                           (&Farage::EditMessage::channelID      , "channel_id"      , SleepyDiscord::json::OPTIONAL_FIELD        ),
                    SleepyDiscord::json::pair                           (&Farage::EditMessage::serverID       , "guild_id"        , SleepyDiscord::json::OPTIONAL_FIELD         ),
                    SleepyDiscord::json::pair                           (&Farage::EditMessage::author         , "author"          , SleepyDiscord::json::OPTIONAL_FIELD        ),
                    SleepyDiscord::json::pair                           (&Farage::EditMessage::content        , "content"         , SleepyDiscord::json::OPTIONAL_FIELD        ),
                    SleepyDiscord::json::pair                           (&Farage::EditMessage::member         , "member"          , SleepyDiscord::json::OPTIONAL_FIELD         ),
                    SleepyDiscord::json::pair                           (&Farage::EditMessage::timestamp      , "timestamp"       , SleepyDiscord::json::OPTIONAL_FIELD        ),
                    SleepyDiscord::json::pair                           (&Farage::EditMessage::editedTimestamp, "edited_timestamp", SleepyDiscord::json::NULLABLE_FIELD         ),
                    SleepyDiscord::json::pair                           (&Farage::EditMessage::tts            , "tts"             , SleepyDiscord::json::OPTIONAL_FIELD        ),
                    SleepyDiscord::json::pair                           (&Farage::EditMessage::mentionEveryone, "mention_everyone", SleepyDiscord::json::OPTIONAL_FIELD        ),
                    SleepyDiscord::json::pair<SleepyDiscord::json::ContainerTypeHelper>(&Farage::EditMessage::mentions       , "mentions"        , SleepyDiscord::json::OPTIONAL_FIELD        ),
                    SleepyDiscord::json::pair<SleepyDiscord::json::ContainerTypeHelper>(&Farage::EditMessage::mentionRoles   , "mention_roles"   , SleepyDiscord::json::OPTIONAL_FIELD        ),
                    SleepyDiscord::json::pair<SleepyDiscord::json::ContainerTypeHelper>(&Farage::EditMessage::attachments    , "attachments"     , SleepyDiscord::json::OPTIONAL_FIELD        ),
                    SleepyDiscord::json::pair<SleepyDiscord::json::ContainerTypeHelper>(&Farage::EditMessage::embeds         , "embeds"          , SleepyDiscord::json::OPTIONAL_FIELD        ),
                    SleepyDiscord::json::pair<SleepyDiscord::json::ContainerTypeHelper>(&Farage::EditMessage::reactions      , "reactions"       , SleepyDiscord::json::OPTIONAL_FIELD         ),
                    SleepyDiscord::json::pair                           (&Farage::EditMessage::pinned         , "pinned"          , SleepyDiscord::json::OPTIONAL_FIELD        ),
                    SleepyDiscord::json::pair                           (&Farage::EditMessage::webhookID      , "webhook_id"      , SleepyDiscord::json::OPTIONAL_FIELD         ),
                    SleepyDiscord::json::pair<SleepyDiscord::json::EnumTypeHelper     >(&Farage::EditMessage::type           , "type"            , SleepyDiscord::json::OPTIONAL_FIELD        ),
                    SleepyDiscord::json::pair                           (&Farage::EditMessage::reference      , "message_reference", SleepyDiscord::json::OPTIONAL_FIELD         )
                );
            JSONStructEnd
    };
    
    User convertUser(SleepyDiscord::User user)
    {
        return User{
            std::move(user.ID),
            std::move(user.username),
            std::move(user.discriminator),
            std::move(user.avatar),
            std::move(user.bot),
            std::move(user.mfa_enabled),
            std::move(user.verified),
            std::move(user.email)
        };
    }
    
    Overwrite convertOverwrite(SleepyDiscord::Overwrite overwrite)
    {
        return Overwrite{
            std::move(overwrite.ID),
            static_cast<int>(overwrite.type),
            std::move(Farage::Permission(overwrite.allow)),
            std::move(Farage::Permission(overwrite.deny))
        };
    }
    
    Channel convertChannel(SleepyDiscord::Channel channel)
    {
        Channel fchannel = {
            std::move(channel.ID),
            std::move(channel.type),
            std::move(channel.serverID),
            std::move(channel.position),
            std::vector<Overwrite>(channel.permissionOverwrites.size()),
            std::move(channel.name),
            std::move(channel.topic),
            std::move(channel.isNSFW),
            std::move(channel.lastMessageID),
            std::move(channel.bitrate),
            std::move(channel.userLimit),
            std::vector<User>(channel.recipients.size()),
            std::move(channel.icon),
            std::move(channel.ownerID),
            std::move(channel.parentID),
            std::move(std::move(channel.lastPinTimestamp))
        };
        {
            auto itt = fchannel.permission_overwrites.begin();
            for (auto it = channel.permissionOverwrites.begin(), ite = channel.permissionOverwrites.end();it != ite;++it,++itt)
                *itt = convertOverwrite(std::move(*it));
        }
        {
            auto itt = fchannel.recipients.begin();
            for (auto it = channel.recipients.begin(), ite = channel.recipients.end();it != ite;++it,++itt)
                *itt = convertUser(std::move(*it));
        }
        return std::move(fchannel);
    }
    
    Role convertRole(SleepyDiscord::Role role)
    {
        return Role{
            std::move(role.ID),
            std::move(role.name),
            std::move(role.color),
            std::move(role.hoist),
            std::move(role.position),
            std::move(role.permissions),
            std::move(role.managed),
            std::move(role.mentionable)
        };
    }
    
    ServerMember convertServerMember(SleepyDiscord::ServerMember member)
    {
        ServerMember fmember = {
            std::move(convertUser(std::move(member.user))),
            std::move(member.nick),
            std::vector<std::string>(member.roles.size()),
            std::move(member.joinedAt),
            std::move(member.deaf),
            std::move(member.mute)
        };
        fmember.roles.assign(member.roles.begin(),member.roles.end());
        return std::move(fmember);
    }
    
    Server convertServer(SleepyDiscord::Server server)
    {
        Server fserver = {
            std::move(server.ID),
            std::move(server.name),
            std::move(server.icon),
            std::move(server.splash),
            std::move(server.ownerID),
            std::move(server.permissions),
            std::move(server.region),
            std::move(server.AFKchannelID),
            std::move(server.AFKTimeout),
            std::move(server.embedEnable),
            std::move(server.embedChannelID),
            std::move(server.verificationLevel),
            std::move(server.defaultMessageNotifications),
            std::vector<Role>(server.roles.size()),
            std::move(server.MFALevel),
            std::move(server.joinedAt),
            std::move(server.large),
            std::move(server.unavailable),
            std::vector<ServerMember>(server.members.size()),
            std::vector<Channel>(server.channels.size())
        };
        {
            auto itt = fserver.roles.begin();
            for (auto it = server.roles.begin(), ite = server.roles.end();it != ite;++it,++itt)
                *itt = std::move(convertRole(std::move(*it)));
        }
        {
            auto itt = fserver.members.begin();
            for (auto it = server.members.begin(), ite = server.members.end();it != ite;++it,++itt)
                *itt = std::move(convertServerMember(std::move(*it)));
        }
        {
            auto itt = fserver.channels.begin();
            for (auto it = server.channels.begin(), ite = server.channels.end();it != ite;++it,++itt)
                *itt = std::move(convertChannel(std::move(*it)));
        }
        return std::move(fserver);
    }
    
    Emoji convertEmoji(SleepyDiscord::Emoji emoji)
    {
        std::vector<std::string> roles;
        roles.reserve(emoji.roles.size());
        for (auto it = emoji.roles.begin(), ite = emoji.roles.end();it != ite;++it)
            roles.push_back(std::move(*it));
        Emoji femoji(
            std::move(emoji.ID),
            std::move(emoji.name),
            std::move(roles),
            std::move(convertUser(std::move(emoji.user))),
            emoji.requireColons,
            emoji.managed,
            emoji.animated
        );
        return std::move(femoji);
    }
    
    Reaction convertReaction(SleepyDiscord::Reaction reaction)
    {
        return Reaction{ std::move(reaction.count), std::move(reaction.me), std::move(convertEmoji(std::move(reaction.emoji))) };
    }
    
    Ready convertReady(SleepyDiscord::Ready readyData)
    {
        Ready fready = {
            std::move(readyData.v),
            std::move(convertUser(std::move(readyData.user))),
            std::vector<Channel>(readyData.privateChannels.size()),
            std::vector<std::string>(readyData.servers.size()),
            std::move(readyData.sessionID)
        };
        {
            auto itt = fready.private_channels.begin();
            for (auto it = readyData.privateChannels.begin(), ite = readyData.privateChannels.end();it != ite;++it,++itt)
                *itt = std::move(convertChannel(std::move(*it)));
        }
        {
            auto itt = fready.guilds.begin();
            for (auto it = readyData.servers.begin(), ite = readyData.servers.end();it != ite;++it,++itt)
                itt->assign(std::move(it->ID));
        }
        //fready.guilds.assign(readyData.servers.begin(),readyData.servers.end());
        return std::move(fready);
    }
    
    ActivityAssets convertActivityAssets(SleepyDiscord::ActivityAssets assets)
    {
        return ActivityAssets{ std::move(assets.largeImage), std::move(assets.largeText), std::move(assets.smallImage), std::move(assets.smallText) };
    }
    
    ActivitySecrets convertActivitySecrets(SleepyDiscord::ActivitySecrets secrets)
    {
        return ActivitySecrets{ std::move(secrets.join), std::move(secrets.spectate), std::move(secrets.match) };
    }
    
    Activity convertActivity(SleepyDiscord::Activity activity)
    {
        return Activity{
            std::move(activity.name),
            std::move(activity.type),
            std::move(activity.url),
            std::pair<std::time_t,std::time_t>(std::move(activity.timestamps.start),std::move(activity.timestamps.end)),
            std::move(activity.applicationID),
            std::move(activity.details),
            std::move(activity.state),
            std::move(convertActivityAssets(std::move(activity.assets))),
            std::move(convertActivitySecrets(std::move(activity.secrets))),
            std::move(activity.instance),
            std::move(activity.flags)
        };
    }
    
    PresenceUpdate convertPresenceUpdate(SleepyDiscord::PresenceUpdate presence)
    {
        PresenceUpdate fpresence = {
            std::move(convertUser(presence.user)),
            std::move(presence.serverID),
            std::move(presence.status),
            std::vector<Activity>(presence.activities.size())
        };
        {
            auto itt = fpresence.activities.begin();
            for (auto it = presence.activities.begin(), ite = presence.activities.end();it != ite;++it,++itt)
                *itt = std::move(convertActivity(std::move(*it)));
        }
        return std::move(fpresence);
    }
    
    Response convertResponse(SleepyDiscord::Response response)
    {
        Response fresponse;
        fresponse.statusCode = response.statusCode;
        fresponse.text = std::move(response.text);
        for (auto& h : response.header)
            fresponse.header.emplace(std::move(h));
        return fresponse;//Response(std::move(response.statusCode),std::move(response.text),std::move(response.header));
    }
    
    VoiceState convertVoiceState(SleepyDiscord::VoiceState state)
    {
        return VoiceState{ std::move(state.serverID), std::move(state.channelID), std::move(state.userID), std::move(state.sessionID), std::move(state.deaf), std::move(state.mute), std::move(state.selfDeaf), std::move(state.selfMute), std::move(state.suppress) };
    }
    
    VoiceRegion convertVoiceRegion(SleepyDiscord::VoiceRegion region)
    {
        return VoiceRegion{ std::move(region.ID), std::move(region.name), std::move(region.vip), std::move(region.optimal), std::move(region.deprecated), std::move(region.custom) };
    }
    
    VoiceServerUpdate convertVoiceServerUpdate(SleepyDiscord::VoiceServerUpdate update)
    {
        return VoiceServerUpdate{ std::move(update.token), std::move(update.serverID), std::move(update.endpoint) };
    }
    
    Invite convertInvite(SleepyDiscord::Invite invite)
    {
        return Invite{ std::move(invite.code), std::move(convertServer(std::move(invite.server))), std::move(convertChannel(std::move(invite.channel))) };
    }
    
    ServerWidget convertServerWidget(SleepyDiscord::ServerWidget embed)
    {
        return ServerWidget{ std::move(embed.enabled), std::move(embed.channelID) };
    }
    
    Attachment convertAttachment(SleepyDiscord::Attachment attach)
    {
        return Attachment{ std::move(attach.ID), std::move(attach.filename), std::move(attach.size), std::move(attach.url), std::move(attach.proxy_url), std::move(attach.height), std::move(attach.width) };
    }
    
    EmbedFooter convertEmbedFooter(SleepyDiscord::EmbedFooter embed)
    {
        return EmbedFooter{ std::move(embed.text), std::move(embed.iconUrl), std::move(embed.proxyIconUrl) };
    }
    
    EmbedImage convertEmbedImage(SleepyDiscord::EmbedImage embed)
    {
        return EmbedImage{ std::move(embed.url), std::move(embed.proxyUrl), std::move(embed.height), std::move(embed.width) };
    }
    
    EmbedImage convertEmbedThumbnail(SleepyDiscord::EmbedThumbnail embed)
    {
        return EmbedImage{ std::move(embed.url), std::move(embed.proxyUrl), std::move(embed.height), std::move(embed.width) };
    }
    
    EmbedVideo convertEmbedVideo(SleepyDiscord::EmbedVideo embed)
    {
        return EmbedVideo{ std::move(embed.url), std::move(embed.height), std::move(embed.width) };
    }
    
    EmbedProvider convertEmbedProvider(SleepyDiscord::EmbedProvider embed)
    {
        return EmbedProvider{ std::move(embed.name), std::move(embed.url) };
    }
    
    EmbedAuthor convertEmbedAuthor(SleepyDiscord::EmbedAuthor embed)
    {
        return EmbedAuthor{ std::move(embed.name), std::move(embed.url), std::move(embed.iconUrl), std::move(embed.proxyIconUrl) };
    }
    
    EmbedField convertEmbedField(SleepyDiscord::EmbedField embed)
    {
        return EmbedField{ std::move(embed.name), std::move(embed.value), std::move(embed.isInline) };
    }
    
    Embed convertEmbed(SleepyDiscord::Embed embed)
    {
        Embed fembed = {
            std::move(embed.title),
            std::move(embed.type),
            std::move(embed.description),
            std::move(embed.url),
            std::move(embed.timestamp),
            std::move(embed.color),
            std::move(convertEmbedFooter(std::move(embed.footer))),
            std::move(convertEmbedImage(std::move(embed.image))),
            std::move(convertEmbedThumbnail(std::move(embed.thumbnail))),
            std::move(convertEmbedVideo(std::move(embed.video))),
            std::move(convertEmbedProvider(std::move(embed.provider))),
            std::move(convertEmbedAuthor(std::move(embed.author))),
            std::vector<EmbedField>(embed.fields.size()),
        };
        auto itt = fembed.fields.begin();
        for (auto it = embed.fields.begin(), ite = embed.fields.end();it != ite;++it,++itt)
            *itt = std::move(convertEmbedField(std::move(*it)));
        return std::move(fembed);
    }
    
    SleepyDiscord::EmbedFooter convertEmbedFooter(EmbedFooter embed)
    {
        SleepyDiscord::EmbedFooter ret;
        ret.text = std::move(embed.text);
        ret.iconUrl = std::move(embed.icon_url);
        ret.proxyIconUrl = std::move(embed.proxy_icon_url);
        return std::move(ret);
    }
    
    SleepyDiscord::EmbedImage convertEmbedImage(EmbedImage embed)
    {
        SleepyDiscord::EmbedImage ret;
        ret.url = std::move(embed.url);
        ret.proxyUrl = std::move(embed.proxy_url);
        ret.height = std::move(embed.height);
        ret.width = std::move(embed.width);
        return std::move(ret);
    }
    
    SleepyDiscord::EmbedThumbnail convertEmbedThumbnail(EmbedImage embed)
    {
        SleepyDiscord::EmbedThumbnail ret;
        ret.url = std::move(embed.url);
        ret.proxyUrl = std::move(embed.proxy_url);
        ret.height = std::move(embed.height);
        ret.width = std::move(embed.width);
        return std::move(ret);
    }
    
    SleepyDiscord::EmbedVideo convertEmbedVideo(EmbedVideo embed)
    {
        SleepyDiscord::EmbedVideo ret;
        ret.url = std::move(embed.url);
        ret.height = std::move(embed.height);
        ret.width = std::move(embed.width);
        return std::move(ret);
    }
    
    SleepyDiscord::EmbedProvider convertEmbedProvider(EmbedProvider embed)
    {
        SleepyDiscord::EmbedProvider ret;
        ret.name = std::move(embed.name);
        ret.url = std::move(embed.url);
        return std::move(ret);
    }
    
    SleepyDiscord::EmbedAuthor convertEmbedAuthor(EmbedAuthor embed)
    {
        SleepyDiscord::EmbedAuthor ret;
        ret.name = std::move(embed.name);
        ret.url = std::move(embed.url);
        ret.iconUrl = std::move(embed.icon_url);
        ret.proxyIconUrl = std::move(embed.proxy_icon_url);
        return std::move(ret);
    }
    
    SleepyDiscord::EmbedField convertEmbedField(EmbedField embed)
    {
        SleepyDiscord::EmbedField ret;
        ret.name = std::move(embed.name);
        ret.value = std::move(embed.value);
        ret.isInline = std::move(embed._inline);
        return std::move(ret);
    }
    
    SleepyDiscord::Embed convertEmbed(Embed embed)
    {
        SleepyDiscord::Embed ret;
        ret.title = std::move(embed.title);
        ret.type = std::move(embed.type);
        ret.description = std::move(embed.description);
        ret.url = std::move(embed.url);
        ret.timestamp = std::move(embed.timestamp);
        ret.color = std::move(embed.color);
        ret.footer = std::move(convertEmbedFooter(std::move(embed.footer)));
        ret.image = std::move(convertEmbedImage(std::move(embed.image)));
        ret.thumbnail = std::move(convertEmbedThumbnail(std::move(embed.thumbnail)));
        ret.video = std::move(convertEmbedVideo(std::move(embed.video)));
        ret.provider = std::move(convertEmbedProvider(std::move(embed.provider)));
        ret.author = std::move(convertEmbedAuthor(std::move(embed.author)));
        ret.fields.reserve(embed.fields.size());
        auto itt = ret.fields.begin();
        for (auto it = embed.fields.begin(), ite = embed.fields.end();it != ite;++it,++itt)
            *itt = std::move(convertEmbedField(std::move(*it)));
        return std::move(ret);
    }
    
    MessageReference convertMessageReference(SleepyDiscord::MessageReference ref)
    {
        MessageReference fref = {
            std::move(ref.messageID),
            std::move(ref.channelID),
            std::move(ref.serverID)
        };
        return std::move(fref);
    }
    SleepyDiscord::MessageReference convertMessageReference(MessageReference ref)
    {
        SleepyDiscord::MessageReference ret;
        ret.messageID = std::move(ref.message_id);
        ret.channelID = std::move(ref.channel_id);
        ret.serverID = std::move(ref.guild_id);
        return std::move(ret);
    }
    
    Message convertMessage(SleepyDiscord::Message message)
    {
        Message fmessage = {
            std::move(message.ID),
            std::move(message.channelID),
            std::move(message.serverID),
            std::move(convertUser(std::move(message.author))),
            std::move(convertServerMember(std::move(message.member))),
            std::move(message.content),
            std::move(message.timestamp),
            std::move(message.editedTimestamp),
            std::move(message.tts),
            std::move(message.mentionEveryone),
            std::vector<User>(message.mentions.size()),
            std::vector<std::string>(message.mentionRoles.size()),
            std::vector<Attachment>(message.attachments.size()),
            std::vector<Embed>(message.embeds.size()),
            std::vector<Reaction>(message.reactions.size()),
            std::move(message.pinned),
            std::move(message.webhookID),
            std::move(message.type),
            std::move(convertMessageReference(std::move(message.messageReference)))
        };
        {
            auto itt = fmessage.mentions.begin();
            for (auto it = message.mentions.begin(), ite = message.mentions.end();it != ite;++it,++itt)
                *itt = std::move(convertUser(std::move(*it)));
        }
        fmessage.mention_roles.assign(message.mentionRoles.begin(),message.mentionRoles.end());
        {
            auto itt = fmessage.attachments.begin();
            for (auto it = message.attachments.begin(), ite = message.attachments.end();it != ite;++it,++itt)
                *itt = std::move(convertAttachment(std::move(*it)));
        }
        {
            auto itt = fmessage.embeds.begin();
            for (auto it = message.embeds.begin(), ite = message.embeds.end();it != ite;++it,++itt)
                *itt = std::move(convertEmbed(std::move(*it)));
        }
        {
            auto itt = fmessage.reactions.begin();
            for (auto it = message.reactions.begin(), ite = message.reactions.end();it != ite;++it,++itt)
                *itt = std::move(convertReaction(std::move(*it)));
        }
        return std::move(fmessage);
    }
    Message convertMessage(Farage::EditMessage message)
    {
        Message fmessage = {
            std::move(message.ID),
            std::move(message.channelID),
            std::move(message.serverID),
            std::move(convertUser(std::move(message.author))),
            std::move(convertServerMember(std::move(message.member))),
            std::move(message.content),
            std::move(message.timestamp),
            std::move(message.editedTimestamp),
            std::move(message.tts),
            std::move(message.mentionEveryone),
            std::vector<User>(message.mentions.size()),
            std::vector<std::string>(message.mentionRoles.size()),
            std::vector<Attachment>(message.attachments.size()),
            std::vector<Embed>(message.embeds.size()),
            std::vector<Reaction>(message.reactions.size()),
            std::move(message.pinned),
            std::move(message.webhookID),
            std::move(message.type),
            std::move(convertMessageReference(std::move(message.reference)))
        };
        {
            auto itt = fmessage.mentions.begin();
            for (auto it = message.mentions.begin(), ite = message.mentions.end();it != ite;++it,++itt)
                *itt = std::move(convertUser(std::move(*it)));
        }
        fmessage.mention_roles.assign(message.mentionRoles.begin(),message.mentionRoles.end());
        {
            auto itt = fmessage.attachments.begin();
            for (auto it = message.attachments.begin(), ite = message.attachments.end();it != ite;++it,++itt)
                *itt = std::move(convertAttachment(std::move(*it)));
        }
        {
            auto itt = fmessage.embeds.begin();
            for (auto it = message.embeds.begin(), ite = message.embeds.end();it != ite;++it,++itt)
                *itt = std::move(convertEmbed(std::move(*it)));
        }
        {
            auto itt = fmessage.reactions.begin();
            for (auto it = message.reactions.begin(), ite = message.reactions.end();it != ite;++it,++itt)
                *itt = std::move(convertReaction(std::move(*it)));
        }
        return std::move(fmessage);
    }
                                                                                
    inline User                convertObject(SleepyDiscord::User user)                 { return convertUser(std::move(user)); }
    inline Overwrite           convertObject(SleepyDiscord::Overwrite overwrite)       { return convertOverwrite(std::move(overwrite)); }
    inline Channel             convertObject(SleepyDiscord::Channel channel)           { return convertChannel(std::move(channel)); }
    inline Role                convertObject(SleepyDiscord::Role role)                 { return convertRole(std::move(role)); }
    inline ServerMember        convertObject(SleepyDiscord::ServerMember member)       { return convertServerMember(std::move(member)); }
    inline Server              convertObject(SleepyDiscord::Server server)             { return convertServer(std::move(server)); }
    inline MessageReference    convertObject(SleepyDiscord::MessageReference ref)      { return convertMessageReference(std::move(ref)); }
    inline Message             convertObject(SleepyDiscord::Message message)           { return convertMessage(std::move(message)); }
    inline Ready               convertObject(SleepyDiscord::Ready readyData)           { return convertReady(std::move(readyData)); }
    inline Emoji               convertObject(SleepyDiscord::Emoji emoji)               { return convertEmoji(std::move(emoji)); }
    inline ActivityAssets      convertObject(SleepyDiscord::ActivityAssets assets)     { return convertActivityAssets(std::move(assets)); }
    inline ActivitySecrets     convertObject(SleepyDiscord::ActivitySecrets secrets)   { return convertActivitySecrets(std::move(secrets)); }
    inline Activity            convertObject(SleepyDiscord::Activity activity)         { return convertActivity(std::move(activity)); }
    inline PresenceUpdate      convertObject(SleepyDiscord::PresenceUpdate presence)   { return convertPresenceUpdate(std::move(presence)); }
    inline Response            convertObject(SleepyDiscord::Response response)         { return convertResponse(std::move(response)); }
    inline VoiceState          convertObject(SleepyDiscord::VoiceState state)          { return convertVoiceState(std::move(state)); }
    inline VoiceServerUpdate   convertObject(SleepyDiscord::VoiceServerUpdate update)  { return convertVoiceServerUpdate(std::move(update)); }
    inline Reaction            convertObject(SleepyDiscord::Reaction reaction)         { return convertReaction(std::move(reaction)); }
    inline Invite              convertObject(SleepyDiscord::Invite invite)             { return convertInvite(std::move(invite)); }
    inline VoiceRegion         convertObject(SleepyDiscord::VoiceRegion region)        { return convertVoiceRegion(std::move(region)); }
    inline ServerWidget        convertObject(SleepyDiscord::ServerWidget embed)        { return convertServerWidget(std::move(embed)); }
    inline Attachment          convertObject(SleepyDiscord::Attachment attach)         { return convertAttachment(std::move(attach)); }
    inline Embed               convertObject(SleepyDiscord::Embed embed)               { return convertEmbed(std::move(embed)); }
    
    template<class FromType, class ToType>
    ArrayResponse<ToType> convertArrayResponse(SleepyDiscord::ArrayResponse<FromType> response)
    {
        std::vector<FromType> array = response.vector();
        std::vector<ToType> list;
        list.reserve(array.size());
        for (auto it = array.begin(), ite = array.end();it != ite;++it)
            list.push_back(std::move(convertObject(std::move(*it))));
        return ArrayResponse<ToType>(std::move(convertResponse(response)),std::move(list));
    }
};

#endif

