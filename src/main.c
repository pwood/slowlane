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
#include <string.h>
#include <time.h>
#include "slowlane.h"
#include "dvb.h"
#include "si.h"
#include "data.h"

/* Local definitions. */
void usage (void);

/* Global variables */
int verbose = 0;

/* Program start. */
int main (int argc, char *argv[]) {
	int crc_dvb = 1, crc_internal = 1, dvb_adapter = 0, dvb_demux = 0, dvb_loop = 1, loop_time = 10;
	int ch, dvb_demux_fd, dvb_bytes, retval, dvb_data_length = 0, processed_bytes = 0, done = 0;
	time_t dvb_loop_start;
	char dvb_buffer[DVB_BUFFER_SIZE];
	char *dvb_data = NULL, *dvb_temp = NULL;
	Network *network;
	Transport *transport;

	/* Process command line options. */
	while ((ch = getopt(argc, argv, "c:C:a:d:l:hv")) != -1) {
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
			case 'l':
                                loop_time = atoi(optarg);
                                slowlane_log(3, "loop_time set to %i.", loop_time);
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

	/* Set filter for NIT. */
	if ((retval = dvb_set_filter(dvb_demux_fd, 0x0010, 0x40, 0xf0, crc_dvb)) < 0) {
		slowlane_log(0, "NIT dvb_set_filter failed and returned %i.", retval);
		return EXIT_FAILURE;
	}

	/* Record when we start this loop.*/
	dvb_loop_start = time(NULL);

	/* Loop obtaining packets until we have enough. */
	while (dvb_loop) {
		/* Read DVB card. */
		if ((dvb_bytes = dvb_read(dvb_demux_fd, dvb_buffer, DVB_BUFFER_SIZE)) <= 0) {
			slowlane_log(0, "dvb_read failed and returned %i.", dvb_bytes);
			dvb_close(dvb_demux_fd);
			return EXIT_FAILURE;
		}

		/* Copy data into dvb_data. */
		if (dvb_data == NULL) {
			slowlane_log(3, "dvb_read read in %i, mallocing memory.", dvb_bytes);
			dvb_data_length = dvb_bytes;
			dvb_data = (char *) malloc (dvb_data_length);
			memcpy(dvb_data, dvb_buffer, dvb_data_length);		
		} else {
			slowlane_log(3, "dvb_read read in %i, already %i here, reallocing memory.", dvb_bytes, dvb_data_length);
			dvb_data = (char *) realloc (dvb_data, dvb_data_length + dvb_bytes);
			memcpy(dvb_data + dvb_data_length, dvb_buffer, dvb_bytes);
                        dvb_data_length += dvb_bytes;
		}

		/* Loop while processing function is reporting success, this is needed for some dvb cards or sasc-ng virtual cards which
		 * don't obey the one packet per read rule. */
		do {
			/* Process SI received. */
			if ((processed_bytes = si_process((unsigned char *)dvb_data, dvb_data_length, crc_internal)) < 0) {
				slowlane_log(0, "si_process failed and returned %i.", processed_bytes);

				/* Dump data and flush buffer. */
				free(dvb_data);
				dvb_data = NULL;
				dvb_data_length = 0;
			} else {
				if (processed_bytes > 0) {
					if (processed_bytes == dvb_data_length) {
						slowlane_log(3, "si_process processed whole data of size %i.", processed_bytes);
						dvb_data_length = 0;
						free(dvb_data);
						dvb_data = NULL;
					} else {
						slowlane_log(3, "si_process processed less then whole data of size %i out of %i.", processed_bytes, dvb_data_length);
						dvb_temp = (char *) malloc (dvb_data_length - processed_bytes);
						memcpy(dvb_temp, dvb_data + processed_bytes, dvb_data_length - processed_bytes);
						free(dvb_data);
						dvb_data = dvb_temp;
						dvb_data_length -= processed_bytes;
					}
				}
			}
		} while (processed_bytes < dvb_data_length && processed_bytes > 0);

		if (dvb_loop == 1) {
			/* Verify if timeout has expired. */
			if (time(NULL) > dvb_loop_start + loop_time) {
				/* Verify if all present networks are complete. */
				done = 1;

				for (network = network_list; network != NULL; network = network->next) {
					if (!section_tracking_check(&network->sections)) {
						done = 0;
					}
				}

				if (done) {
					/* Inform the user. */
					slowlane_log(2, "NIT tables complete (%i), moving to BAT and DST tables.", dvb_loop);

					/* Set filter for BAT and SDT. */
					if ((retval = dvb_set_filter(dvb_demux_fd, 0x0011, 0x40, 0xf0, crc_dvb)) < 0) {
						slowlane_log(0, "BAT/DST dvb_set_filter failed and returned %i.", retval);
						return EXIT_FAILURE;
					}

					/* Essentially starting the loop again. */
				        dvb_loop_start = time(NULL);
					dvb_loop = 2;
				}
			}
		} else {
			/* Verify if timeout has expired. */
			if (time(NULL) > dvb_loop_start + loop_time) {
				/* Verify if all present networks are complete. */
				done = 1;

				for (network = network_list; network != NULL; network = network->next) {
					for (transport = network->transports; transport != NULL; transport = transport->next) {
						if (transport->sections.populated == 0 || !section_tracking_check(&transport->sections)) {
							done = 0;
						}
					}
				}

				if (done) {
					/* Inform the user. */
					slowlane_log(2, "BAT and DST tables complete (%i).", dvb_loop);

					dvb_loop = 0;
					break;
				}
			}
		}
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
	printf("\t-l <seconds>\tMinimum Seconds on DVB Loop (<default = 10>)\n");
	printf("\t-v\t\tIncrement Verbose Level (<default = 0>)\n");
}
