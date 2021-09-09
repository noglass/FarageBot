#ifndef _FARAGE_DISCORD_CONTAINERS_
#define _FARAGE_DISCORD_CONTAINERS_

#include <string>
#include <ctime>
#include <map>
#include <vector>
#include <unordered_map>
#include <utility>

namespace Farage
{
    enum Permission : int64_t
    {
        CREATE_INSTANT_INVITE = 0x00000001, //Allows creation of instant invites
        KICK_MEMBERS    /**/  = 0x00000002, //Allows kicking members
        BAN_MEMBERS     /**/  = 0x00000004, //Allows banning members
        ADMINISTRATOR   /**/  = 0x00000008, //Allows all permissions and bypasses channel permission overwrites
        MANAGE_CHANNELS /**/  = 0x00000010, //Allows management and editing of channels
        MANAGE_GUILD    /**/  = 0x00000020, //Allows management and editing of the guild
        ADD_REACTIONS         = 0x00000040, //Allows for the addition of reactions to messages
        VIEW_AUDIT_LOG        = 0x00000080, //Allows for viewing of audit logs
        VIEW_CHANNEL          = 0x00000400, //Allows guild members to view a channel
        SEND_MESSAGES         = 0x00000800, //Allows for sending messages in a channel.
        SEND_TTS_MESSAGES     = 0x00001000, //Allows for sending of /tts messages
        MANAGE_MESSAGES /**/  = 0x00002000, //Allows for deletion of other users messages
        EMBED_LINKS           = 0x00004000, //Links sent by this user will be auto - embedded
        ATTACH_FILES          = 0x00008000, //Allows for uploading images and files
        READ_MESSAGE_HISTORY  = 0x00010000, //Allows for reading of message history
        MENTION_EVERYONE      = 0x00020000, //Allows for using the @everyone tag to notify all users in a channel, and the @here tag to notify all online users in a channel
        USE_EXTERNAL_EMOJIS   = 0x00040000, //Allows the usage of custom emojis from other servers
        CONNECT               = 0x00100000, //Allows for joining of a voice channel
        SPEAK                 = 0x00200000, //Allows for speaking in a voice channel
        MUTE_MEMBERS          = 0x00400000, //Allows for muting members in a voice channel
        DEAFEN_MEMBERS        = 0x00800000, //Allows for deafening of members in a voice channel
        MOVE_MEMBERS          = 0x01000000, //Allows for moving of members between voice channels
        USE_VAD               = 0x02000000, //Allows for using voice - activity - detection in a voice channel
        PRIORITY_SPEAKER      = 0x00000100, //Allows for using priority speaker in a voice channel
        CHANGE_NICKNAME       = 0x04000000, //Allows for modification of own nickname
        MANAGE_NICKNAMES      = 0x08000000, //Allows for modification of other users nicknames
        MANAGE_ROLES    /**/  = 0x10000000, //Allows management and editing of roles
        MANAGE_WEBHOOKS /**/  = 0x20000000, //Allows management and editing of webhooks
        MANAGE_EMOJIS   /**/  = 0x40000000, //Allows management and editing of emojis
        //              /**/ These permissions require the owner account to use two-factor authentication when used on a guild that has server-wide 2FA enabled.

        NONE                  = 0x000000000, //this permission doens't exist, I made it up
        ALL                   = 0xFFFFFFFFF,

        READ_MESSAGES = VIEW_CHANNEL,
    };
    
    struct User
    {
        //void *raw;
        std::string id;
        std::string username;
        std::string discriminator;
        std::string avatar;
        bool bot;
        bool mfa_enabled;
        //std::string locale;
        bool verified;
        std::string email;
        std::string banner;
        size_t accent_color;
    };
    
    struct Role
    {
        std::string id;
        std::string name;
        int color;
        bool hoist;
        int position;
        uint64_t permissions;
        bool managed;
        bool mentionable;
    };
    
