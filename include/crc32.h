/* Slowlane - Utility to populate and maintain the MythTV channels tables
 * with data extracted from the propriatary Media Highway middleware data.
 *
 * Peter Wood <peter+slowlane@alastria.net>
 *
 * crc32.h - CRC32 computation headers.
 */

#ifndef __CRC32_H_
#define __CRC32_H_ 1

u_int32_t crc32 (const char *d, int len, u_int32_t crc);

#endif
