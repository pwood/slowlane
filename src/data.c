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
#include "slowlane.h"
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

Bouquet * filter_data (int filter_bouquet_id, unsigned char filter_region_count, unsigned char *filter_region, int filter_dvbs, int filter_hd, int filter_user_number) {
	/* Temporary variables. */
	Bouquet *final_bouquet;
	Bouquet *bouquet;
	OpenTVChannel *channel, *next_channel;
	int i = 0, done = 0;

	/* Create our bouquet with everything we need. */
	final_bouquet = bouquet_new();

	/* Process BAT/SMT data to form channnel list. */
	for (bouquet = bouquet_list; bouquet != NULL; bouquet = bouquet->next) {
		if (filter_bouquet_id == 0 || filter_bouquet_id == bouquet->bouquet_id) {
			for (channel = bouquet->channels; channel != NULL;) {
				next_channel = channel->next;
				done = 0;

				if (filter_region_count) {
					for (i = 0; i < filter_region_count; i++) {
						if (filter_region[i] == channel->region) {
							done = 1;
						}
					}
				} else {
					done = 1;
				}

				if (done) {
					channel->transport = transport_get_with_original_network_id(channel->original_network_id, channel->transport_id);

					if (!channel->transport) {
						slowlane_log(1, "Could not find transport %i on network %i for bouquet %i and service %i.", channel->transport_id, channel->original_network_id, bouquet->bouquet_id, channel->service_id);
					} else {
						if (channel->transport->modulation_system >= filter_dvbs) {
							slowlane_log(3, "Transport %i is a modulation system of %i, however program running in DVB-S%i mode.", channel->transport->transport_id, channel->transport->modulation_system, filter_dvbs);
						} else {
							if (channel->transport->frequency < 1000000 || channel->transport->frequency > 1400000) {
								slowlane_log(2, "Transport %i has frequency (%i) outside of the Ku band, ignoring as DVB-S cards can't tune it.", channel->transport->transport_id, channel->transport->frequency);
							} else {
								channel->service = service_get(channel->transport, channel->service_id);

								if (!channel->service) {
									slowlane_log(1, "Could not find service %i on network %i for bouquet %i and transport %i.", channel->service_id, channel->original_network_id, bouquet->bouquet_id, channel->transport_id);
								} else {
									if (channel->service->type == 1 || channel->service->type == 2 || channel->service->type == 4 || channel->service->type == 5 || channel->service->type == 25) {
										if (channel->service->type == 25 && !filter_hd) {
											slowlane_log(3, "Ignoring service %i:%i as is HD, running not in HD mode.", channel->service->service_id, channel->transport->transport_id);
										} else {
											if (channel->user_number > filter_user_number) {
												slowlane_log(3, "Ignoring service %i:%i as user number %i is above %i.", channel->service->service_id, channel->transport->transport_id, channel->user_number, filter_user_number);
											} else {
												channel->next = NULL;
												opentv_channel_add(final_bouquet, channel);
											}
										}
									}
								}
							}
						}
					}
				}

				channel = next_channel;
			}
		}
	}

	return final_bouquet;
}
