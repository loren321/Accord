#ifndef ACCORD_UTIL_CRYPTO_UTIL_H
#define ACCORD_UTIL_CRYPTO_UTIL_H

#include <vector>
#include <string>
#include <stddef.h>

namespace accord {
namespace util {

class CryptoUtil {
public:
    static std::vector<unsigned char> getRandomBytes(int length);
    static std::string getRandomString(int length);
    static uint64_t getRandomUINT64();
};

} /* namespace util */
} /* namespace accord */

#endif