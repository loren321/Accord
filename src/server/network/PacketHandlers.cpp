#include <accordserver/network/PacketHandlers.h>

#include <accordserver/log/Logger.h>
#include <accordserver/Server.h>
#include <accordserver/Authentication.h>
#include <accordserver/thread/Thread.h>
#include <accordserver/util/CryptoUtil.h>

#include <accordshared/network/packet/SerializationPacket.h>
#include <accordshared/network/packet/TokenPacket.h>
#include <accordshared/network/packet/ErrorPacket.h>
#include <accordshared/network/packet/KeepAlivePacket.h>
#include <accordshared/error/ErrorCodes.h>
#include <accordshared/types/Request.h>
#include <accordshared/types/Return.h>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <Magick++.h>

#include <iostream>

namespace accord {
namespace network {

util::FunctionMap PacketHandlers::serializationMap = {
    { types::ADD_COMMUNITY_REQUEST, &PacketHandlers::handleAddCommunityRequest },
    { types::COMMUNITIES_TABLE_REQUEST, &PacketHandlers::handleCommunitiesTable },
    { types::CHANNELS_REQUEST, &PacketHandlers::handleChannels },
    { types::AUTH_WITH_TOKEN_REQUEST, &PacketHandlers::handleTokenAuth },
    { types::MESSAGES_REQUEST, &PacketHandlers::handleMessagesRequest },
    { types::SEND_MESSAGE_REQUEST, &PacketHandlers::handleSubmitMessage },
    { types::ADD_CHANNEL_REQUEST, &PacketHandlers::handleAddChannel },
    { types::USER_REQUEST, &PacketHandlers::handleUser },
    { types::SEND_INVITE_REQUEST, &PacketHandlers::handleSendInvite },
    { types::INVITE_REQUEST, &PacketHandlers::handleGenInvite },
    { types::ONLINE_LIST_REQUEST, &PacketHandlers::handleOnlineList }
};

bool checkLoggedIn(thread::Client *client, const std::string &token)
{
    if (!Authentication::checkToken(token)) {
        network::ErrorPacket packet;
        const auto msg = packet.construct(AUTH_ERR);
        client->write(msg);
        return false;
    }

    if (!client->user.table) {
        network::ErrorPacket packet;
        const auto msg = packet.construct(NOT_LOGGED_IN_ERR);
        client->write(msg);
        return false;
    }
    return true;
}

bool PacketHandlers::receiveSendMessagePacket(const std::vector<char> &body, PacketData *data)
{
	thread::Client *client = (thread::Client*) data;
    //client->server.broadcast(std::string(body.begin(), body.end()), client->channel);
    log::Logger::log(log::DEBUG, "Received send message packet with body: " +
                     std::string(body.begin(), body.end()));
	return true;
}

bool PacketHandlers::receiveErrorPacket(const std::vector<char> &body, PacketData *data)
{
    log::Logger::log(log::DEBUG, "Received error packet with body: " +
                     std::string(body.begin(), body.end()));
	return true;
}

bool PacketHandlers::receiveDisconnectPacket(const std::vector<char> &body, PacketData *data)
{
	thread::Client *client = (thread::Client*) data;
	client->thread.removeClient(client);
	log::Logger::log(log::DEBUG, "Removed client");
	return true;
}

bool PacketHandlers::receiveAuthPacket(const std::vector<char> &body, PacketData *data)
{
    //get arguments
    thread::Client *client = (thread::Client*) data;
    std::string bodyString(body.begin(), body.end());
    std::vector<std::string> strings;
    boost::split(strings, bodyString, boost::is_any_of(
                 std::string(1, (char)0x3)),
                 boost::token_compress_on);
    if (strings.size() != 2) {
        network::ErrorPacket packet;
        const auto msg = packet.construct(ARGS_ERR);
        client->write(msg);
        return false;
    }
    const auto token = Authentication::authUser(client->thread.database,
                                                 strings[0], strings[1]);
    //authenticate the user
    if (token.token.length() != TOKEN_LEN) {
        network::ErrorPacket packet;
        const auto msg = packet.construct(AUTH_ERR);
        client->write(msg);
        return false;
    }
    client->user = client->thread.database.getUser(strings[0]);
    log::Logger::log(log::DEBUG, "Successfully authenticated client!");

    auto channelList = client->thread.database.getChannelsForUser(client->user.id());
    for (auto channel : channelList) {
        types::UserData userData(client->user.id(), client->user.name(), "");
        client->server.registerOnlineMember(channel.id(), userData, client);
        client->channelList.push_back(channel.id());
    }
    client->server.notifyStatusChange(client->user.id(), client);

    //send token to client
    network::SerializationPacket packet;
    const auto json = util::Serialization::serialize(token);
    const auto msg = packet.construct(types::AUTH_REQUEST, json);
    client->write(msg);

    return true;
}

bool PacketHandlers::receiveRegisterPacket(const std::vector<char> &body, PacketData *data)
{
    thread::Client *client = (thread::Client*) data;
    std::string bodyString(body.begin(), body.end());
    std::vector<std::string> strings;
    boost::split(strings, bodyString, boost::is_any_of(
                 std::string(1, (char)0x3)),
                 boost::token_compress_on);
     if (strings.size() != 3)
        return false;
    bool ret = Authentication::registerUser(client->thread.database, strings[0],
                                 strings[1], strings[2]);
    if (!ret) {
        network::ErrorPacket packet;
        const auto msg = packet.construct(REGIST_ERR);
        client->write(msg);
        return false;
    }

    const auto token = Authentication::authUser(client->thread.database,
                                                 strings[0], strings[2]);
    client->user = client->thread.database.getUser(strings[0]);
    network::SerializationPacket packet;
    const auto json = util::Serialization::serialize(token);
    std::vector<char> message = packet.construct(types::AUTH_REQUEST, json);
    client->write(message);

    return true;
}

bool PacketHandlers::receiveNoopPacket(const std::vector<char> &body, PacketData *data)
{
    thread::Client *client = (thread::Client*) data;
    log::Logger::log(log::WARNING, "Received packet NOOP from " + client->user.name());
    return true;
}

bool PacketHandlers::receiveSerializationPacket(const std::vector<char> &body, PacketData *data)
{
    return util::Serialization::receive(serializationMap, body, data);
}

bool PacketHandlers::receiveKeepAlivePacket(const std::vector<char> &body,
                                            PacketData *data)
{
    (void)body; /* supress not-used warnings */
    auto client = (thread::Client*) data;
    network::KeepAlivePacket packet;
    const auto msg = packet.construct();
    client->write(msg);
    return true;
}


bool PacketHandlers::handleCommunitiesTable(PacketData *data, const std::vector<char> &body)
{
    const auto request = util::Serialization::deserealize<
            types::Communities>(body);
    thread::Client *client = (thread::Client*) data;
    if (!Authentication::checkToken(request.token)) {
        network::ErrorPacket packet;
        const auto msg = packet.construct(AUTH_ERR);
        client->write(msg);
        return false;
    }

    if (!client->user.table) {
        network::ErrorPacket packet;
        const auto msg = packet.construct(NOT_LOGGED_IN_ERR);
        client->write(msg);
        return false;
    }

    const auto communities = client->thread.database.getCommunitiesForUser(
                client->user.id());
    std::vector<types::CommunitiesTable> shared;
    for (auto community : communities)
        shared.push_back(database::Database::communityServerToShared(community));

    network::SerializationPacket packet;
    const auto json = util::Serialization::serialize(shared);
    const auto msg = packet.construct(types::COMMUNITIES_TABLE_REQUEST, json);
    client->write(msg);
    return true;
}

bool PacketHandlers::handleAddCommunityRequest(PacketData *data, const std::vector<char> &body)
{
    types::AddCommunity request = util::Serialization::deserealize<
            types::AddCommunity>(body);
    thread::Client *client = (thread::Client*) data;

    if (!Authentication::checkToken(request.token)) {
        network::ErrorPacket packet;
        const auto msg = packet.construct(AUTH_ERR);
        client->write(msg);
        return false;
    }

    if (!client->user.table) {
        network::ErrorPacket packet;
        const auto msg = packet.construct(NOT_LOGGED_IN_ERR);
        client->write(msg);
        return false;
    }

    //compress the image to a 200x200 jpg first and put it in the request
    auto &profilepic = request.profilepic;
    log::Logger::log(log::DEBUG, "profilepic.size(): " + std::to_string(profilepic.size()));
    Magick::Blob blob(&profilepic[0], profilepic.size());
    Magick::Image image(blob);
    image.resize("200x200");
    image.magick("JPEG");
    image.write(&blob);
    profilepic.clear();
    const auto imageData = (const char*) blob.data();
    const auto imageLength = blob.length();
    std::copy(imageData, imageData + imageLength, std::back_inserter(profilepic));
    log::Logger::log(log::DEBUG, "profilepic.size(): " + std::to_string(profilepic.size()));

    uint64_t communityId = util::CryptoUtil::getRandomUINT64();
    database::table_communities community;
    if (!client->thread.database.initCommunity(communityId,
                                               client->user.id(),
                                               request, &community)) {
        network::ErrorPacket packet;
        const auto msg = packet.construct(REQUEST_ERR);
        client->write(msg);
        return false;
    }

    //we added a community, now let's send it back so that they can add it to their list
    types::CommunitiesTable table = database::Database::communityServerToShared(community);
    network::SerializationPacket packet;
    auto json = util::Serialization::serialize(table);
    auto msg = packet.construct(types::COMMUNITY_TABLE_REQUEST, json);
    client->write(msg);

    //done for now
    return true;
}

bool PacketHandlers::handleChannels(PacketData *data,
                                    const std::vector<char> &body)
{
    thread::Client *client = (thread::Client*) data;
    const auto request = util::Serialization::deserealize<
            types::Channels>(body);
    /* check if token is valid */
    if (!Authentication::checkToken(request.token)) {
        network::ErrorPacket packet;
        const auto msg = packet.construct(AUTH_ERR);
        client->write(msg);
        return false;
    }

    if (!client->user.table) {
        network::ErrorPacket packet;
        const auto msg = packet.construct(NOT_LOGGED_IN_ERR);
        client->write(msg);
        return false;
    }

    const auto &community = request.community;

    /* check if user is in community
     * we don't want random users querying for channels
     */
    if (!client->thread.database.isUserInCommunity(
                client->user.id(), community)) {
        network::ErrorPacket packet;
        const auto msg = packet.construct(FORBIDDEN_ERR);
        client->write(msg);
        return false;
    }
    const auto channels = client->thread.database.getChannelsForCommunity(
                community);

    types::ChannelsReturn ret;
    ret.community = community;
    for (auto channel : channels)
        ret.channels.push_back(database::Database::channelServerToShared(channel));

    network::SerializationPacket packet;
    const auto json = util::Serialization::serialize(ret);
    const auto msg = packet.construct(types::CHANNELS_REQUEST, json);
    client->write(msg);

    return true;
}

bool PacketHandlers::handleTokenAuth(PacketData *data, const std::vector<char> &body)
{
    auto client = (thread::Client*) data;
    const auto request = util::Serialization::deserealize<
            types::Token>(body);
    if (!Authentication::authUser(request)) {
        network::ErrorPacket packet;
        const auto msg = packet.construct(AUTH_ERR);
        client->write(msg);
        return false;
    }

    /* token is valid we can log the user in */
    client->user = client->thread.database.getUser(
                request.id);
    return true;
}

bool PacketHandlers::handleMessagesRequest(PacketData *data, const std::vector<char> &body)
{
    auto client = (thread::Client*) data;
    const auto request = util::Serialization::deserealize<
            types::Messages>(body);

    if (!checkLoggedIn(client, request.token))
        return false;

    if (!client->thread.database.canUserViewChannel(client->user.id(),
                                                    request.channel)) {
        network::ErrorPacket packet;
        const auto msg = packet.construct(FORBIDDEN_ERR);
        client->write(msg);
        return false;
    }

    const auto messages = client->thread.database.getMessagesForChannel(
                request.channel);
    types::MessagesReturn ret;
    ret.channel = request.channel;
    for (auto message : messages)
        ret.messages.push_back(database::Database::messageServerToShared
                               (message));

    network::SerializationPacket packet;
    const auto json = util::Serialization::serialize(ret);
    const auto msg = packet.construct(types::MESSAGES_REQUEST, json);
    client->write(msg);
    return true;
}

bool PacketHandlers::handleSubmitMessage(PacketData *data,
                                         const std::vector<char> &body)
{
    auto client = (thread::Client*) data;
    const auto request = util::Serialization::deserealize<
            types::SendMessage>(body);
    if (!checkLoggedIn(client, request.token))
        return false;
    database::table_messages message;
    if (!client->thread.database.submitMessage(request.channel,
                                               client->user.id(),
                                               request.message,
                                               request.timestamp,
                                               &message)) {
        network::ErrorPacket packet;
        const auto msg = packet.construct(REQUEST_ERR);
        client->write(msg);
        return false;
    }

    network::SerializationPacket packet;
    auto json = util::Serialization::serialize(types::MessageSuccess(
                                                         message.id()));
    auto msg = packet.construct(types::MESSAGE_SUCCESS, json);
    client->write(msg);

    types::MessagesTable toSend = database::Database::messageServerToShared(message);
    json = util::Serialization::serialize(toSend);
    msg = packet.construct(types::MESSAGE_REQUEST, json);
    client->server.broadcast(msg);
    return true;
}

bool PacketHandlers::handleAddChannel(PacketData *data,
                                      const std::vector<char> &body)
{
    auto client = (thread::Client*) data;
    const auto request = util::Serialization::deserealize<
            types::AddChannel>(body);
    if (!checkLoggedIn(client, request.token))
        return false;

    database::table_channels channel;
    const auto id = util::CryptoUtil::getRandomUINT64();
    if (!client->thread.database.initChannel(id, request, &channel)) {
        network::ErrorPacket packet;
        const auto msg = packet.construct(REQUEST_ERR);
        client->write(msg);
        return false;
    }

    network::SerializationPacket packet;
    types::ChannelsTable table = database::Database::channelServerToShared(
                channel);
    const auto json = util::Serialization::serialize(table);
    const auto msg = packet.construct(types::CHANNEL_REQUEST, json);
    client->write(msg);
    return true;
}

bool PacketHandlers::handleUser(PacketData *data,
                                const std::vector<char> &body)
{
    auto client = (thread::Client*) data;
    const auto request = util::Serialization::deserealize<
            types::User>(body);
    if (!checkLoggedIn(client, request.token))
        return false;
    /*
     * TODO: determine if we want to check if user is associated
     *  with requestor
     */
    database::table_users user = client->thread.database.getUser(request.id);
    if (!user.table) {
        network::ErrorPacket packet;
        const auto msg = packet.construct(REQUEST_ERR);
        client->write(msg);
        return false;
    }

    types::UserData ret(user.id(), user.name(), "");
    network::SerializationPacket packet;
    const auto json = util::Serialization::serialize(ret);
    const auto msg = packet.construct(types::USER_REQUEST, json);
    client->write(msg);
    return true;
}

bool PacketHandlers::handleSendInvite(PacketData *data,
                                      const std::vector<char> &body)
{
    auto client = (thread::Client*) data;
    const auto invite = util::Serialization::deserealize<std::string>(body);
    if (!client->user.table) {
        network::ErrorPacket packet;
        const auto msg = packet.construct(NOT_LOGGED_IN_ERR);
        client->write(msg);
        return false;
    }
    log::Logger::log(log::DEBUG, invite);

    if (!client->thread.isInviteValid(invite)) {
        log::Logger::log(log::DEBUG, "invite handeling prematurely aborted");
        network::ErrorPacket packet;
        const auto msg = packet.construct(NOT_FOUND);
        client->write(msg);
        return false;
    }

    const auto communityId = client->thread.getCommunityForInvite(invite);
    if (client->thread.database.isUserInCommunity(client->user.id(),
                                                  communityId)) {
        network::ErrorPacket packet;
        const auto msg = packet.construct(ALREADY_IN_ERR);
        client->write(msg);
        return false;
    }

    if (!client->thread.database.addMember(communityId, client->user.id())) {
        network::ErrorPacket packet;
        const auto msg = packet.construct(REQUEST_ERR);
        client->write(msg);
        return false;
    }

    types::CommunitiesTable ret = database::Database::communityServerToShared(
                client->thread.database.getCommunity(communityId));
    network::SerializationPacket packet;
    const auto json = util::Serialization::serialize(ret);
    const auto msg = packet.construct(types::COMMUNITY_TABLE_REQUEST, json);
    client->write(msg);
    return true;
}

bool PacketHandlers::handleGenInvite(PacketData *data,
                                     const std::vector<char> &body)
{
    auto client = (thread::Client*) data;
    const auto request = util::Serialization::deserealize<
            types::Invite>(body);
    if (!checkLoggedIn(client, request.token))
        return false;

    /* TODO: permission and stuff q.q */
    if (!client->thread.database.isUserInCommunity(
                client->user.id(), request.id)) {
        network::ErrorPacket packet;
        const auto msg = packet.construct(FORBIDDEN_ERR);
        client->write(msg);
        return false;
    }

    auto invite = client->thread.genInvite(request.id);
    types::InviteRet ret(request.id, invite);
    network::SerializationPacket packet;
    const auto json = util::Serialization::serialize(ret);
    const auto msg = packet.construct(types::INVITE_REQUEST, json);
    client->write(msg);
    return true;
}

bool PacketHandlers::handleOnlineList(PacketData *data,
                                      const std::vector<char> &body)
{
    auto client = (thread::Client*) data;
    const auto request = util::Serialization::deserealize<
            types::OnlineList>(body);
    if (!checkLoggedIn(client, request.token))
        return false;

    /* TODO: permission and stuff q.q like checking
     * if they are in the channel */

    const auto onlineList = client->server.getOnlineList(request.id);
    types::OnlineListRet ret(onlineList, request.id);
    network::SerializationPacket packet;
    const auto json = util::Serialization::serialize(ret);
    const auto msg = packet.construct(types::ONLINE_LIST_REQUEST, json);
    client->write(msg);
    return true;
}

} /* namespace network */
} /* namespace accord */
