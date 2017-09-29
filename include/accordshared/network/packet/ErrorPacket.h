#ifndef NETWORK_PACKET_ERROR_PACKET_H
#define NETWORK_PACKET_ERROR_PACKET_H

#include <event2/bufferevent.h>

#include <accordshared/error/ErrorCodes.h>
#include <accordshared/network/Packet.h>

namespace accord {
namespace network {

class ErrorPacket : public Packet {
public:
	virtual ErrorPacket *clone() const {
		return new ErrorPacket();
	}

	int getId() const {
		return ERROR_PACKET;
	}
	
	static int dispatch(struct bufferevent *bufferEvent, Error error);

	std::string construct(Error error);
	size_t getBufferSize() const;
};

} /* namespace network */
} /* namespace accord */

#endif