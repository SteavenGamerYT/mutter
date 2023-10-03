/*
 * Wayland Support
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: José Expósito <jose.exposito89@gmail.com>
 */

#include "config.h"

#include "wayland/meta-wayland-pointer-gesture-hold.h"

#include <glib.h>

#include "wayland/meta-wayland-pointer.h"
#include "wayland/meta-wayland-seat.h"
#include "wayland/meta-wayland-surface.h"

#include "pointer-gestures-unstable-v1-server-protocol.h"

static void
handle_hold_begin (MetaWaylandPointer *pointer,
                   const ClutterEvent *event)
{
  MetaWaylandPointerClient *pointer_client;
  MetaWaylandSeat *seat;
  struct wl_resource *resource;
  uint32_t serial, fingers;

  pointer_client = pointer->focus_client;
  seat = meta_wayland_pointer_get_seat (pointer);
  serial = wl_display_next_serial (seat->wl_display);
  fingers = clutter_event_get_touchpad_gesture_finger_count (event);

  pointer_client->active_touchpad_gesture = clutter_event_type (event);

  wl_resource_for_each (resource, &pointer_client->hold_gesture_resources)
    {
      zwp_pointer_gesture_hold_v1_send_begin (resource, serial,
                                              clutter_event_get_time (event),
                                              pointer->focus_surface->resource,
                                              fingers);
    }
}

static void
broadcast_end (MetaWaylandPointer *pointer,
               uint32_t            serial,
               uint32_t            time,
               gboolean            cancelled)
{
  MetaWaylandPointerClient *pointer_client;
  struct wl_resource *resource;

  pointer_client = pointer->focus_client;

  wl_resource_for_each (resource, &pointer_client->hold_gesture_resources)
    {
      zwp_pointer_gesture_hold_v1_send_end (resource, serial,
                                            time, cancelled);
    }

  pointer_client->active_touchpad_gesture = CLUTTER_NOTHING;
}

static void
handle_hold_end (MetaWaylandPointer *pointer,
                 const ClutterEvent *event)
{
  MetaWaylandSeat *seat;
  gboolean cancelled = FALSE;
  uint32_t serial;

  seat = meta_wayland_pointer_get_seat (pointer);
  serial = wl_display_next_serial (seat->wl_display);

  if (clutter_event_get_gesture_phase (event) ==
      CLUTTER_TOUCHPAD_GESTURE_PHASE_CANCEL)
    cancelled = TRUE;

  broadcast_end (pointer, serial,
                 clutter_event_get_time (event),
                 cancelled);
}

gboolean
meta_wayland_pointer_gesture_hold_handle_event (MetaWaylandPointer *pointer,
                                                const ClutterEvent *event)
{
  if (clutter_event_type (event) != CLUTTER_TOUCHPAD_HOLD)
    return FALSE;

  if (!pointer->focus_client)
    return FALSE;

  switch (clutter_event_get_gesture_phase (event))
    {
    case CLUTTER_TOUCHPAD_GESTURE_PHASE_BEGIN:
      handle_hold_begin (pointer, event);
      break;
    case CLUTTER_TOUCHPAD_GESTURE_PHASE_END:
    case CLUTTER_TOUCHPAD_GESTURE_PHASE_CANCEL:
      handle_hold_end (pointer, event);
      break;
    default:
      return FALSE;
    }

  return TRUE;
}

static void
pointer_gesture_hold_release (struct wl_client   *client,
                              struct wl_resource *resource)
{
  wl_resource_destroy (resource);
}

static const struct zwp_pointer_gesture_hold_v1_interface pointer_gesture_hold_interface = {
  pointer_gesture_hold_release
};

void
meta_wayland_pointer_gesture_hold_create_new_resource (MetaWaylandPointer *pointer,
                                                       struct wl_client   *client,
                                                       struct wl_resource *pointer_resource,
                                                       uint32_t            id)
{
  MetaWaylandPointerClient *pointer_client;
  struct wl_resource *res;

  res = wl_resource_create (client, &zwp_pointer_gesture_hold_v1_interface,
                            wl_resource_get_version (pointer_resource), id);
  wl_resource_set_implementation (res, &pointer_gesture_hold_interface, pointer,
                                  meta_wayland_pointer_unbind_pointer_client_resource);

  if (pointer)
    {
      pointer_client = meta_wayland_pointer_get_pointer_client (pointer, client);
      g_return_if_fail (pointer_client != NULL);

      wl_list_insert (&pointer_client->hold_gesture_resources,
                      wl_resource_get_link (res));
    }
}

void
meta_wayland_pointer_gesture_hold_cancel (MetaWaylandPointer *pointer,
                                          uint32_t            serial)
{
  broadcast_end (pointer, serial,
                 us2ms (g_get_monotonic_time ()),
                 TRUE);
}