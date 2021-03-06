#include "api/farage.h"
#include <iostream>
#include <fstream>
#include "shared/libini.h"
using namespace Farage;

#define MAKEMENTION
#include "common_func.h"

#define VERSION "v0.3.6"

extern "C" Info Module
{
    "Metta-Pins",
    "Madison",
    "Mettaton's Pinned Messages",
    VERSION,
    "http://you.justca.me/",
    FARAGE_API_VERSION
};

namespace MetaPin
{
    int pinCmd(Handle&,int,const std::string[],const Message&);
    int unpinCmd(Handle&,int,const std::string[],const Message&);
    int addPinsCmd(Handle&,int,const std::string[],const Message&);
    int cyclePinsCmd(Handle&,int,const std::string[],const Message&);
    INIObject pins;
    void addPin(Handle& handle, const std::string& guildID, const std::string& channelID, const std::string& messageID, const std::string& userID);
    void setupDelMsg(Handle& handle, const std::string& userID, const std::string& channelID, const std::string& message);
    int pinTimer(Handle& handle, Timer* timer, void* data);
    int deletePinned(Handle& handle, ReactHook* hook, const ServerMember& member, const Channel& channel, const std::string& messageID, const std::string& guildID, const Emoji& emoji)
    {
        deleteMessage(channel.id,messageID);
        return PLUGIN_ERASE | PLUGIN_HANDLED;
    }
    int pinReactHook(Handle& handle, ReactHook* hook, const ServerMember& member, const Channel& channel, const std::string& messageID, const std::string& guildID, const Emoji& emoji)
    {
        Global* global = recallGlobal();
        if (global != nullptr)
        {
            if (global->getAdminFlags(guildID,member.user.id) & PIN)
            {
                addPin(handle,guildID,channel.id,messageID,member.user.id);
                //removeReaction(channel.id,messageID,emoji.encoded(),member.user.id);
            }
        }
        return PLUGIN_HANDLED;
    }
    int shareFeedHook(Handle& handle, ReactHook* hook, const ServerMember& member, const Channel& channel, const std::string& messageID, const std::string& guildID, const Emoji& emoji)
    {
        if (member.user.id == recallGlobal()->self.id)
            return PLUGIN_HANDLED;
        ObjectResponse<Message> response = getMessage(channel.id,messageID);
        if (response.response.error())
            reactToID(channel.id,messageID,"%E2%9D%93");
        else if ((response.object.webhook_id.size() > 0) && (response.object.embeds.size() > 0))
        {
            for (auto it = response.object.reactions.begin(), ite = response.object.reactions.end();it != ite;++it)
            {
                if (it->emoji == emoji)
                {
                    if (it->count == 1)
                    {
                        sendEmbed("737115353792118846",std::move(response.object.embeds.front()),makeMention(member.user.id,guildID));
                        //removeReaction(channel.id,messageID,emoji.encoded(),member.user.id);
                        reactToID(channel.id,messageID,emoji.encoded());
                    }
                    break;
                }
            }
        }
        return PLUGIN_HANDLED;
    }
    int shareSlimFeedHook(Handle& handle, ReactHook* hook, const ServerMember& member, const Channel& channel, const std::string& messageID, const std::string& guildID, const Emoji& emoji)
    {
        if (member.user.id == recallGlobal()->self.id)
            return PLUGIN_HANDLED;
        ObjectResponse<Message> response = getMessage(channel.id,messageID);
        if (response.response.error())
            reactToID(channel.id,messageID,"%E2%9D%93");
        else if ((response.object.webhook_id.size() > 0) && (response.object.embeds.size() > 0))
        {
            for (auto it = response.object.reactions.begin(), ite = response.object.reactions.end();it != ite;++it)
            {
                if (it->emoji == emoji)
                {
                    if (it->count == 1)
                    {
                        auto embed = response.object.embeds.front();
                        std::string message = makeMention(member.user.id,guildID) + "\n> " + embed.title;
                        std::string url = embed.image.url;
                        if (url.find("ifttt.com/images/no_image_card.png") != std::string::npos)
                        {
                            url = embed.description;
                            sendMessage("737115353792118846",message + '\n' + url);
                            reactToID(channel.id,messageID,emoji.encoded());
                        }
                        else
                        {
                            std::string file = "reddit/" + embed.author.name + '_' + embed.footer.text + url.substr(0,url.find_last_of('?')).substr(url.find_last_of('.'));
                            system(("curl -s -o '" + file + "' '" + url + '\'').c_str());
                            std::ifstream image (file);
                            if (image.is_open())
                            {
                                bool err = (image.peek() == std::char_traits<char>::eof());
                                image.close();
                                if (err)
                                {
                                    std::cout<<"File contains errors!"<<std::endl;
                                    reactToID(channel.id,messageID,"%E2%9D%97");
                                }
                                else
                                {
                                    sendFile("737115353792118846",file,message);
                                    reactToID(channel.id,messageID,emoji.encoded());
                                }
                            }
                            else
                            {
                                std::cout<<"Cannot open file."<<std::endl;
                                reactToID(channel.id,messageID,"%E2%9D%97");
                            }
                        }
                    }
                    break;
                }
            }
        }
        return PLUGIN_HANDLED;
    }
    int awwRepostHook(Handle& handle, ReactHook* hook, const ServerMember& member, const Channel& channel, const std::string& messageID, const std::string& guildID, const Emoji& emoji)
    {
        if ((member.user.id == recallGlobal()->self.id) || (channel.id == "737115353792118846"))
            return PLUGIN_HANDLED;
        ObjectResponse<Message> response = getMessage(channel.id,messageID);
        if (response.response.error())
            reactToID(channel.id,messageID,"%E2%9D%93");
        else
        {
            for (auto it = response.object.reactions.begin(), ite = response.object.reactions.end();it != ite;++it)
            {
                if (it->emoji == emoji)
                {
                    if (it->count == 1)
                    {
                        std::vector<std::string> urls;
                        std::string message = makeMention(member.user.id,guildID) + " https://discordapp.com/channels/" + guildID + '/' + channel.id + '/' + messageID + "\n> " + response.object.content;
                        for (auto& att : response.object.attachments)
                            message = message + ' ' + att.url;
                        sendMessage("737115353792118846",message);
                        reactToID(channel.id,messageID,emoji.encoded());
                    }
                    break;
                }
            }
        }
        return PLUGIN_HANDLED;
    }
};