    struct Emoji
    {
        Emoji() : custom(false) {}
        Emoji(std::string _name)
        {
            animated = false;
            if (_name.substr(0,2) == "a:")
            {
                animated = true;
                _name.erase(0,2);
            }
            else if (_name.front() == ':')
                _name.erase(0,1);
            size_t pos = _name.find(':');
            if (pos != std::string::npos)
            {
                name = _name.substr(0,pos);
                id = _name.substr(pos+1);
                custom = true;
            }
            else
                name = _name;
        }
        Emoji(const std::string &_name, const std::string &_id, bool anim = false) : name(_name), id(_id), animated(anim)
        {
            if (id.size() > 0)
                custom = true;
            else
                custom = false;
        }
        Emoji(std::string _id, std::string _name, std::vector<std::string>&& _roles, User&& _user, bool _require_colons, bool _managed, bool _animated) :
            id(std::move(_id)), name(std::move(_name)), roles(std::move(_roles)), user(std::move(_user)), require_colons(_require_colons), managed(_managed), animated(_animated)
        {
            if (id.size() > 0)
                custom = true;
            else
                custom = false;
        }
        std::string id;
        std::string name;
        std::vector<std::string> roles;
        User user;
        bool require_colons;
        bool managed;
        bool animated;
        bool custom;
        inline bool operator==(const Emoji &r) { return ((id.size() > 0) ? (id == r.id) : (name == r.name)); }
        inline bool operator!=(const Emoji &r) { return !operator==(r); }
        std::string display() const
        {
            std::string out = name;
            if (id.size() > 0)
                out = (animated ? "<a:" : "<:") + out + ':' + id + '>';
            return std::move(out);
        }
        std::string encoded() const
        {
            std::string out;
            if (id.size() > 0)
                out = (animated ? "a:" : std::string(":")) + name + ':' + id;
            else
            {
                std::string hex;
                unsigned char n;
                for (auto it = name.begin(), ite = name.end();it != ite;++it)
                {
                    n = *it;
                    do
                    {
                        hex += "0123456789ABCDEF"[n & 15];
                        n >>= 4;
                    } while (n);
                    out.append("%" + std::string(hex.rbegin(),hex.rend()));
                    hex.clear();
                }
            }
            return std::move(out);
        }
    };
    
    struct Reaction
    {
        int count;
        bool me;
        Emoji emoji;
    };
    
    struct ServerMember
    {
        User user;
        std::string nick;
        std::vector<std::string> roles;
        std::string joined_at;
        bool deaf;
        bool mute;
    };
    
    struct Attachment
    {
        std::string id;
        std::string filename;
        uint64_t size;
        std::string url;
        std::string proxy_url;
        uint64_t height;
        uint64_t width;
    };
    
    struct EmbedFooter
    {
        std::string text;
        std::string icon_url;
        std::string proxy_icon_url;
    };
    
    struct EmbedImage
    {
        std::string url;
        std::string proxy_url;
        int64_t height = 0;
        int64_t width = 0;
    };
    
    struct EmbedVideo
    {
        std::string url;
        int64_t height = 0;
        int64_t width = 0;
    };
    
    struct EmbedProvider
    {
        std::string name;
        std::string url;
    };
    
    struct EmbedAuthor
    {
        std::string name;
        std::string url;
        std::string icon_url;
        std::string proxy_icon_url;
    };
    
    struct EmbedField
    {
        std::string name;
        std::string value;
        bool _inline;
    };
    
    struct Embed
    {
        std::string title;
        std::string type;
        std::string description;
        std::string url;
        std::string timestamp;
        int32_t color;
        EmbedFooter footer;
        EmbedImage image;
        EmbedImage thumbnail;
        EmbedVideo video;
        EmbedProvider provider;
        EmbedAuthor author;
        std::vector<EmbedField> fields;
    };
    
    struct MessageReference
    {
        std::string message_id;
        std::string channel_id;
        std::string guild_id;
    };
    
    struct Message
    {
        //void *raw;
        std::string id;
        std::string channel_id;
        std::string guild_id;
        User author;
        ServerMember member;
        std::string content;
        std::string timestamp;
        std::string edited_timestamp;
        bool tts;
        bool mention_everyone;
        std::vector<User> mentions;
        std::vector<std::string> mention_roles;
        std::vector<Attachment> attachments;
        std::vector<Embed> embeds;
        std::vector<Reaction> reactions;
        bool pinned;
        std::string webhook_id;
        int type;
        MessageReference message_reference;
    };
    
