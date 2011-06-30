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
int si_process_nit(unsigned char *buffer, int buffer_length);
int si_process_sdt(unsigned char *buffer, int buffer_length);
int si_process_bat(unsigned char *buffer, int buffer_length);
int si_process_descriptors(unsigned char *buffer, int buffer_length);
int si_process_descriptor_service(unsigned char *buffer, int buffer_length);
int si_process_descriptor_country_availability(unsigned char *buffer, int buffer_length);
int si_process_descriptor_generic_name(unsigned char *buffer, int buffer_length);
int si_process_descriptor_opentv_channel_information(unsigned char *buffer, int buffer_length);
int si_process_descriptor_satellite_delivery_system(unsigned char *buffer, int buffer_length);

#endif