extern "C" int onModuleStart(Handle &handle, Global *global)
{
    recallGlobal(global);
    handle.createGlobVar("metapin_version",VERSION,"MetaPins Version",GVAR_CONSTANT);
    handle.regChatCmd("pin",&MetaPin::pinCmd,PIN,"Pin a message forever.");
    handle.regChatCmd("unpin",&MetaPin::unpinCmd,PIN,"Did I say forever? Just kidding!");
    handle.regChatCmd("addpins",&MetaPin::addPinsCmd,PIN,"Stock up on some pins.");
    handle.regChatCmd("cyclepins",&MetaPin::cyclePinsCmd,PIN,"Gimmie some fresh pins.");
    handle.hookReaction("pinHook",&MetaPin::pinReactHook,0,"📌");
    handle.hookReactionChannel("feedHook",&MetaPin::shareFeedHook,0,"737115927157407744","🏆");
    handle.hookReactionChannel("slimFeedHook",&MetaPin::shareSlimFeedHook,0,"737115927157407744","heartcat:737118450115412048");
    handle.hookReaction("awwRepostHook",&MetaPin::awwRepostHook,0,"🔁");
    MetaPin::pins.open("metapins.ini");
    return 0;
}

extern "C" int onMessage(Handle& handle, Event event, void* message, void* nil, void* foo, void* bar)
{
    Message *msg = (Message*)message;
    if ((msg->type == 6) && (msg->message_reference.message_id.size() > 0))
    {
        consoleOut(msg->author.username + " has pinned a message https://discordapp.com/channels/" + msg->guild_id + '/' + msg->channel_id + '/' + msg->message_reference.message_id);
        if (msg->author.id == recallGlobal()->self.id)
            deleteMessage(msg->channel_id,msg->id);
        else
        {
            size_t change = MetaPin::pins.items(msg->channel_id);
            MetaPin::pins(msg->channel_id,msg->message_reference.message_id) = "1";
            if (change != MetaPin::pins.items(msg->channel_id))
                MetaPin::pins.write("metapins.ini");
        }
    }
    else if ((msg->channel_id == "737115927157407744") && (msg->webhook_id.size() > 0) && (msg->embeds.size() > 0))
    {
        auto embed = msg->embeds.front();
        if ((embed.description.find("v.redd.it") != std::string::npos)
         || (embed.description.find("youtu.be") != std::string::npos)
         || (embed.description.find("youtube.com") != std::string::npos)
         || (embed.image.url.find("ifttt.com/images/no_image_card.png") != std::string::npos)
         || (embed.description.find(".gifv") != std::string::npos)
         || (embed.description.find(".webm") != std::string::npos))
            sendMessage(msg->channel_id,embed.description);
    }
    return PLUGIN_CONTINUE;
}

