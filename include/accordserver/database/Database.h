#ifndef ACCORD_DATABASE_DATABASE_H
#define ACCORD_DATABASE_DATABASE_H

#include <string>
#include <mysql++.h>
#include <ssqls.h>
#include <memory>

#include <accordshared/types/Database.h>
#include <accordshared/types/Request.h>

namespace accord {
namespace database {

/*
 * mysql++ has an annoying fucking bug where you can't include generated
 * structures twice. So I create my own wrapper which circumvents this...
 */

struct users;
struct friends;
struct communities;
struct community_members;
struct channels;
struct channel_members;
struct messages;

struct table_users {
    table_users() {}
    table_users(std::shared_ptr<users> table);
    std::shared_ptr<users> table;
    mysqlpp::sql_bigint_unsigned &id();
    mysqlpp::sql_varchar &name();
    mysqlpp::sql_blob_null &profilepic();
    mysqlpp::sql_int &friends();
    mysqlpp::sql_int &communities();
    mysqlpp::sql_varchar &email();
    mysqlpp::sql_varchar &password();
    mysqlpp::sql_varchar &salt();
};

struct table_friends {
    table_friends() {}
    table_friends(std::shared_ptr<friends> table);
    std::shared_ptr<friends> table;
    mysqlpp::sql_bigint_unsigned &id();
    mysqlpp::sql_bigint_unsigned &user1();
    mysqlpp::sql_bigint_unsigned &user2();
    mysqlpp::sql_enum &status();
};

struct table_communities {
    table_communities() {}
    table_communities(std::shared_ptr<communities> table);
    std::shared_ptr<communities> table;
    mysqlpp::sql_bigint_unsigned &id();
    mysqlpp::sql_varchar &name();
    mysqlpp::sql_blob_null &profilepic();
    mysqlpp::sql_int &members();
    mysqlpp::sql_int &channels();
};

struct table_community_members {
    table_community_members() {}
    table_community_members(std::shared_ptr<community_members> table);
    std::shared_ptr<community_members> table;
    mysqlpp::sql_bigint_unsigned &id();
    mysqlpp::sql_bigint_unsigned &user();
};

struct table_channels {
    table_channels() {}
    table_channels(std::shared_ptr<channels> table);
    std::shared_ptr<channels> table;
    mysqlpp::sql_bigint_unsigned &id();
    mysqlpp::sql_bigint_unsigned &community();
    mysqlpp::sql_varchar &name();
    mysqlpp::sql_varchar &description();
};

struct table_channel_members {
    table_channel_members() {}
    table_channel_members(std::shared_ptr<channel_members> table);
    std::shared_ptr<channel_members> table;
    mysqlpp::sql_bigint_unsigned &id();
    mysqlpp::sql_bigint_unsigned &user();
};

struct table_messages {
    table_messages() {}
    table_messages(std::shared_ptr<messages> table);
    std::shared_ptr<messages> table;
    mysqlpp::sql_bigint_unsigned &id();
    mysqlpp::sql_bigint_unsigned &channel();
    mysqlpp::sql_bigint_unsigned &sender();
    mysqlpp::sql_varchar &contents();
    mysqlpp::sql_bigint_unsigned &timestamp();
};

struct DatabaseOptions {
    DatabaseOptions() {}
    DatabaseOptions(std::string name, std::string address, std::string user,
                    std::string password, int port)
        : name(name), address(address), user(user), password(password),
    port(port) {}
    std::string name, address, user, password;
    int port;
};

class Database {
public:
    Database(const DatabaseOptions &options);
    ~Database();

    int connect();
    void disconnect();
    bool verify();
    bool initDatabase();
    bool initUser(uint64_t id, const std::string &name,
                  const std::string &email,
                  const std::string &password,
                  const std::string &salt);
    bool initCommunity(uint64_t id, uint64_t user, const types::AddCommunity &request,
                       table_communities *ret = nullptr);
    bool initChannel(uint64_t id, const types::AddChannel &request,
                     table_channels *ret = nullptr);
    bool initMessage(uint64_t id, uint64_t channel,
                     uint64_t sender, const std::string &msg,
                     uint64_t timestamp, table_messages *ret = nullptr);
    bool submitMessage(uint64_t channel, uint64_t sender,
                       const std::string &msg, uint64_t timestamp,
                       table_messages *ret = nullptr);
    bool addMember(uint64_t id, uint64_t user);
    bool addChannel(uint64_t id);
    bool sendFriendRequest(uint64_t from, uint64_t to);
    bool acceptFriendRequest(uint64_t id);
    bool isUserInCommunity(uint64_t userId, uint64_t communityId);
    bool canUserViewChannel(uint64_t userId, uint64_t channelId);
    table_users getUser(const std::string &login);
    table_users getUser(uint64_t id);
    table_channels getChannel(uint64_t id);
    table_communities getCommunity(uint64_t id);
    table_messages getMessage(uint64_t id);
    std::vector<table_communities> getCommunitiesForUser(uint64_t id);
    std::vector<table_channels> getChannelsForCommunity(uint64_t id);
    std::vector<table_messages> getMessagesForChannel(uint64_t id);
    std::vector<table_channels> getChannelsForUser(uint64_t id);
    std::vector<table_users> getUsersForChannel(uint64_t id);
    std::vector<table_users> getUsersForCommunity(uint64_t id);
    mysqlpp::Query query(std::string statement);

    //helper functions
    static types::CommunitiesTable communityServerToShared(table_communities community);
    static types::ChannelsTable channelServerToShared(table_channels channel);
    static types::MessagesTable messageServerToShared(table_messages message);
    static std::vector<char> sqlBlobNullableToVectorChar(mysqlpp::sql_blob_null blob);
    static mysqlpp::sql_blob_null vectorChartoSqlBlobNullable(const std::vector<char> &vector);
private:
    const DatabaseOptions &options;
    mysqlpp::Connection connection;
};

} /* namespace database */
} /* namespace accord */

#endif
