/* BGP Notification state handling
 * Copyright (C) 1996, 97, 98 Kunihiro Ishiguro
 *
 * Recast for pthreaded bgpd: Copyright (C) Chris Hall (GMCH), Highwayman
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <string.h>

#include "lib/zassert.h"
#include "lib/memory.h"

#include "bgpd/bgp_notification.h"

/*==============================================================================
 * A bgp_notify structure encapsulates the contents of a BGP NOTIFICATION
 * message.
 *
 *
 */

/*------------------------------------------------------------------------------
 * Calculate size for bgp_notify of given length
 *
 * Rounds up to multiple of 32, such that is always at least 16 bytes available.
 */
static inline bgp_size_t
bgp_notify_size(bgp_size_t length)
{
  return sizeof(struct bgp_notify) + (((length + 32 + 16 - 1) / 32) * 32) ;
} ;

/*==============================================================================
 * Create/Destroy bgp_notify
 */

/*------------------------------------------------------------------------------
 * Allocate and initialise new notification
 *
 * Can specify an expected amount of data.
 */
extern bgp_notify
bgp_notify_new(bgp_nom_code_t code, bgp_nom_subcode_t subcode,
                                                              bgp_size_t expect)
{
  bgp_notify notification ;
  bgp_size_t size = bgp_notify_size(expect) ;

  notification = XCALLOC(MTYPE_BGP_NOTIFY, size) ;

  notification->code    = code ;
  notification->subcode = subcode ;
  notification->size    = size ;
  notification->length  = 0 ;

  return notification ;
} ;

/*------------------------------------------------------------------------------
 * Free notification structure
 *
 * Does nothing if there is no structure.
 */
extern void
bgp_notify_free(bgp_notify notification)
{
  if (notification != NULL)
    XFREE(MTYPE_BGP_NOTIFY, notification) ;
} ;

/*------------------------------------------------------------------------------
 * Duplicate existing notification (if any)
 */
extern bgp_notify
bgp_notify_dup(bgp_notify notification)
{
  bgp_notify duplicate ;
  bgp_size_t size ;

  if (notification == NULL)
    return NULL ;

  size = bgp_notify_size(notification->length) ;
  duplicate = XMALLOC(MTYPE_BGP_NOTIFY, size) ;

  memcpy((void*)duplicate, (void*)notification, size) ;
  duplicate->size = size ;

  return duplicate ;
} ;

/*------------------------------------------------------------------------------
 * Unset pointer to notification and free any existing notification structure.
 *
 * Does nothing if there is no structure.
 */
extern void
bgp_notify_unset(bgp_notify* p_notification)
{
  if (*p_notification != NULL)
    XFREE(MTYPE_BGP_NOTIFY, *p_notification) ;  /* sets *p_notification NULL */
} ;

/*------------------------------------------------------------------------------
 * Set pointer to notification
 *
 * Frees any existing notification at the destination.
 *
 * NB: copies the source pointer -- so must be clear about responsibility
 *     for the notification structure.
 */
extern void
bgp_notify_set(bgp_notify* p_dst, bgp_notify src)
{
  bgp_notify_free(*p_dst) ;
  *p_dst = src ;
} ;

/*------------------------------------------------------------------------------
 * Set pointer to notification to a *copy* of the source.
 *
 * Frees any existing notification at the destination.
 */
extern void
bgp_notify_set_dup(bgp_notify* p_dst, bgp_notify src)
{
  bgp_notify_set(p_dst, bgp_notify_dup(src)) ;
} ;

/*------------------------------------------------------------------------------
 * Set pointer to notification and unset source pointer
 *
 * Frees any existing notification at the destination.
 *
 * NB: responsibility for the notification structure passes to the destination.
 */
extern void
bgp_notify_set_mov(bgp_notify* p_dst, bgp_notify* p_src)
{
  bgp_notify_free(*p_dst) ;
  *p_dst = *p_src ;
  *p_src = NULL ;
} ;

/*==============================================================================
 * Append data to given notification
 *
 * Copes with zero length append.
 *
 * NB: returns possibly NEW ADDRESS of the notification.
 */
extern bgp_notify
bgp_notify_append_data(bgp_notify notification, const void* data,
                                                                 bgp_size_t len)
{
  bgp_size_t new_length = notification->length + len ;

  if ((sizeof(struct bgp_notify) + new_length) > notification->size)
    {
      bgp_size_t size = bgp_notify_size(new_length) ;
      notification = XREALLOC(MTYPE_BGP_NOTIFY, notification, size) ;
      memset((char*)notification + notification->size, 0,
                                                    size - notification->size) ;
      notification->size = size ;
    } ;

  if (len > 0)
    memcpy((char*)(notification->data) + notification->length, data, len) ;

  notification->length = new_length ;

  return notification ;
} ;