int MetaPin::pinCmd(Handle& handle, int argc, const std::string argv[], const Message& message)
{
    Global *global = recallGlobal();
    if (argc < 2)
        sendMessage(message.channel_id,"Usage: `" + global->prefix(message.guild_id) + "pin <message_id>`");
    else
    {
        ObjectResponse<Message> response = getMessage(message.channel_id,argv[1]);
        if (response.response.error())
            MetaPin::setupDelMsg(handle,message.author.id,message.channel_id,"Error: Cannot find the message `" + argv[1] + "`!");
        else
            MetaPin::addPin(handle,message.guild_id,message.channel_id,argv[1],message.author.id);
    }
    return PLUGIN_HANDLED;
}

int MetaPin::unpinCmd(Handle& handle, int argc, const std::string argv[], const Message& message)
{
    Global *global = recallGlobal();
    if (argc < 2)
        sendMessage(message.channel_id,"Usage: `" + global->prefix(message.guild_id) + "unpin <message_id>`");
    else
    {
        ObjectResponse<Message> response = getMessage(message.channel_id,argv[1]);
        if (response.response.error())
            MetaPin::setupDelMsg(handle,message.author.id,message.channel_id,"Error: Cannot find the message `" + argv[1] + "`!");
        else
        {
            try
            {
                unpinMessage(message.channel_id,argv[1]);
                MetaPin::pins.find(message.channel_id,argv[1]);
                //if (MetaPin::pins.find(message.channel_id,argv[1]).size() > 0)
                    //MetaPin::setupDelMsg(handle,message.author.id,message.channel_id,makeMention(message.author.id,message.guild_id) + " has unpinned a message https://discordapp.com/channels/" + message.guild_id + '/' + message.channel_id + '/' + argv[1]);
                MetaPin::pins.erase(message.channel_id,argv[1]);
            } catch (const std::out_of_range& err)
            {
                MetaPin::setupDelMsg(handle,message.author.id,message.channel_id,"Error: That message isn't pinned!");
                return PLUGIN_HANDLED;
            }
            auto topic = MetaPin::pins.topic_it(message.channel_id);
            if (topic->items() > 49)
            {
                int found = 0;
                for (int found = 0;found < 3;++found)
                {
                    for (auto it = topic->begin(), ite = topic->end();it != ite;++it)
                    {
                        if ((found) && (it->value == "0"))
                        {
                            MetaPin::addPin(handle,message.guild_id,message.channel_id,it->item,global->self.id);
                            found = 3;
                            break;
                        }
                        if ((!found) && (it->value == "1"))
                            found = 1;
                    }
                }
            }
            MetaPin::pins.write("metapins.ini");
            reactToID(message.channel_id,message.id,"%E2%9C%85");
        }
    }
    return PLUGIN_HANDLED;
}

int MetaPin::addPinsCmd(Handle& handle, int argc, const std::string argv[], const Message& message)
{
    Global *global = recallGlobal();
    auto pinned = getPinnedMessages(message.channel_id);
    if (pinned.response.error())
        MetaPin::setupDelMsg(handle,message.author.id,message.channel_id,"Error getting the pins! Maybe there aren't any?");
    else
    {
        auto topic = MetaPin::pins.topic_it(message.channel_id);
        if (topic != MetaPin::pins.end())
            for (auto it = topic->begin(), ite = topic->end();it != ite;++it)
                it->value = "0";
        for (auto it = pinned.array.rbegin(), ite = pinned.array.rend();it != ite;++it)
            MetaPin::pins(message.channel_id,it->id) = "1";
        MetaPin::pins.write("metapins.ini");
        MetaPin::setupDelMsg(handle,message.author.id,message.channel_id,"Successfully stocked up on " + std::to_string(MetaPin::pins.items(message.channel_id)) + " pins!");
    }
    return PLUGIN_HANDLED;
}

