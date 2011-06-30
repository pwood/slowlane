/* Slowlane - Utility to populate and maintain the MythTV channels tables
 * with data extracted from the propriatary Media Highway middleware data.
 *
 * Peter Wood <peter+slowlane@alastria.net>
 *
 * si.c - Service Information handling and processing.
 */

/* Includes */
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "slowlane.h"
#include "si.h"
#include "crc32.h"

/* Process a SI packet received. Returns -1 serious error, lenght of processed bytes. */
int si_process(unsigned char *buffer, int buffer_length, int internal_crc) {
	unsigned char table_type;
	unsigned short table_length;
	u_int32_t calculated_crc;	

	/* We can not process a packet smaller then 3 bytes, table type and length of data, chances are if it's just 3 then we'll fail anyway. */
	if (buffer_length < 3) {
		/* Not a critical failure. */
		slowlane_log(2, "Passed less then 3 bytes into si_process, count was %i.", buffer_length);
		return 0;
	}

	/* First, extract table type and length of data. */
	table_type = buffer[0];
	table_length = ((buffer[1] & 0x0f) << 8) | buffer[2];

	/* Check for buffer underrun? We haven't checked CRC yet, but if the bytes are currupt then we'll either
	 * be well into the packet, in which case CRC will fail, or be well beyond packet at which case we'll underrun. */
	if (table_length + 3 > buffer_length) {
		/* Not enough data. */
		slowlane_log(2, "Buffer underrun based on table_length, required length is %i and buffer only has %i.", table_length + 3, buffer_length);
		return 0;
	}

	/* Do we need to check the CRC? */
	calculated_crc = crc32((char *) buffer, table_length + 3, 0xffffffff);

	if (!internal_crc) {
		if (calculated_crc) {
			slowlane_log(2, "Internal CRC confirmation disabled, packet did fail CRC though, remainder was 0x%x!", calculated_crc);
			calculated_crc = 0;
		} else {
			slowlane_log(3, "Internal CRC confirmation disabled, packet did pass CRC though as %i.", calculated_crc);
		}
	} 
	
	/* Check the CRC remainder, should be 0 if it's a valid packet. */
	if (calculated_crc) {
		/* Again not a critical fault. */
		slowlane_log(2, "Packet failed CRC check. CRC remaineder was 0x%x.", calculated_crc);
		return -1;
	}

	/* Let user know if they've asked for this level. */
	slowlane_log(2, "Valid %x packet recieved, length is %i which leaves %i bytes in buffer.", table_type, table_length + 3, buffer_length - table_length - 3);

	/* Switch based on table_type. */
	switch (table_type) {
		case 0x40: /* Network Information Table - This Mux */
		case 0x41: /* Network Information Table - Other Muxes */
			slowlane_log(3, "Packet identified as NIT, passing %i bytes to si_process_nit.", table_length);
			si_process_nit(buffer + 3, table_length);
			break;

		case 0x42: /* Service Description Table - This Mux */
		case 0x46: /* Service Description Table - Other Muxes */
			slowlane_log(3, "Packet identified as SDT, passing %i bytes to si_process_sdt.", table_length);
			si_process_sdt(buffer + 3, table_length);
			break;

		case 0x4a: /* Bouquet Association Table */
			slowlane_log(3, "Packet identified as BAT, passing %i bytes to si_process_bat.", table_length);
			si_process_bat(buffer + 3, table_length);
			break;

		default:
			slowlane_log(1, "Valid, but unknown table 0x%x of size %i was received.", table_type, table_length + 3);
			break;
	}

	return table_length + 3;
}

