/*
    $Id: hw_checksum.h,v 1.3 2008-01-12 16:10:19 awgn Exp $

    Copyright (c) 2003 Nicola Bonelli <bonelli@antifork.org>
                                       <bonelli@netserv.iet.unipi.it>


    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef HW_CHECKSUM_H
#define HW_CHECKSUM_H

/*
 * This list contains the ethernet adapters with the support
 * of IP hw_checksum
 */

char *hw_checksum_list[] = {
	"8139cp",
	"acenic"
	"D-Link DL2000-based linux driver",
	"hamachi",
	"ns83820",
	"starfire",
	"sungem",
	"tg3",
	"typhoon",
	"via-rhine",
};

#endif /* HW_CHECKSUM_H */
