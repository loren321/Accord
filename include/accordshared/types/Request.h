#ifndef ACCORD_TYPES_REQUEST_H
#define ACCORD_TYPES_REQUEST_H

#include <string>
#include <vector>

#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

namespace accord {
namespace types {

enum RequestIds {
    COMMUNITIES_TABLE_REQUEST = 0,
    ADD_COMMUNITY_REQUEST = 1,
    CHANNELS_REQUEST = 2,
    AUTH_REQUEST = 3, //with password
    AUTH_WITH_TOKEN_REQUEST = 4, //with token
    COMMUNITY_TABLE_REQUEST = 5,
    MESSAGE_REQUEST = 6,
    MESSAGES_REQUEST = 7,
    SEND_MESSAGE_REQUEST = 8,
    MESSAGE_SUCCESS = 9, //signals that a message was received successfully
    ADD_CHANNEL_REQUEST = 10,
    CHANNEL_REQUEST = 11
};

class Communities {
public:
    Communities() { }
    Communities(const std::string &token) : token(token) { }
    std::string token;

    template<class Archive>
    void serialize(Archive &archive)
    {
        archive(token);
    }
};

class AddCommunity {
public:
    AddCommunity() { }
    AddCommunity(std::string name, std::vector<char> profilepic,
                 const std::string &token)
        : name(name), profilepic(profilepic), token(token) { }
    std::string name;
    std::vector<char> profilepic;
    std::string token;

    template<class Archive>
    void serialize(Archive &archive)
    {
        archive(name, profilepic, token);
    }
};

class AddChannel {
public:
    AddChannel() { }
    AddChannel(uint64_t community, std::string name, std::string description,
               const std::string &token)
        : community(community), name(name), description(description),
    token(token){ }
    uint64_t community;
    std::string name;
    std::string description;
    std::string token;

    template<class Archive>
    void serialize(Archive &archive)
    {
        archive(community, name, description, token);
    }
};

class Channels {
public:
    Channels() { }
    Channels(uint64_t community, const std::string &token)
        : community(community), token(token) { }
    uint64_t community;
    std::string token;

    template<class Archive>
    void serialize(Archive &archive)
    {
        archive(community, token);
    }
};

class SendMessage {
public:
    SendMessage() { }
    SendMessage(uint64_t channel, const std::string &msg,
                uint64_t timestamp, const std::string &token)
        : channel(channel), message(msg), timestamp(timestamp),
          token(token) { }
    uint64_t channel, timestamp;
    std::string message, token;

    template<class Archive>
    void serialize(Archive &archive)
    {
        archive(channel, message, timestamp, token);
    }
};

class Messages {
public:
    Messages() { }
    Messages(uint64_t channel, const std::string &token)
        : channel(channel), token(token) { }
    uint64_t channel;
    std::string token;

    template<class Archive>
    void serialize(Archive &archive)
    {
        archive(channel, token);
    }
};

} /* namespace types */
} /* namespace accord */

#endif