/* Process NIT packet. */
int si_process_nit(unsigned char *buffer, int buffer_length) {
	unsigned short network_id, network_descriptors_length, transport_stream_loop_length, transport_stream_id, original_network_id, transport_descriptors_length;
	unsigned char version, section_number, last_section_number;
	int position;

	/* Sanity check. */
	if (buffer_length <= 7) {
		slowlane_log(1, "NIT not long enough for basic no looped data, size was %i.", buffer_length);
		return -1;
	}

	network_id = (buffer[0] << 8) | buffer[1];
        version = (buffer[2] & 0x3e) >> 1;
        section_number = buffer[3];
        last_section_number = buffer[4];
	network_descriptors_length = ((buffer[5] & 0x0f) << 8) | buffer[6];

        /* Display basic Bouquet data. */
        slowlane_log(3, "Network: ID: %i Version: %i Section: %i Last: %i", network_id, version, section_number, last_section_number);

	/* Set processing position at the end of the header. */
	position = 7;

	/* Process descriptors */
	si_process_descriptors(buffer+position, network_descriptors_length);
	position += network_descriptors_length;

        /* Get size of the next loop. */
        transport_stream_loop_length = ((buffer[position] & 0x0f) << 8) | buffer[position+1];
        position += 2;

        /* Loop through the services until we hit the end of the buffer. */
        while (position < (buffer_length - 4)) {
                /* Sanity check. */
                if ((buffer_length - position) <= 6) {
                        slowlane_log(1, "NIT service loop not long enough for basic data, position was %i in %i.", position, buffer_length);
                        return -1;
                }

		/* Get TS Details. */
		transport_stream_id = (buffer[position] << 8) | buffer[position + 1];
		original_network_id = (buffer[position + 2] << 8) | buffer[position + 3];
		transport_descriptors_length = ((buffer[position + 4] & 0x0f) << 8) | buffer[position + 5];
		position += 6;

		/* Display TS details. */
		slowlane_log(3, "Network TS ID: %i Original Network ID: %i", transport_stream_id, original_network_id);

		/* Fetch TS details. */
		si_process_descriptors(buffer+position, transport_descriptors_length);
		position += transport_descriptors_length;
	}	

	return 0;
}

/* Process SDT packet. */
int si_process_sdt(unsigned char *buffer, int buffer_length) {
	unsigned short transport_stream_id, original_network_id, service_id, descriptors_loop_length;
	unsigned char version, section_number, last_section_number, running_mode, free_ca_mode;
	int position;

	/* Sanity check. */
	if (buffer_length <= 8) {
		slowlane_log(1, "SDT not long enough for basic no looped data, size was %i.", buffer_length);
		return -1;
	}

	/* Extract all basic data. */
	transport_stream_id = (buffer[0] << 8) | buffer[1];
	version = (buffer[2] & 0x3e) >> 1;
	section_number = buffer[3];
	last_section_number = buffer[4];
	original_network_id = (buffer[5] << 8) | buffer[6];

	/* Display basic SDT data. */
	slowlane_log(3, "SDT: TS_ID: %i Version: %i Section: %i Last: %i Original Net: %i", transport_stream_id, version, section_number, last_section_number, original_network_id);

	/* Set processing position at the end of the header. */
	position = 8;

	/* Loop through the services until we hit the end of the buffer. */
	while (position < (buffer_length - 4)) {
		/* Sanity check. */
		if ((buffer_length - position) <= 5) {
			slowlane_log(1, "SDT service loop not long enough for basic data, position was %i in %i.", position, buffer_length);
			return -1;
		}

		service_id = (buffer[position] << 8) | buffer[position + 1];
		running_mode = (buffer[position+3] & 0xe0) >> 5;
		free_ca_mode = (buffer[position+3] & 0x10) >> 4;
		descriptors_loop_length = ((buffer[position + 3] & 0x0f) << 8) | buffer[position + 4];

		/* Display service found. */
		slowlane_log(3, "SDT: Service: %i Running: %i Free CA: %i Descriptors: %i", service_id, running_mode, free_ca_mode, descriptors_loop_length);

		/* Move position on beyond service header. */
		position += 5;

		/* Process descriptors */
		si_process_descriptors(buffer+position, descriptors_loop_length);
		position += descriptors_loop_length;
	}

	return 0;
}

