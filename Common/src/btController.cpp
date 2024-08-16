#include "btController.h"
#include "pico/cyw43_arch.h"
#include <inttypes.h>

#define RFCOMM_SERVER_CHANNEL 1
#define PRINT_DEBUG false

static BtController* instance = nullptr;

static void static_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
	if (instance == nullptr) return;
	instance->packet_handler(packet_type, channel, packet, size);
}

BtController::BtController(std::string name, int btClass) :
	dataHandler(nullptr), dataSender(nullptr)
{
	instance = this;

	l2cap_init();
	rfcomm_init();
	rfcomm_register_service(static_packet_handler, RFCOMM_SERVER_CHANNEL, 0xffff);

	sdp_init();
	memset(spp_service_buffer, 0, sizeof(spp_service_buffer));
	spp_create_sdp_record(spp_service_buffer, sdp_create_service_record_handle(), RFCOMM_SERVER_CHANNEL, "btController");
	btstack_assert(de_get_len(spp_service_buffer) <= sizeof(spp_service_buffer));
	sdp_register_service(spp_service_buffer);

	// register for HCI events
	hci_event_callback_registration.callback = &static_packet_handler;
	hci_add_event_handler(&hci_event_callback_registration);

	// setup device
	gap_set_class_of_device(btClass);
	gap_ssp_set_io_capability(SSP_IO_CAPABILITY_DISPLAY_YES_NO);
	gap_set_local_name(name.c_str());
	gap_discoverable_control(1);

	hci_power_control(HCI_POWER_ON);
}

void BtController::poll()
{
	cyw43_arch_poll();
}

void BtController::packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
	switch (packet_type)
	{
		case HCI_EVENT_PACKET:
			hci_packet_handler(packet, size);
			break;

		case RFCOMM_DATA_PACKET:
			if (dataHandler != nullptr) dataHandler(packet, size);
			break;

		default:
			printf("Unhandled packet type: %d\n", packet_type);
			break;
	}
}

void BtController::setDataHandler(rfcomm_data_handler funcPtr)
{
	dataHandler = funcPtr;
}

void BtController::setDataSender(rfcomm_data_sender funcPtr)
{
	dataSender = funcPtr;
}

