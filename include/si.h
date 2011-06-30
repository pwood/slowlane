/* Slowlane - Utility to populate and maintain the MythTV channels tables
 * with data extracted from the propriatary Media Highway middleware data.
 *
 * Peter Wood <peter+slowlane@alastria.net>
 *
 * si.h - Service Information function headers.
 */

#ifndef __SI_H_
#define __SI_H_ 1

int si_process(unsigned char *buffer, int buffer_length, int internal_crc);

#endif