/* Process BAT packet. */
int si_process_bat(unsigned char *buffer, int buffer_length) {
	unsigned short bouquet_id, bouquet_descriptors_length, transport_stream_loop_length, transport_stream_id, original_network_id, transport_descriptors_length;
	unsigned char version, section_number, last_section_number;
	int position;

	/* Sanity check. */
	if (buffer_length <= 7) {
		slowlane_log(1, "BAT not long enough for basic no looped data, size was %i.", buffer_length);
		return -1;
	}

	bouquet_id = (buffer[0] << 8) | buffer[1];
        version = (buffer[2] & 0x3e) >> 1;
        section_number = buffer[3];
        last_section_number = buffer[4];
	bouquet_descriptors_length = ((buffer[5] & 0x0f) << 8) | buffer[6];

        /* Display basic Bouquet data. */
        slowlane_log(3, "Bouquet: ID: %i Version: %i Section: %i Last: %i", bouquet_id, version, section_number, last_section_number);

        /* Set processing position at the end of the header. */
        position = 7;

	/* Process descriptors */
	si_process_descriptors(buffer+position, bouquet_descriptors_length);
	position += bouquet_descriptors_length;

	/* Get size of the next loop. */
	transport_stream_loop_length = ((buffer[position] & 0x0f) << 8) | buffer[position+1];
	position += 2;

	/* Loop through transport streams */
	while (position < (buffer_length - 4)) {
		/* Sanity Check */
		if ((buffer_length - position) <= 6) {
			slowlane_log(1, "BAT service loop not long enough for basic data, position was %i in %i.", position, buffer_length);
			return -1;
		}

		/* Extract data. */
		transport_stream_id = (buffer[position] << 8) | buffer[position + 1];
		original_network_id = (buffer[position + 2] << 8) | buffer[position + 3];
		transport_descriptors_length = ((buffer[position + 4] & 0x0f) << 8) | buffer[position + 5];
		position += 6;

        	/* Display stream Bouquet data. */
	        slowlane_log(3, "Bouquet Stream: Transport ID: %i Original Network: %i", transport_stream_id, original_network_id);

		/* Extract descriptors. */
                si_process_descriptors(buffer+position, transport_descriptors_length);
		position += transport_descriptors_length;
	}

	return 0;
}

int si_process_descriptors(unsigned char *buffer, int buffer_length) {
	int position = 0;
	unsigned char descriptor_id, descriptor_length;

	/* Loop through descriptors. */
	while (position < buffer_length) {
		if ((buffer_length - position) <= 2) {
			slowlane_log(1, "Descriptor loop not long enough for basic data, position was %i in %i.", position, buffer_length);
			return -1;
		}

		/* Obtain id and length. */
		descriptor_id = buffer[position];
		descriptor_length = buffer[position + 1];
		position += 2;

		slowlane_log(3, "Descriptor: ID: %x Length: %i", descriptor_id, descriptor_length);

		/* Sanity check. */		
		if ((buffer_length - position) < descriptor_length) {
                        slowlane_log(1, "Not enough data in buffer to pass to descriptor function, position was %i in %i, descriptor length is %i.", position, buffer_length, descriptor_length);
                        return -1;
		}

		switch(descriptor_id) {
			case 0x43: /* Satellite Delivery System */
				si_process_descriptor_satellite_delivery_system(buffer+position, descriptor_length);
				break;

			case 0x40: /* Network Name */
			case 0x47: /* Bouquet Name */
				si_process_descriptor_generic_name(buffer+position, descriptor_length);
				break;

			case 0x48: /* Service Descriptor */
				si_process_descriptor_service(buffer+position, descriptor_length);
				break;

			case 0x49: /* Country Available */
				si_process_descriptor_country_availability(buffer+position, descriptor_length);
				break;

			case 0xb1: /* MediaHighway Propriatary - Channel Information. */
				si_process_descriptor_opentv_channel_information(buffer+position, descriptor_length);
				break;

			case 0x41: /* Service link. */
			case 0x4a: /* Linkage Descriptor */
			case 0x4b: /* NVOD Reference. */
			case 0x4c: /* Time Shifted Service */
			case 0x5f: /* Private data specifier. */
			case 0xb2: /* MediaHighway Propriatary - On screen message (Possibly Huffmann) */
				break;
			default:
				slowlane_log(2, "Unhandled descriptor id %x.", descriptor_id);
				break;
		}

		position += descriptor_length;
	}

	return 0;
}

int si_process_descriptor_service(unsigned char *buffer, int buffer_length) {
	int position = 0;
	unsigned char service_type, service_provider_name_length, service_name_length;
	char service_provider_name[256], service_name[256];

	/* Sanity check. */
	if (buffer_length <= 2) {
		slowlane_log(1, "Service descriptor not long enough for basic no looped data, size was %i.", buffer_length);
		return -1;
	}

	service_type = buffer[0];
	service_provider_name_length = buffer[1];
	position += 2;

	/* Sanity check again. */
	if ((buffer_length - position) <= service_provider_name_length) {
		slowlane_log(1, "Not enough data for service provider name, position was %i in %i, size was %i.", position, buffer_length, service_provider_name_length);
		return -1;
	}

	memcpy(&service_provider_name, buffer+position, service_provider_name_length);
	service_provider_name[service_provider_name_length] = '\0';
	position += service_provider_name_length;

	service_name_length = buffer[position];
	position += 1;

	/* Sanity check again. */
	if ((buffer_length - position) < service_name_length) {
		slowlane_log(1, "Not enough data for service name, position was %i in %i, size was %i.", position, buffer_length, service_name_length);
		return -1;
	}

        memcpy(&service_name, buffer+position, service_name_length);
        service_name[service_name_length] = '\0';
        position += service_name_length;

	slowlane_log(3, "Descriptor: Name: %s Provider: %s Type 0x%x", service_name, service_provider_name, service_type);

	return 0;
}

