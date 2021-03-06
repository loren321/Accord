#include <accordserver/database/Database.h>
#include <accordserver/log/Logger.h>
#include <accordserver/util/CryptoUtil.h>

namespace accord {
namespace database {

sql_create_8(users, 1, 8,
             mysqlpp::sql_bigint_unsigned, id,
             mysqlpp::sql_varchar, name,
             mysqlpp::sql_blob_null, profilepic,
             mysqlpp::sql_int, friends,
             mysqlpp::sql_int, communities,
             mysqlpp::sql_varchar, email,
             mysqlpp::sql_varchar, password,
             mysqlpp::sql_varchar, salt);

sql_create_4(friends, 1, 4,
             mysqlpp::sql_bigint_unsigned, id,
             mysqlpp::sql_bigint_unsigned, user1,
             mysqlpp::sql_bigint_unsigned, user2,
             mysqlpp::sql_enum, status);

sql_create_5(communities, 1, 5,
             mysqlpp::sql_bigint_unsigned, id,
             mysqlpp::sql_varchar, name,
             mysqlpp::sql_blob_null, profilepic,
             mysqlpp::sql_int, members,
             mysqlpp::sql_int, channels);

sql_create_2(community_members, 1, 2,
             mysqlpp::sql_bigint_unsigned, id,
             mysqlpp::sql_bigint_unsigned, user);

sql_create_4(channels, 1, 4,
             mysqlpp::sql_bigint_unsigned, id,
             mysqlpp::sql_bigint_unsigned, community,
             mysqlpp::sql_varchar, name,
             mysqlpp::sql_varchar, description);

sql_create_2(channel_members, 1, 2,
             mysqlpp::sql_bigint_unsigned, id,
             mysqlpp::sql_bigint_unsigned, user);

sql_create_5(messages, 1, 5,
             mysqlpp::sql_bigint_unsigned, id,
             mysqlpp::sql_bigint_unsigned, channel,
             mysqlpp::sql_bigint_unsigned, sender,
             mysqlpp::sql_varchar, contents,
             mysqlpp::sql_bigint_unsigned, timestamp);

table_users::table_users(std::shared_ptr<users> table) : table(table)
{}

mysqlpp::sql_bigint_unsigned& table_users::id()
{
    return table->id;
}

mysqlpp::sql_varchar& table_users::name()
{
    return table->name;
}
mysqlpp::sql_blob_null& table_users::profilepic()
{
    return table->profilepic;
}

mysqlpp::sql_int& table_users::friends()
{
    return table->friends;
}

mysqlpp::sql_int& table_users::communities()
{
    return table->communities;
}

mysqlpp::sql_varchar& table_users::email()
{
    return table->email;
}

mysqlpp::sql_varchar& table_users::password()
{
    return table->password;
}

mysqlpp::sql_varchar& table_users::salt()
{
    return table->salt;
}

table_friends::table_friends(std::shared_ptr<friends> table) : table(table) { }

mysqlpp::sql_bigint_unsigned& table_friends::id()
{
    return table->id;
}

mysqlpp::sql_bigint_unsigned& table_friends::user1()
{
    return table->user1;
}

mysqlpp::sql_bigint_unsigned& table_friends::user2()
{
    return table->user2;
}

mysqlpp::sql_enum& table_friends::status()
{
    return table->status;
}

table_communities::table_communities(std::shared_ptr<communities> table)
    : table(table) { }

mysqlpp::sql_bigint_unsigned& table_communities::id()
{
    return table->id;
}

mysqlpp::sql_varchar& table_communities::name()
{
    return table->name;
}

mysqlpp::sql_blob_null& table_communities::profilepic()
{
    return table->profilepic;
}

mysqlpp::sql_int& table_communities::members()
{
    return table->members;
}

mysqlpp::sql_int& table_communities::channels()
{
    return table->channels;
}

table_community_members::table_community_members(
        std::shared_ptr<community_members> table) : table(table) { }

mysqlpp::sql_bigint_unsigned& table_community_members::id()
{
    return table->id;
}

mysqlpp::sql_bigint_unsigned& table_community_members::user()
{
    return table->user;
}

table_channels::table_channels(
        std::shared_ptr<channels> table) : table(table) { }

mysqlpp::sql_bigint_unsigned &table_channels::id()
{
    return table->id;
}

mysqlpp::sql_bigint_unsigned &table_channels::community()
{
    return table->community;
}

mysqlpp::sql_varchar &table_channels::name()
{
    return table->name;
}

mysqlpp::sql_varchar &table_channels::description()
{
    return table->description;
}

table_channel_members::table_channel_members(std::shared_ptr<channel_members>
                                             table) : table(table) { }

mysqlpp::sql_bigint_unsigned &table_channel_members::id()
{
    return table->id;
}

mysqlpp::sql_bigint_unsigned &table_channel_members::user()
{
    return table->user;
}

table_messages::table_messages(std::shared_ptr<messages> table)
    : table(table) { }

mysqlpp::sql_bigint_unsigned &table_messages::id()
{
    return table->id;
}

mysqlpp::sql_bigint_unsigned &table_messages::channel()
{
    return table->channel;
}

mysqlpp::sql_bigint_unsigned &table_messages::sender()
{
    return table->sender;
}

mysqlpp::sql_varchar &table_messages::contents()
{
    return table->contents;
}

mysqlpp::sql_bigint_unsigned &table_messages::timestamp()
{
    return table->timestamp;
}

Database::Database(const DatabaseOptions &options)
    : options(options)
{

}

Database::~Database()
{
    if (connection.connected())
        connection.disconnect();
}

int Database::connect()
{
    connection.set_option(new mysqlpp::ReconnectOption(true));
    return connection.connect(options.name.c_str(), options.address.c_str(),
                              options.user.c_str(),
                       options.password.c_str(), options.port);
}

void Database::disconnect()
{
    connection.disconnect();
}

bool Database::verify()
{
    mysqlpp::Query query = connection.query("SHOW TABLES LIKE 'users'");
    mysqlpp::StoreQueryResult res;
    if (res = query.store())
        if (res.empty())
            return false;
    query = connection.query("SHOW TABLES LIKE 'communities'");
    if (res = query.store())
        if (res.empty())
            return false;
    query = connection.query("SHOW TABLES LIKE 'friends'");
    if (res = query.store())
        if (res.empty())
            return false;
    query = connection.query("SHOW TABLES LIKE 'community_members'");
    if (res = query.store())
        if (res.empty())
            return false;
    query = connection.query("SHOW TABLES LIKE 'channels'");
    if (res = query.store())
        if (res.empty())
            return false;
    query = connection.query("SHOW TABLES LIKE 'channel_members'");
    if (res = query.store())
        if (res.empty())
            return false;
    query = connection.query("SHOW TABLES LIKE 'messages'");
    if (res = query.store())
        if (res.empty())
            return false;
    return true;
}

bool Database::initDatabase()
{
    try {
        mysqlpp::Query query = connection.
                query("CREATE TABLE users (id BIGINT UNSIGNED, "
                      "name VARCHAR(255), profilepic "
                      "VARBINARY(65535), friends INT,"
                      "communities INT, email VARCHAR(255),"
                      "password VARCHAR(255), salt VARCHAR(255))");
        if (!query.execute())
            return false;
        query = connection.query(
                "CREATE TABLE communities (id BIGINT UNSIGNED,"
                "name VARCHAR(255), profilepic VARBINARY(65535),"
                "members INT, channels INT)");
        if (!query.execute())
            return false;
        query = connection.query(
                "CREATE TABLE friends (id BIGINT UNSIGNED, "
                "user1 BIGINT UNSIGNED, user2 BIGINT UNSIGNED,"
                "status ENUM('pending', 'accepted'))");
        if (!query.execute())
            return false;
        query = connection.query(
                "CREATE TABLE community_members (id BIGINT UNSIGNED, "
                "user BIGINT UNSIGNED)");
        if (!query.execute())
            return false;
        query = connection.query(
                    "CREATE TABLE channels (id BIGINT UNSIGNED, "
                    "community BIGINT UNSIGNED, name VARCHAR(255),"
                    "description VARCHAR(255))");
        if (!query.execute())
            return false;
        query = connection.query(
                    "CREATE TABLE channel_members (id BIGINT UNSIGNED, "
                    "user BIGINT UNSIGNED)");
        if (!query.execute())
            return false;
        query = connection.query(
                    "CREATE TABLE messages (id BIGINT UNSIGNED, "
                    "channel BIGINT UNSIGNED, sender BIGINT UNSIGNED,"
                    " contents VARCHAR(2000),"
                    "timestamp BIGINT UNSIGNED)");
        if (!query.execute())
            return false;
    } catch (mysqlpp::BadQuery &e) {
        log::Logger::log(log::WARNING, "Bad query made in database init: " +
                         std::string(e.what()));
    }
    return true;
}

mysqlpp::Query Database::query(std::string statement)
{
    return connection.query(statement.c_str());
}

bool Database::initUser(uint64_t id, const std::string &name,
                        const std::string &email,
                        const std::string &password,
                        const std::string &salt)
{

    mysqlpp::Query query = connection.query();
    users user(id, name, mysqlpp::null, 0, 0, email, password, salt);
    query.insert(user);
    return query.execute();
}

bool Database::initCommunity(uint64_t id, uint64_t user,
                             const types::AddCommunity &request,
                             table_communities *ret)
{
    table_communities check = getCommunity(id);
    if (check.table != NULL)
        return false;

    mysqlpp::Query query = connection.query();
    communities community(id, request.name,
                          vectorChartoSqlBlobNullable(request.profilepic), 0, 0);
    query.insert(community);
    if(!query.execute())
        return false;
    if(!addMember(id, user))
        return false;

    *ret = table_communities(std::make_shared<communities>(community));
    return true;
}

bool Database::initChannel(uint64_t id, const types::AddChannel &request,
                           table_channels *ret)
{
    table_channels check = getChannel(id);
    if (check.table != nullptr)
        return false;

    auto query = connection.query();
    channels channel(id, request.community, request.name, request.description);
    query.insert(channel);
    if (!query.execute())
        return false;
    if (!addChannel(request.community))
        return false;

    auto users = getUsersForCommunity(request.community);
    for (auto user : users) {
        channel_members channel_member(id, user.id());
        query.insert(channel_member);
        if (!query.execute())
            return false;
    }
    *ret = table_channels(std::make_shared<channels>(channel));
    return true;
}

bool Database::initMessage(uint64_t id, uint64_t channel, uint64_t sender,
                           const std::string&msg, uint64_t timestamp,
                           table_messages *ret)
{
    table_messages check = getMessage(id);
    if (check.table != nullptr) {
        log::Logger::log(log::WARNING, "Message already exists!");
        return false;
    }

    auto query = connection.query();
    messages message(id, channel, sender, msg, timestamp);
    query.insert(message);
    if (!query.execute())
        return false;
    *ret = table_messages(std::make_shared<messages>(message));
    return true;
}

bool Database::submitMessage(uint64_t channel, uint64_t sender,
                             const std::string &msg,
                             uint64_t timestamp, table_messages *ret)
{
    auto id = util::CryptoUtil::getRandomUINT64();
    return initMessage(id, channel, sender, msg, timestamp, ret);
}

bool Database::addMember(uint64_t id, uint64_t user)
{
    mysqlpp::Query query = connection.query();

    table_communities community = getCommunity(id);
    communities originalCommunity = *community.table;
    community.members()++;
    query.update(originalCommunity, *community.table);
    if(!query.execute())
        return false;

    table_users userTable = getUser(user);
    users originalUser = *userTable.table;
    userTable.communities()++;
    query.update(originalUser, *userTable.table);
    if (!query.execute())
        return false;

    community_members community_member(id, user);
    query.insert(community_member);
    if(!query.execute())
        return false;

    std::vector<table_channels> channels = getChannelsForCommunity(id);
    for (table_channels &channel : channels) {
        channel_members channel_member(channel.id(), user);
        query.insert(channel_member);
        if (!query.execute())
            return false;
    }

    return true;
}

bool Database::addChannel(uint64_t id)
{
    table_communities community = getCommunity(id);
    if (!community.table)
        return false;
    communities originalCommunity = *community.table;
    community.channels()++;

    auto query = connection.query();
    query.update(originalCommunity, *community.table);
    return query.execute();
}

bool Database::sendFriendRequest(uint64_t from, uint64_t to)
{
    mysqlpp::Query query = connection.query();

    uint64_t id = util::CryptoUtil::getRandomUINT64();
    friends request(id, from, to, "pending");
    query.insert(request);
    return query.execute();
}

bool Database::acceptFriendRequest(uint64_t id)
{
    mysqlpp::Query query = connection.query("SELECT * FROM friends WHERE"
                                            " id=" + std::to_string(id));
    std::vector<friends> res;
    query.storein(res);
    if (res.size() != 1) {
        //what
        log::Logger::log(log::WARNING, "Friend request id " +
                         std::to_string(id) + " has multiple entries..?");
        return false;
    }
    friends request = res[0];
    friends original = res[0];
    request.status = "accepted";
    query.update(original, request);
    return query.execute();
}

bool Database::isUserInCommunity(uint64_t userId, uint64_t communityId)
{
    auto query = connection.query("SELECT * FROM community_members WHERE"
                                  " id=" + std::to_string(communityId) +
                                  " AND user=" + std::to_string(userId));
    std::vector<community_members> res;
    query.storein(res);
    if (res.size() > 1) {
        /* there is something wrong with the database,
         * assume that the user is in there
         * and report a warning. This is non-fatal
         * no need to crash the server or inconvencience
         * the client
         */
        log::Logger::log(log::WARNING, "There are multiple entries"
                                       "for a user in community_members."
                                       "PLEASE CHECK!");
        return true;
    }
    if (res.size() == 0)
        return false;
    return true;
}

bool Database::canUserViewChannel(uint64_t userId, uint64_t channelId)
{
    auto query = connection.query("SELECT * FROM community_members WHERE id="
                                  "(SELECT community FROM channels WHERE id="
                                  + std::to_string(channelId) + ")"
                                  " AND user=" + std::to_string(userId));
    std::vector<community_members> res;
    query.storein(res);
    if (res.size() > 1) {
        /* same deal as above */
        log::Logger::log(log::WARNING, "There are multiple entries"
                                       "for a user in community_members."
                                       "PLEASE CHECK!");
        return true;
    }
    if (res.size() == 0)
        return false;
    return true;
}

table_users Database::getUser(const std::string &login)
{
    mysqlpp::Query query = connection.query("SELECT * FROM users WHERE"
                                          " name='" + login + "' OR"
                                          " email='" + login + "'");
    std::vector<users> res;
    query.storein(res);
    if (res.size() != 1) {
        log::Logger::log(log::WARNING, "Login " + login + " has multiple"
                                                          " entries!");
        return table_users(NULL);
    }
    auto user = std::make_shared<users>(res[0]);
    table_users table(user);
    return table;
}

table_users Database::getUser(uint64_t id)
{
    mysqlpp::Query query = connection.query("SELECT * FROM users WHERE"
                                          " id=" + std::to_string(id));
    std::vector<users> res;
    query.storein(res);
    if (res.size() != 1) {
        log::Logger::log(log::WARNING, "User Id " + std::to_string(id) +
                         " has multiple entries!");
        return table_users(NULL);
    }
    auto user = std::make_shared<users>(res[0]);
    table_users table(user);
    return table;
}

table_channels Database::getChannel(uint64_t id)
{
    auto query = connection.query("SELECT * FROM channels WHERE "
                                  "id=" + std::to_string(id));
    std::vector<channels> res;
    query.storein(res);
    if (res.size() != 1)
        return table_channels(nullptr);

    auto channel = std::make_shared<channels>(res[0]);
    table_channels table(channel);
    return table;
}

table_communities Database::getCommunity(uint64_t id)
{
    mysqlpp::Query query = connection.query("SELECT * FROM communities WHERE "
                                            "id=" + std::to_string(id));
    std::vector<communities> res;
    query.storein(res);
    if (res.size() != 1) {
        log::Logger::log(log::WARNING, "Community Id " + std::to_string(id) +
                         " has multiple entries!");
        return table_communities(NULL);
    }
    auto community = std::make_shared<communities>(res[0]);
    table_communities table(community);
    return table;
}

table_messages Database::getMessage(uint64_t id)
{
    auto query = connection.query("SELECT * FROM messages WHERE "
                                  "id=" + std::to_string(id));
    std::vector<messages> res;
    query.storein(res);
    if (res.size() != 1)
        return table_messages(nullptr);
    table_messages table(std::make_shared<messages>(res[0]));
    return table;
}

std::vector<table_communities> Database::getCommunitiesForUser(uint64_t id)
{
    std::vector<table_communities> ret;
    std::vector<communities> res;
    mysqlpp::Query query = connection.query("SELECT * FROM communities WHERE id IN"
                                            " (SELECT community_members.id FROM"
                                            " community_members WHERE user=" +
                                            std::to_string(id) + ");");
    query.storein(res);
    for (size_t i = 0; i < res.size(); i++) {
        auto community = std::make_shared<communities>(res[i]);
        ret.emplace_back(community);
    }
    return ret;
}

std::vector<table_channels> Database::getChannelsForCommunity(uint64_t id)
{
    std::vector<table_channels> ret;
    std::vector<channels> res;
    auto query = connection.query("SELECT * FROM channels WHERE community="
                                  + std::to_string(id));
    query.storein(res);
    for (size_t i = 0; i < res.size(); i++) {
        auto channel = std::make_shared<channels>(res[i]);
        ret.emplace_back(channel);
    }
    return ret;
}

std::vector<table_channels> Database::getChannelsForUser(uint64_t id)
{
    std::vector<table_channels> ret;
    std::vector<channels> res;
    auto query = connection.query("SELECT * FROM channels WHERE id IN"
                                  " (SELECT channel_members.id FROM "
                                  "channel_members WHERE user=" +
                                  std::to_string(id) + ");");
    query.storein(res);
    for (size_t i = 0; i < res.size(); i++) {
        auto channel = std::make_shared<channels>(res[i]);
        ret.emplace_back(channel);
    }
    return ret;
}

std::vector<table_messages> Database::getMessagesForChannel(uint64_t id)
{
    std::vector<table_messages> ret;
    std::vector<messages> res;
    auto query = connection.query("SELECT * FROM messages WHERE channel="
                                  + std::to_string(id));
    query.storein(res);
    for (auto message : res)
        ret.emplace_back(std::make_shared<messages>(message));
    return ret;
}

/*
 * currently unused, maybe useful sometime in the future?
*/
std::vector<table_users> Database::getUsersForChannel(uint64_t id)
{
    std::vector<table_users> ret;
    std::vector<users> res;
    auto query = connection.query("SELECT * FROM users WHERE id IN "
                                  "(SELECT channel_members.user FROM "
                                  "channel_members WHERE id=" +
                                  std::to_string(id) + ");");
    query.storein(res);
    for (users user : res) {
        auto user_ptr = std::make_shared<users>(user);
        ret.emplace_back(user_ptr);
    }
    return ret;
}

std::vector<table_users> Database::getUsersForCommunity(uint64_t id)
{
    std::vector<table_users> ret;
    std::vector<users> res;
    auto query = connection.query("SELECT * FROM users WHERE id IN "
                                  "(SELECT community_members.user FROM "
                                  "community_members WHERE id=" +
                                  std::to_string(id) + ");");
    query.storein(res);
    for (auto user: res) {
        auto user_ptr = std::make_shared<users>(user);
        ret.emplace_back(user_ptr);
    }
    return ret;
}

types::CommunitiesTable Database::communityServerToShared(table_communities community)
{
    return types::CommunitiesTable((uint64_t) community.id(), (std::string) community.name(),
                                   sqlBlobNullableToVectorChar(community.profilepic()),
                                   (int) community.members(),
                                   (int) community.channels());
}

types::ChannelsTable Database::channelServerToShared(table_channels channel)
{
    return types::ChannelsTable((uint64_t) channel.id(), (uint64_t)
                                channel.community(), (std::string)
                                channel.name(), (std::string)
                                channel.description());
}

types::MessagesTable Database::messageServerToShared(table_messages message)
{
    return types::MessagesTable((uint64_t) message.id(),
                                (uint64_t) message.channel(),
                                (uint64_t) message.sender(),
                                (std::string) message.contents(),
                                (uint64_t) message.timestamp());
}

std::vector<char> Database::sqlBlobNullableToVectorChar(mysqlpp::sql_blob_null blob)
{
    return blob.is_null ? std::vector<char>() : std::vector<char>(blob.data.begin(), blob.data.end());
}

mysqlpp::sql_blob_null Database::vectorChartoSqlBlobNullable(const std::vector<char> &vector)
{
    mysqlpp::String string(std::string(vector.begin(), vector.end()),
                     mysqlpp::mysql_type_info::string_type, vector.empty());
    return mysqlpp::sql_blob_null(string);
}

} /* namespace database */
} /* namespace accord */
