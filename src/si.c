/* Slowlane - Utility to populate and maintain the MythTV channels tables
 * with data extracted from the propriatary Media Highway middleware data.
 *
 * Peter Wood <peter+slowlane@alastria.net>
 *
 * si.c - Service Information handling and processing.
 */

/* Includes */
#include <stdio.h>
#include <sys/types.h>
#include "slowlane.h"
#include "si.h"
#include "crc32.h"

/* Process a SI packet received. Returns -1 serious error, 0 nothing processed, 1+ packets processed. */
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
		return 0;
	}

	/* Let user know if they've asked for this level. */
	slowlane_log(2, "Valid %x packet recieved, length is %i which leaves %i bytes in buffer.", table_type, table_length + 3, buffer_length - table_length - 3);

	/* Switch based on table_type. */
	switch (table_type) {
		case 0x40: /* Network Information Table - This Mux */
		case 0x41: /* Network Information Table - Other Muxes */
			break;

		case 0x42: /* Service Description Table - This Mux */
		case 0x46: /* Service Description Table - Other Muxes */
			break;

		case 0x4a: /* Bouquet Association Table */
			break;

		default:
			slowlane_log(1, "Valid, but unknown table 0x%x of size %i was received.", table_type, table_length + 3);
			break;
	}

	return 0;
}
