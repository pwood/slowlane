/* Slowlane - Utility to populate and maintain the MythTV channels tables
 * with data extracted from the propriatary Media Highway middleware data.
 *
 * Peter Wood <peter+slowlane@alastria.net>
 *
 * main.c - Main program execution and control.
 */

/* Includes */
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include "slowlane.h"
#include "dvb.h"
#include "si.h"

/* Local definitions. */
void usage (void);

/* Global variables */
int verbose = 0;

/* Program start. */
int main (int argc, char *argv[]) {
	int crc_dvb = 1, crc_internal = 1, dvb_adapter = 0, dvb_demux = 0, dvb_loop = 1;
	int ch, dvb_demux_fd, dvb_bytes, retval;
	char dvb_buffer[DVB_BUFFER_SIZE];

	/* Process command line options. */
	while ((ch = getopt(argc, argv, "c:C:a:d:hv")) != -1) {
		switch (ch) {
			case 'c':
				crc_dvb = atoi(optarg);
				slowlane_log(3, "crc_dvb set to %i.", crc_dvb);
				break;
			case 'C':
				crc_internal = atoi(optarg);
				slowlane_log(3, "crc_internal set to %i.", crc_internal);
				break;
			case 'a':
				dvb_adapter = atoi(optarg);
				slowlane_log(3, "dvb_adapter set to %i.", dvb_adapter);
				break;
			case 'd':
				dvb_demux = atoi(optarg);
				slowlane_log(3, "dvb_demux set to %i.", dvb_demux);
				break;
			case 'v':
				verbose++;
				slowlane_log(0, "verbose set to %i.", verbose);
				break;
			case 'h':
			default:
				usage();
				return EXIT_FAILURE;
				break;
		}
	}

	/* Fetch fd for DVB card. */
	if ((dvb_demux_fd = dvb_open(dvb_adapter, dvb_demux)) < 1) {
		slowlane_log(0, "dvb_open failed and returned %i.", dvb_demux_fd);
		return EXIT_FAILURE;
	}

	/* XXX - Two loops will be needed here, once to obtain the BAT and SDT, then once again for the NIT. */

	/* Set filter for BAT and SDT. */
	if ((retval = dvb_set_filter(dvb_demux_fd, 0x0011, 0x40, 0xf0, crc_dvb)) < 0) {
		slowlane_log(0, "BAT/DST dvb_set_filter failed and returned %i.", retval);
		return EXIT_FAILURE;
	}

	/* Loop obtaining packets until we have enough. */
	while (dvb_loop) {
		/* Read DVB card. */
		if ((dvb_bytes = dvb_read(dvb_demux_fd, dvb_buffer, DVB_BUFFER_SIZE)) <= 0) {
			slowlane_log(0, "dvb_read failed and returned %i.", dvb_bytes);
			dvb_close(dvb_demux_fd);
			return EXIT_FAILURE;
		}

		/* Process SI received. */
		if (si_process((unsigned char *)dvb_buffer, dvb_bytes, crc_internal) < 0) {
			slowlane_log(0, "si_process failed and returned %i.", dvb_demux_fd);
			dvb_close(dvb_demux_fd);
			return EXIT_FAILURE;
		}

		/* XXX - Rework to handle multiple packets in same or accross reads. sasc-ng breaks the one packet per read rule. */
	}

	/* Set filter for NIT. */
	if ((retval = dvb_set_filter(dvb_demux_fd, 0x0010, 0x40, 0xf0, crc_dvb)) < 0) {
		slowlane_log(0, "NIT dvb_set_filter failed and returned %i.", retval);
		return EXIT_FAILURE;
	}

	/* Close fd now we're done. */
	dvb_close(dvb_demux_fd);

	/* XXX - Process BAT/SMT data to form channnel list. */
	/* XXX - Update MySQL database with it. */
	/* XXX - Somehow handle xmltv overrides. */

	return EXIT_SUCCESS;
}

/* Display usage information. */
void usage (void) {
	printf("%s (%s) by %s\n", SLOWLANE_NAME, SLOWLANE_VERSION, SLOWLANE_AUTHOR);
	printf("\t-c <flag>\tCRC Check (DVB Stack) (0 = Off, 1 = On <default>)\n");
	printf("\t-C <flag>\tCRC Check (Internal) (0 = Off, 1 = On <default>)\n");
	printf("\t-a <number>\tDVB Adapter Number (<default = 0>)\n");
	printf("\t-d <number>\tDVB Demux Number (<default = 0>)\n");
	printf("\t-v\t\tIncrement Verbose Level (<default = 0>)\n");
}
