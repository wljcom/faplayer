/**
 * @file keys.c
 * @brief X C Bindings VLC keyboard event handling
 */
/*****************************************************************************
 * Copyright © 2009 Rémi Denis-Courmont
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>

#include <xcb/xcb.h>
#include <vlc_common.h>
#include "xcb_vlc.h"

#ifdef HAVE_XCB_KEYSYMS
#include <xcb/xcb_keysyms.h>
#include <X11/keysym.h>
#include <X11/XF86keysym.h>
#include <vlc_keys.h>

struct key_handler_t
{
    vlc_object_t      *obj;
    xcb_key_symbols_t *syms;
};

/**
 * Create an X11 key event handler for a VLC window.
 * The caller shall arrange receiving applicable X11 events, and pass them to
 * ProcessKeyEvent() later.
 *
 * @param obj VLC object owning an X window
 * @param conn XCB connection to the X server (to fetch key mappings)
 * @return NULL on error, or a key handling context.
 */
key_handler_t *CreateKeyHandler (vlc_object_t *obj, xcb_connection_t *conn)
{
    key_handler_t *ctx = malloc (sizeof (*ctx));
    if (!ctx)
        return NULL;

    ctx->obj = obj;
    ctx->syms = xcb_key_symbols_alloc (conn);
    return ctx;
}

void DestroyKeyHandler (key_handler_t *ctx)
{
    xcb_key_symbols_free (ctx->syms);
    free (ctx);
}

static int keysymcmp (const void *pa, const void *pb)
{
    int a = *(const xcb_keysym_t *)pa;
    int b = *(const xcb_keysym_t *)pb;

    return a - b;
}

static uint_fast32_t ConvertKeySym (xcb_keysym_t sym)
{
    static const struct
    {
        xcb_keysym_t x11;
        uint32_t vlc;
    } *res, tab[] = {
#include "xcb_keysym.h"
    }, old[] = {
#include "keysym.h"
    };

    /* X11 Latin-1 range */
    if (sym <= 0xff)
        return sym;
    /* X11 Unicode range */
    if (sym >= 0x1000100 && sym <= 0x110ffff)
        return sym - 0x1000000;

    /* Special keys */
    res = bsearch (&sym, tab, sizeof (tab) / sizeof (tab[0]), sizeof (tab[0]),
                   keysymcmp);
    if (res != NULL)
        return res->vlc;
    /* Legacy X11 symbols outside the Unicode range */
    res = bsearch (&sym, old, sizeof (old) / sizeof (old[0]), sizeof (old[0]),
                   keysymcmp);
    if (res != NULL)
        return res->vlc;

    return KEY_UNSET;
}


/**
 * Process an X11 event, convert into VLC hotkey event if applicable.
 *
 * @param ctx key handler created with CreateKeyHandler()
 * @param ev XCB event to process
 * @return 0 if the event was handled and free()'d, non-zero otherwise
 */
int ProcessKeyEvent (key_handler_t *ctx, xcb_generic_event_t *ev)
{
    assert (ctx);

    switch (ev->response_type & 0x7f)
    {
        case XCB_KEY_PRESS:
        {
            xcb_key_press_event_t *e = (xcb_key_press_event_t *)ev;
            xcb_keysym_t sym = xcb_key_press_lookup_keysym (ctx->syms, e, 0);
            uint_fast32_t vk = ConvertKeySym (sym);

            msg_Dbg (ctx->obj, "key: 0x%08"PRIxFAST32, vk);
            if (vk == KEY_UNSET)
                break;
            if (e->state & XCB_MOD_MASK_SHIFT)
                vk |= KEY_MODIFIER_SHIFT;
            if (e->state & XCB_MOD_MASK_CONTROL)
                vk |= KEY_MODIFIER_CTRL;
            if (e->state & XCB_MOD_MASK_1)
                vk |= KEY_MODIFIER_ALT;
            if (e->state & XCB_MOD_MASK_4)
                vk |= KEY_MODIFIER_META;
            var_SetInteger (ctx->obj->p_libvlc, "key-pressed", vk);
            break;
        }

        case XCB_KEY_RELEASE:
            break;

        case XCB_MAPPING_NOTIFY:
        {
            xcb_mapping_notify_event_t *e = (xcb_mapping_notify_event_t *)ev;
            msg_Dbg (ctx->obj, "refreshing keyboard mapping");
            xcb_refresh_keyboard_mapping (ctx->syms, e);
            break;
        }

        default:
            return -1;
    }

    free (ev);
    return 0;
}

#else /* HAVE_XCB_KEYSYMS */

key_handler_t *CreateKeyHandler (vlc_object_t *obj, xcb_connection_t *conn)
{
    msg_Err (obj, "X11 key press support not compiled-in");
    (void) conn;
    return NULL;
}

void DestroyKeyHandler (key_handler_t *ctx)
{
    (void) ctx;
    abort ();
}

int ProcessKeyEvent (key_handler_t *ctx, xcb_generic_event_t *ev)
{
    (void) ctx;
    (void) ev;
    abort ();
}

#endif /* HAVE_XCB_KEYSYMS */