void BtController::hci_packet_handler(uint8_t *packet, uint16_t size)
{
	bd_addr_t event_addr;
	uint8_t rfcomm_channel_nr;

	switch (hci_event_packet_get_type(packet))
	{
		case HCI_EVENT_CONNECTION_REQUEST:
			printf("Connection request: ");
			for (int i = 0; i < 4; ++i)
			{
				printf("%02x ", packet[8 + i]);
			}
			printf("\n");
			break;

		case HCI_EVENT_COMMAND_STATUS:
			printf("Command status: ");
			for (int i = 0; i < 4; ++i)
			{
				printf("%02x ", packet[2 + i]);
			}
			printf("\n");
			break;

		case BTSTACK_EVENT_NR_CONNECTIONS_CHANGED:
			printf("Number of connections changed: %d\n", packet[2]);
			break;

		case HCI_EVENT_TRANSPORT_PACKET_SENT:
			if (!PRINT_DEBUG) break;
			printf("Transport packet sent: ");
			for (int i = 0; i < size; ++i)
			{
				printf("%02x ", packet[i]);
			}
			printf("\n");
			break;

		// case HCI_EVENT_NUMBER_OF_COMPLETED_PACKETS:
		// 	printf("HCI_EVENT_NUMBER_OF_COMPLETED_PACKETS (%d): ", size);
		// 	for (int i = 0; i < size; ++i)
		// 	{
		// 		printf("%02x ", packet[i]);
		// 	}
		// 	printf("\n");
		// 	break;

		case HCI_EVENT_MAX_SLOTS_CHANGED:
			printf("Max slots changed to: %d\n", packet[4]);
			break;

		case HCI_EVENT_CONNECTION_COMPLETE:
			printf("Connection complete to: ");
			printf("%02x:", packet[10]);
			printf("%02x:", packet[9]);
			printf("%02x:", packet[8]);
			printf("%02x:", packet[7]);
			printf("%02x:", packet[6]);
			printf("%02x. ", packet[5]);
			printf("Connection handle: %02x %02x. ", packet[3], packet[4]);
			printf("LinkType: %02x. ", packet[11]);
			printf("Encryption: %02x.\n", packet[12]);
			break;

		case HCI_EVENT_IO_CAPABILITY_RESPONSE:
			printf("IO capability response.\n");
			break;

		case HCI_EVENT_IO_CAPABILITY_REQUEST:
			printf("IO capability request.\n");
			break;

		case HCI_EVENT_SIMPLE_PAIRING_COMPLETE:
			printf("Simple pairing complete.\n");
			break;

		case HCI_EVENT_COMMAND_COMPLETE:
			if (!PRINT_DEBUG) break;
			printf("Command complete.\n");
			break;

		case HCI_EVENT_LINK_KEY_NOTIFICATION:
			printf("Link key notification: ");
			for (int i = 0; i < 16; ++i)
			{
				printf("%02x ", packet[7 + i]);
			}
			printf(". Key type: %02x\n", packet[24]);
			break;

		case HCI_EVENT_ENCRYPTION_CHANGE:
			printf("Encryption change: %02x\n", packet[5]);
			break;

		case HCI_EVENT_NUMBER_OF_COMPLETED_PACKETS:
			if (!PRINT_DEBUG) break;
			printf("Number of completed packets: %d\n", little_endian_read_16(packet, 5));
			break;

		case HCI_EVENT_DISCONNECTION_COMPLETE:
			printf("Disconnection complete: %02x\n", packet[5]);
			break;

		case HCI_EVENT_PIN_CODE_REQUEST:
			// inform about pin code request
			printf("Pin code request - using '0000'\n");
			hci_event_pin_code_request_get_bd_addr(packet, event_addr);
			gap_pin_code_response(event_addr, "0000");
			break;

		case HCI_EVENT_USER_CONFIRMATION_REQUEST:
			// inform about user confirmation request
			printf("SSP User Confirmation Request with numeric value '%06" PRIu32 "'\n", little_endian_read_32(packet, 8));
			printf("SSP User Confirmation Auto accept\n");
			break;

		case RFCOMM_EVENT_INCOMING_CONNECTION:
			rfcomm_event_incoming_connection_get_bd_addr(packet, event_addr);
			rfcomm_channel_nr = rfcomm_event_incoming_connection_get_server_channel(packet);
			rfcomm_cid = rfcomm_event_incoming_connection_get_rfcomm_cid(packet);
			printf("RFCOMM channel 0x%02x requested for %s\n", rfcomm_channel_nr, bd_addr_to_str(event_addr));
			rfcomm_accept_connection(rfcomm_cid);
			break;

		case RFCOMM_EVENT_CHANNEL_OPENED:
			if (rfcomm_event_channel_opened_get_status(packet))
			{
				printf("RFCOMM channel open failed, status 0x%02x\n", rfcomm_event_channel_opened_get_status(packet));
			}
			else
			{
				rfcomm_cid = rfcomm_event_channel_opened_get_rfcomm_cid(packet);
				rfcomm_mtu = rfcomm_event_channel_opened_get_max_frame_size(packet);
				printf("RFCOMM channel open succeeded. New RFCOMM Channel ID 0x%02x, max frame size %u\n", rfcomm_cid, rfcomm_mtu);

				// disable page/inquiry scan to get max performance
				gap_discoverable_control(0);
				gap_connectable_control(0);

				rfcomm_request_can_send_now_event(rfcomm_cid);
			}
			break;

		case RFCOMM_EVENT_CAN_SEND_NOW:
			sendData();
			break;

		case RFCOMM_EVENT_CHANNEL_CLOSED:
			printf("RFCOMM channel closed\n");
			rfcomm_cid = 0;

			// re-enable page/inquiry scan again
			gap_discoverable_control(1);
			gap_connectable_control(1);
			break;

		default:
			if (!PRINT_DEBUG) break;
			printf("UNHANDLED HCI EVENT: ");
			for (int i = 0; i < size; ++i)
			{
				printf("%02x ", packet[i]);
			}
			printf("\n");
			break;
	}
}

void BtController::sendData()
{
	if (dataSender != nullptr)
	{
		dataSender(sendBuffer, sendBufferLength);

		if (sendBufferLength > 0)
		{
			rfcomm_send(rfcomm_cid, sendBuffer, sendBufferLength);
			sendBufferLength = 0;
			rfcomm_request_can_send_now_event(rfcomm_cid);
			printf("SENT PACKET\n");
		}
	}
}
