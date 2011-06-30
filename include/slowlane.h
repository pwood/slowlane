/* Slowlane - Utility to populate and maintain the MythTV channels tables
 * with data extracted from the propriatary Media Highway middleware data.
 *
 * Peter Wood <peter+slowlane@alastria.net>
 *
 * slowlane.h - Program wide settings and values.
 */

#ifndef __SLOWLANE_H_
#define __SLOWLANE_H_ 1

#define SLOWLANE_NAME "slowlane"
#define SLOWLANE_VERSION "0.0.1"
#define SLOWLANE_AUTHOR "Peter Wood <peter+slowlane@alastria.net>"

#include <err.h>

extern int verbose;

#define slowlane_log(v, fmt, ...) do { if (v <= verbose) { warnx("[%s:%u:%i] " fmt, __FILE__, __LINE__, v, __VA_ARGS__); } } while (0)

#endif
