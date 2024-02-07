#pragma once

#include "btstack.h"
#include <string>
#include <vector>

typedef uint8_t Byte;
typedef std::vector<Byte> ByteArray;

class BtController
{
	typedef void (*rfcomm_data_handler)(uint8_t *packet, uint16_t size);

public:
	explicit BtController(std::string name, int btClass);
	virtual ~BtController() {}

	void poll();
	void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
	
	void setDataHandler(rfcomm_data_handler funPtr);

protected:
	void hci_packet_handler(uint8_t *packet, uint16_t size);

private:
	uint8_t spp_service_buffer[150];
	uint16_t rfcomm_mtu;
	uint16_t rfcomm_cid = 0;
	btstack_packet_callback_registration_t hci_event_callback_registration;

	rfcomm_data_handler dataHandler;
};