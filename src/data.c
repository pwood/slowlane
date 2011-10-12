/* Slowlane - Utility to populate and maintain the MythTV channels tables
 * with data extracted from the propriatary Media Highway middleware data.
 *
 * Peter Wood <peter+slowlane@alastria.net>
 *
 * data.c - Stored Data functions.
 */

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data.h"

Network *network_list = NULL;
Bouquet *bouquet_list = NULL;

/* Network */
Network * network_get (unsigned short network_id) {
	Network *network_ptr;

	for (network_ptr = network_list; network_ptr != NULL; network_ptr = network_ptr->next) {
		if (network_ptr->network_id == network_id) {
			return network_ptr;
		}
	}

	return NULL;
}

void network_add (Network *new_ptr) {
	if (network_list == NULL) {
		new_ptr->next = NULL;
	} else {
		new_ptr->next = network_list;	
	}

	network_list = new_ptr;
}

Network * network_new (void) {
	Network *network_ptr = (Network *) malloc(sizeof(Network));
	memset(network_ptr, '\0', sizeof(Network));
	return network_ptr;
}

/* Transport */
Transport * transport_get (Network *network_ptr, unsigned short transport_id) {
	Transport *transport_ptr;

	for (transport_ptr = network_ptr->transports; transport_ptr != NULL; transport_ptr = transport_ptr->next) {
		if (transport_ptr->transport_id == transport_id) {
			return transport_ptr;
		}
	}

	return NULL;
}

Transport * transport_get_with_original_network_id (unsigned short original_network_id, unsigned short transport_id) {
	Network *network_ptr;
	Transport *transport_ptr;

	for (network_ptr = network_list; network_ptr != NULL; network_ptr = network_ptr->next) {
		for (transport_ptr = network_ptr->transports; transport_ptr != NULL; transport_ptr = transport_ptr->next) {
			if (transport_ptr->transport_id == transport_id && transport_ptr->original_network_id == original_network_id) {
				return transport_ptr;
			}
		}
	}

	return NULL;
}

void transport_add (Network *network_ptr, Transport *new_ptr) {
	if (network_ptr->transports == NULL) {
		new_ptr->next = NULL;
	} else {
		new_ptr->next = network_ptr->transports;
	}

	network_ptr->transports = new_ptr;
}

Transport * transport_new (void) {
	Transport *transport_ptr = (Transport *) malloc(sizeof(Transport));
	memset(transport_ptr, '\0', sizeof(Transport));
	return transport_ptr;
}

/* Service */
Service * service_get (Transport *transport_ptr, unsigned short service_id) {
        Service *service_ptr;

        for (service_ptr = transport_ptr->services; service_ptr != NULL; service_ptr = service_ptr->next) {
                if (service_ptr->service_id == service_id) {
                        return service_ptr;
                }
        }

        return NULL;
}

void service_add (Transport *transport_ptr, Service *new_ptr) {
        if (transport_ptr->services == NULL) {
                new_ptr->next = NULL;
        } else {
                new_ptr->next = transport_ptr->services;
        }

        transport_ptr->services = new_ptr;
}

Service * service_new (void) {
        Service *service_ptr = (Service *) malloc(sizeof(Service));
        memset(service_ptr, '\0', sizeof(Service));
        return service_ptr;
}

/* Bouquet */
Bouquet * bouquet_get (unsigned short bouquet_id) {
        Bouquet *bouquet_ptr;

        for (bouquet_ptr = bouquet_list; bouquet_ptr != NULL; bouquet_ptr = bouquet_ptr->next) {
                if (bouquet_ptr->bouquet_id == bouquet_id) {
                        return bouquet_ptr;
                }
        }

        return NULL;
}

void bouquet_add (Bouquet *new_ptr) {
        if (bouquet_list == NULL) {
                new_ptr->next = NULL;
        } else {
                new_ptr->next = bouquet_list;
        }

        bouquet_list = new_ptr;
}

Bouquet * bouquet_new (void) {
        Bouquet *bouquet_ptr = (Bouquet *) malloc(sizeof(Bouquet));
        memset(bouquet_ptr, '\0', sizeof(Bouquet));
        return bouquet_ptr;
}

/* OpenTVChannel */
OpenTVChannel * opentv_channel_get (Bouquet *bouquet_ptr, unsigned short channel_number) {
        OpenTVChannel *opentv_channel_ptr;

        for (opentv_channel_ptr = bouquet_ptr->channels; opentv_channel_ptr != NULL; opentv_channel_ptr = opentv_channel_ptr->next) {
                if (opentv_channel_ptr->channel_number == channel_number) {
                        return opentv_channel_ptr;
                }
        }

        return NULL;
}

void opentv_channel_add (Bouquet *bouquet_ptr, OpenTVChannel *new_ptr) {
        if (bouquet_ptr->channels == NULL) {
                new_ptr->next = NULL;
        } else {
                new_ptr->next = bouquet_ptr->channels;
        }

        bouquet_ptr->channels = new_ptr;
}

OpenTVChannel * opentv_channel_new (void) {
        OpenTVChannel *opentv_channel_ptr = (OpenTVChannel *) malloc(sizeof(OpenTVChannel));
        memset(opentv_channel_ptr, '\0', sizeof(OpenTVChannel));
        return opentv_channel_ptr;
}

int section_tracking_check (SectionTracking *section_tracking) {
	int i = 0;
	
	for (i = 0; i <= section_tracking->last_section; i++) {
		if (section_tracking->received_section[i] == 0)
			return 0;
	}

	return 1;
}