int si_process_descriptor_country_availability(unsigned char *buffer, int buffer_length) {
        int position = 0;
        unsigned char country_availability_flag;
        char country[4];

        /* Sanity check. */
        if (buffer_length <= 1) {
                slowlane_log(1, "Country availability descriptor not long enough for basic no looped data, size was %i.", buffer_length);
                return -1;
        }

	country_availability_flag = (buffer[0] & 0xe0) >> 7;
	position += 1;

        slowlane_log(3, "Country availability: Is Available: %i", country_availability_flag);	

	while (position < buffer_length) {
		memcpy(&country, buffer+position, 3);
		country[3] = '\0';
		position += 3;
	        slowlane_log(3, "Country availability: %s", country);
	}

        return 0;
}

int si_process_descriptor_generic_name(unsigned char *buffer, int buffer_length) {
	char name[256];

	memcpy(&name, buffer, buffer_length);
	name[buffer_length] = '\0';

	slowlane_log(3, "Name: %s", name);

	return 0;
}

int si_process_descriptor_opentv_channel_information(unsigned char *buffer, int buffer_length) {
	unsigned short region, service_id, channel_id, user_id, flags;
	unsigned char type;
	int position = 0;

	region = (buffer[0] << 8) | buffer[1];
	slowlane_log(3, "OpenTV Region: %i", region);

	position += 2;

	while (position < buffer_length) {
		if ((buffer_length - position) < 9) {
			slowlane_log(1, "Unable to read OpenTV channel information, lack of buffer position is %i of %i.", position, buffer_length);
			return -1;
		}

		service_id = (buffer[position] << 8) | buffer[position + 1];
		type = buffer[position + 2];
		channel_id = (buffer[position + 3] << 8) | buffer[position + 4];
		user_id = (buffer[position + 5] << 8) | buffer[position + 6];
		flags = (buffer[position + 7] << 8) | buffer[position + 8];

		slowlane_log(3, "OpenTV Channel: Service: %i Type: %i Channel: %i User: %i Flags: %x", service_id, type, channel_id, user_id, flags);

		position += 9;
	}

	return 0;
}

int si_process_descriptor_satellite_delivery_system(unsigned char *buffer, int buffer_length) {
	unsigned int frequency = 0, symbol_rate = 0, tmp;
	unsigned short orbital_position = 0;
	unsigned char west_easy_flag, polarization, roll_off, modulation_system, modulation_type, fec, pos, bcd;

	/* Calculate frequency from BSD. */

	tmp = (buffer[0] << 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3];

	for (pos = 0; pos < 8; pos++) {
		frequency *= 10;
		bcd = (tmp & 0xf0000000) >> 28;
		tmp = tmp << 4;
		frequency += bcd;
	}

        tmp = (buffer[4] << 8) + buffer[5];

        for (pos = 0; pos < 4; pos++) {
                orbital_position *= 10;
                bcd = (tmp & 0xf000) >> 12;
                tmp = tmp << 4;
                orbital_position += bcd;
        }

	west_easy_flag = (buffer[6] & 0x80) >> 7;
	polarization = (buffer[6] & 0x60) >> 5;
	roll_off = (buffer[6] & 0x18) >> 3;
	modulation_system = (buffer[6] & 0x04) >> 2;
	modulation_type = (buffer[6] & 0x03);

	tmp = (buffer[7] << 24) + (buffer[8] << 16) + (buffer[9] << 8) + buffer[10];

	for (pos = 0; pos < 7; pos++) {
		symbol_rate *= 10;
		bcd = (tmp & 0xf0000000) >> 28;
		tmp = tmp << 4;
		symbol_rate += bcd;
	}

	fec = (buffer[10] & 0x0f);

	slowlane_log(3, "SDS: Freq: %i Symbol: %i Orbit: %i West/East: %i Polorization: %i Roll Off: %i ModSys: %i ModType: %i FEC: %i", frequency, symbol_rate, orbital_position, west_easy_flag, polarization, roll_off, modulation_system, modulation_type, fec);

	return 0;
}