    struct Overwrite
    {
        std::string id;
        int type;
        Permission allow;
        Permission deny;
    };
    
    struct Channel
    {
        //void *raw;
        std::string id;
        int type;
        std::string guild_id;
        int position;
        std::vector<Overwrite> permission_overwrites;
        std::string name;
        std::string topic;
        bool nsfw;
        std::string last_message_id;
        int bitrate;
        int user_limit;
        std::vector<User> recipients;
        std::string icon;
        std::string owner_id;
        //std::string application_id;
        std::string parent_id;
        std::string last_pin_timestamp;
    };
    
    struct Server
    {
        std::string id;
        std::string name;
        std::string icon;
        std::string splash;
        //bool owner;
        std::string owner_id;
        uint64_t permissions;
        std::string region;
        std::string afk_channel_id;
        int afk_timeout;
        bool embed_enabled;
        std::string embed_channel_id;
        int verification_level;
        int default_message_notifications;
        //int explicit_content_filter;
        std::vector<Role> roles;
        //std::vector<Emoji> emojis;
        int mfa_level;
        //std::string application_id;
        std::string joined_at;
        bool large;
        bool unavailable;
        std::vector<ServerMember> members;
        std::vector<Channel> channels;
    };
    
    struct Ready
    {
        int v;
        User user;
        std::vector<Channel> private_channels;
        std::vector<std::string> guilds;
        std::string session_id;
    };
    
    struct ActivityAssets
    {
        std::string large_image;
        std::string large_text;
        std::string small_image;
        std::string small_text;
    };
    
    struct ActivitySecrets
    {
        std::string join;
        std::string spectate;
        std::string match;
    };
    
    struct Activity
    {
        std::string name;
        int type;
        std::string url;
        std::pair<std::time_t,std::time_t> timestamps;
        std::string application_id;
        std::string details;
        std::string state;
        ActivityAssets assets;
        ActivitySecrets secrets;
        bool instance;
        int flags;
    };
    
    struct PresenceUpdate
    {
        User user;
        std::string guild_id;
        std::string status;
        std::vector<Activity> activities;
    };
    
    struct Response
    {
        int32_t statusCode;
        std::string text;
        std::map<std::string,std::string> header;
        inline bool error() const
        {
            return 400 <= statusCode;
        }
        Response() {}
        Response(int32_t _statusCode) : statusCode(_statusCode) {}
        Response(int32_t _statusCode, std::string _text, std::map<std::string,std::string> _header)
        {
            statusCode = _statusCode;
            text = _text;
            header = _header;
        }
    };
    
    struct VoiceState
    {
        std::string guild_id;
        std::string channel_id;
        std::string user_id;
        std::string session_id;
        bool deaf;
        bool mute;
        bool self_deaf;
        bool self_mute;
        bool suppress;
    };
    
    struct VoiceRegion
    {
        std::string id;
        std::string name;
        bool vip;
        bool optimal;
        bool deprecated;
        bool custom;
    };
    
    struct VoiceServerUpdate
    {
        std::string token;
        std::string guild_id;
        std::string endpoint;
    };
    
    struct Invite
    {
        std::string code;
        Server server;
        Channel channel;
    };
    
    struct ServerWidget
    {
        bool enabled;
        std::string channel_id;
    };
    
    template<class T>
    struct ObjectResponse
    {
        ObjectResponse(Response resp, T obj) : response(std::move(resp)), object(std::move(obj)) {}
        ObjectResponse() {}
        
        Response response;
        T object;
    };
    
    struct BoolResponse
    {
        BoolResponse(Response resp, bool success) : response(std::move(resp)), result(success) {}
        BoolResponse() {}
        
        Response response;
        bool result;
    };
    
    template<class T>
    struct ArrayResponse
    {
        ArrayResponse(Response resp, std::vector<T> list) : response(std::move(resp)), array(std::move(list)) {}
        ArrayResponse(Response resp) : response(std::move(resp)) {}
        ArrayResponse() {}
        
        Response response;
        std::vector<T> array;
    };
    
    enum GetMessagesKey {na, around, before, after, limit};
    
    struct Info
    {
        std::string name;
        std::string author;
        std::string description;
        std::string version;
        std::string url;
        std::string API_VER;
    };
};

#endif

