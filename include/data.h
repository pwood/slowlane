/* Slowlane - Utility to populate and maintain the MythTV channels tables
 * with data extracted from the propriatary Media Highway middleware data.
 *
 * Peter Wood <peter+slowlane@alastria.net>
 *
 * data.h - Stored Data function headers.
 */

#ifndef __DATA_H_
#define __DATA_H_ 1

/* Structure for section tracking. */
typedef struct tSectionTracking {
	/* Which version of the table are we working on. */
	unsigned char	version;

	/* Which is the last section of this table. */
	unsigned char	last_section;

	/* Which sections have been received. */
	unsigned char	received_section[0xff];
} SectionTracking;

/* Entries required for storing BAT details. */

typedef struct tOpenTVChannel {
	/* Linked List */
	struct tOpenTVChannel	*next;

	/* Details About OpenTVChannel */
	unsigned short	transport_id;
	unsigned short	original_network_id;
	unsigned short	service_id;

	unsigned char	region;
	unsigned char	type;
	unsigned short	channel_number;
	unsigned short	user_number;
	unsigned short	flags;
} OpenTVChannel;

typedef struct tBouquet {
	/* Linked List */
	struct tBouquet		*next;

	/* Section Tracking */
	SectionTracking	sections;

	/* OpenTVChannels */
	OpenTVChannel	*channels;

	/* Details About Bouquet */
	unsigned short	bouquet_id;
	char		*name;
} Bouquet;

/* Entries required for storing SMT and NIT details. */

typedef struct tService {
	/* Linked List */
	struct tService		*next;

	/* Details About Service */
	unsigned short	service_id;
	unsigned char	running;
	unsigned char	free_ca;
	unsigned char	type;

	char		*name;
	char		*alt_name;
	char		*provider;

} Service;

typedef struct tTransport {
	/* Linked List */
	struct tTransport	*next;

	/* Section Tracking */
	SectionTracking	sections;

	/* Services */
	Service		*services;

	/* Details About Transport */
	unsigned short	original_network_id;
	unsigned short	transport_id;

	unsigned char	modulation_system;

	unsigned int	frequency;
	unsigned int	symbol_rate;
	unsigned char	polarization;
	unsigned char	moduleation_type;
	unsigned char	fec;
	unsigned char	roll_off;

	unsigned short	orbital_position;
	unsigned char	west_east_flag;
} Transport;

typedef struct tNetwork {
	/* Linked List */
	struct tNetwork		*next;

	/* Section Tracking */
	SectionTracking	sections;

	/* Transports */
	Transport	*transports;

	/* Details About Network */
	unsigned short	network_id;
	char		*name;
} Network;

extern Network *network_list;
extern Bouquet *bouquet_list;

#endif