int MetaPin::cyclePinsCmd(Handle& handle, int argc, const std::string argv[], const Message& message)
{
    Global *global = recallGlobal();
    auto topic = MetaPin::pins.topic_it(message.channel_id);
    int rotate = 10;
    if (argc > 1)
    {
        if (std::isdigit(argv[1].at(0)))
            rotate = std::stoi(argv[1]);
        if (rotate > 25)
            rotate = 25;
        if (rotate < 1)
            rotate = 10;
    }
    if (topic == MetaPin::pins.end())
        MetaPin::setupDelMsg(handle,message.author.id,message.channel_id,"There are no pins to cycle!");
    else if (topic->items() < 51)
    {
        int count = 0;
        for (auto it = topic->begin();it != topic->end();)
        {
            if (it->value == "0")
            {
                pinMessage(message.channel_id,it->item);
                std::string item = it->item;
                it = topic->erase(it);
                (*topic)(item) = "1";
                ++count;
            }
            else
                ++it;
        }
        if (count == 0)
            MetaPin::setupDelMsg(handle,message.author.id,message.channel_id,"There are not enough pins to cycle!");
        else
        {
            MetaPin::pins.write("metapins.ini");
            MetaPin::setupDelMsg(handle,message.author.id,message.channel_id,"Repinned " + std::to_string(count) + " message(s)!");
        }
    }
    else
    {
        int need = topic->items();
        if (need < 50+rotate)
            need %= rotate;
        else
            need = rotate;
        int rem = need;
        std::vector<std::string> bottom;
        std::vector<INIObject::INIItem> top;
        auto it = topic->begin();
        for (;it != topic->end();)
        {
            if (it->value != "0")
            {
                --rem;
                unpinMessage(message.channel_id,it->item);
                top.emplace_back(it->item,"0");
                it = topic->erase(it);
            }
            else
                ++it;
            if (!rem)
                break;
        }
        int timer = 3;
        for (;it != topic->end();)
        {
            if (it->value == "0")
            {
                --need;
                //it->value = "1";
                std::pair<std::string,std::string>* data = new std::pair<std::string,std::string>(message.channel_id,it->item);
                handle.createTimer("pin",timer++,&MetaPin::pinTimer,(void*)data);
                bottom.push_back(it->item);
                it = topic->erase(it);
            }
            else
                ++it;
            if (!need)
                break;
        }
        if (need)
        {
            for (it = topic->begin();it != topic->end();)
            {
                if (it->value == "0")
                {
                    --need;
                    //it->value = "1";
                    std::pair<std::string,std::string>* data = new std::pair<std::string,std::string>(message.channel_id,it->item);
                    handle.createTimer("pin",timer++,&MetaPin::pinTimer,(void*)data);
                    bottom.push_back(it->item);
                    it = topic->erase(it);
                }
                else
                    ++it;
                if (!need)
                    break;
            }
        }
        for (auto& i : bottom)
            (*topic)(i) = "1";
        topic->insert_range(topic->begin(),top.begin(),top.end());
        MetaPin::pins.write("metapins.ini");
        MetaPin::setupDelMsg(handle,message.author.id,message.channel_id,"It shall be done.");
    }
    return PLUGIN_HANDLED;
}

int MetaPin::pinTimer(Handle& handle, Timer* timer, void* data)
{
    std::pair<std::string,std::string>* pin = (std::pair<std::string,std::string>*)data;
    pinMessage(pin->first,pin->second);
    delete pin;
    return PLUGIN_HANDLED;
}

void MetaPin::addPin(Handle& handle, const std::string& guildID, const std::string& channelID, const std::string& messageID, const std::string& userID)
{
    try
    {
        if (MetaPin::pins.find(channelID,messageID) == "1")
            return;
    } catch (const std::out_of_range& err) {}
    MetaPin::pins(channelID,messageID) = "1";
    MetaPin::pins.write("metapins.ini");
    auto topic = MetaPin::pins.topic_it(channelID);
    int seconds = 1;
    if (topic->items() > 50)
    {
        for (auto it = topic->begin(), ite = topic->end();it != ite;++it)
        {
            if (it->value == "1")
            {
                it->value = "0";
                unpinMessage(channelID,it->item);
                seconds = 4;
                break;
            }
        }
    }
    std::pair<std::string,std::string>* data = new std::pair<std::string,std::string>(channelID,messageID);
    handle.createTimer("pin",seconds,&MetaPin::pinTimer,(void*)data);
    MetaPin::setupDelMsg(handle,userID,channelID,makeMention(userID,guildID) + " has pinned a message https://discordapp.com/channels/" + guildID + '/' + channelID + '/' + messageID);
}

void MetaPin::setupDelMsg(Handle& handle, const std::string& userID, const std::string& channelID, const std::string& message)
{
    ObjectResponse<Message> resp = sendMessage(channelID,message);
    if (!resp.response.error())
        handle.hookReactionMessage(resp.object.id,&MetaPin::deletePinned,0,resp.object.id,"❌",userID);
}







