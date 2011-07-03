/* Slowlane - Utility to populate and maintain the MythTV channels tables
 * with data extracted from the propriatary Media Highway middleware data.
 *
 * Peter Wood <peter+slowlane@alastria.net>
 *
 * data.c - Stored Data functions.
 */

/* Includes */
#include <stdio.h>
#include "data.h"

Network *network_list = NULL;
Bouquet *bouquet_list = NULL;
