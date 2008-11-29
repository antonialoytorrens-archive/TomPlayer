/* Imported from the dvbstream project
 *
 * Modified for use with MPlayer, for details see the changelog at
 * http://svn.mplayerhq.hu/mplayer/trunk/
 * $Id: rtp.h 25563 2007-12-31 16:15:50Z diego $
 */

#ifndef RTP_H
#define RTP_H

int read_rtp_from_server(int fd, char *buffer, int length);

#endif /* RTP_H */
