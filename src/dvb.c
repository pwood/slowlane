/* Slowlane - Utility to populate and maintain the MythTV channels tables
 * with data extracted from the propriatary Media Highway middleware data.
 *
 * Peter Wood <peter+slowlane@alastria.net>
 *
 * dvb.c - Function to open DVB card, receive packets and close card.
 */

/* Includes */
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/dvb/dmx.h>
#include <linux/dvb/frontend.h>
#include "dvb.h"
#include "slowlane.h"

/* Open DVB interface and set up required PID and SI filters. */
int dvb_open(int dvb_adapter, int dvb_demux) {
	int dvb_demux_fd = 0, count;
	char dvb_device[32];

	/* Create device filename to open. */
	count = snprintf(dvb_device, sizeof(dvb_device), "/dev/dvb/adapter%i/demux%i", dvb_adapter, dvb_demux);

	/* If there's less characters then the minimum or it wanted to use more then the max, error. */
	if (count < 24 || count > 31) {
		slowlane_log(1, "Unable to create device name for demux, character count was %i.", count);
		return -1;
	}

	slowlane_log(1, "Demux device name is %s.", dvb_device);

	/* Open the demux interface. */
	if ((dvb_demux_fd = open(dvb_device, O_RDWR)) < 0) {
		slowlane_log(0, "unable to open DVB demux (%s)", dvb_device);
		return -1;
	}

	slowlane_log(2, "Demux device opened on fd %i.", dvb_demux_fd);

	/* We now have a viable DVB fd. */
	return dvb_demux_fd;
}

/* Close DVB interface. */
void dvb_close(int dvb_demux_fd) {
	if (dvb_demux_fd > 0) {
		slowlane_log(2, "Closing demux file descriptor %i.", dvb_demux_fd);
		close(dvb_demux_fd);
	}
}

/* Read from DVB interface. */
int dvb_read(int dvb_demux_fd, char *buffer, int buffer_length) {
	if (dvb_demux_fd <= 0) {
		slowlane_log(0, "Unable to read from for demux as fd is %i.", dvb_demux_fd);
		return -1;
	}

	return read(dvb_demux_fd, buffer, buffer_length);
}

/* Set up filter for DVB interface. */
int dvb_set_filter(int dvb_demux_fd, unsigned short pid, unsigned char table, unsigned char mask, int crc_dvb) {
	struct dmx_sct_filter_params sctFilterParams;
	int retval;

	/* Set up filter on demux stream to only obtain data relivent to Sky channel list. */
	memset(&sctFilterParams, 0, sizeof(sctFilterParams));

	/* Don't ever stop filtering, and don't wait for a DMX_START ioctl. */
	sctFilterParams.timeout = 0;
	sctFilterParams.flags = DMX_IMMEDIATE_START;

	/* Only enable DVB CRC checks if required, supported in case DVB card can't or stack wont. */
	if (crc_dvb) {
		sctFilterParams.flags |= DMX_CHECK_CRC;
	} else {
		slowlane_log(1, "Not enabling DVB stack/hardware CRC check as crc_dvb is %i!", crc_dvb);
	}

	/* Configure PID and table filter. */
	sctFilterParams.pid = pid;
	sctFilterParams.filter.filter[0] = table;
	sctFilterParams.filter.mask[0] = mask;

	/* Actually set filter on demux. */
	if ((retval = ioctl(dvb_demux_fd, DMX_SET_FILTER, &sctFilterParams)) < 0) {
		close(dvb_demux_fd);
		slowlane_log(0, "Unable to install demux filter, value is %i.", retval);
		return -1;
	}

	slowlane_log(1, "Installed filter on fd %i for PID 0x%x for table 0x%x/0x%x, CRC = %i.", dvb_demux_fd, pid, table, mask, crc_dvb);
	return 0;
}
