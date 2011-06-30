/* Slowlane - Utility to populate and maintain the MythTV channels tables
 * with data extracted from the propriatary Media Highway middleware data.
 *
 * Peter Wood <peter+slowlane@alastria.net>
 *
 * dvb.h - DVB function headers.
 */

#ifndef __DVB_H_
#define __DVB_H_ 1

#define DVB_BUFFER_SIZE 2*4096

int dvb_open(int dvb_adapter, int dvb_demux);
void dvb_close(int dvb_demux_fd);
int dvb_read(int dvb_demux_fd, char *buffer, int buffer_length);
int dvb_set_filter(int dvb_demux_fd, unsigned short pid, unsigned char table, unsigned char mask, int crc_dvb);

#endif
