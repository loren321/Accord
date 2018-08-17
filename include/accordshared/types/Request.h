#ifndef ACCORD_TYPES_REQUEST_H
#define ACCORD_TYPES_REQUEST_H

#include <string>
#include <vector>

#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

namespace accord {
namespace types {

class AddCommunity {
public:
    AddCommunity() { }
    AddCommunity(std::string name, std::vector<char> profilepic)
        : name(name), profilepic(profilepic) { }
    std::string name;
    std::vector<char> profilepic;

    template<class Archive>
    void serialize(Archive &archive)
    {
        archive(name, profilepic);
    }
};

} /* namespace types */
} /* namespace accord */

#endif
