/*****************************************************************************
 * wxwindows.cpp : wxWindows plugin for vlc
 *****************************************************************************
 * Copyright (C) 2000-2001 VideoLAN
 * $Id: wxwindows.cpp,v 1.3 2002/11/20 15:58:15 gbazin Exp $
 *
 * Authors: Gildas Bazin <gbazin@netcourrier.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 *****************************************************************************/

/*****************************************************************************
 * Preamble
 *****************************************************************************/
#include <stdlib.h>                                      /* malloc(), free() */
#include <errno.h>                                                 /* ENOMEM */
#include <string.h>                                            /* strerror() */
#include <stdio.h>

#include <vlc/vlc.h>
#include <vlc/intf.h>

/* Let wxWindows take care of the i18n stuff */
#undef _

#ifdef WIN32                                                 /* mingw32 hack */
#undef Yield
#undef CreateDialog
#endif

#include <wx/wxprec.h>
#include <wx/wx.h>
#include <wx/imagpng.h>

#include "wxwindows.h"

/*****************************************************************************
 * Local prototypes.
 *****************************************************************************/
static int  Open         ( vlc_object_t * );
static void Close        ( vlc_object_t * );

static void Run          ( intf_thread_t * );

/*****************************************************************************
 * Local classes declarations.
 *****************************************************************************/
class Instance: public wxApp
{
public:
    Instance();
    Instance( intf_thread_t *_p_intf );

    bool OnInit();

private:
    intf_thread_t *p_intf;
};

/*****************************************************************************
 * Module descriptor
 *****************************************************************************/
vlc_module_begin();
    add_category_hint( N_("wxWindows"), NULL );
    set_description( (char *) _("wxWindows interface module") );
    set_capability( "interface", 50 );
    set_callbacks( Open, Close );
    set_program( "wxvlc" );
vlc_module_end();

/*****************************************************************************
 * Open: initialize and create window
 *****************************************************************************/
static int Open( vlc_object_t *p_this )
{
    intf_thread_t *p_intf = (intf_thread_t *)p_this;

    /* Allocate instance and initialize some members */
    p_intf->p_sys = (intf_sys_t *)malloc( sizeof( intf_sys_t ) );
    if( p_intf->p_sys == NULL )
    {
        msg_Err( p_intf, "out of memory" );
        return VLC_ENOMEM;
    }

    p_intf->pf_run = Run;

    p_intf->p_sys->p_sub = msg_Subscribe( p_intf );

    /* Initialize wxWindows thread */
    p_intf->p_sys->b_playing = 0;
    p_intf->p_sys->b_popup_changed = 0;
    p_intf->p_sys->b_window_changed = 0;
    p_intf->p_sys->b_playlist_changed = 0;

    p_intf->p_sys->p_input = NULL;
    p_intf->p_sys->i_playing = -1;
    p_intf->p_sys->b_slider_free = 1;
    p_intf->p_sys->i_slider_pos = p_intf->p_sys->i_slider_oldpos = 0;

    p_intf->p_sys->i_part = -1;

    return VLC_SUCCESS;
}

/*****************************************************************************
 * Close: destroy interface window
 *****************************************************************************/
static void Close( vlc_object_t *p_this )
{
    intf_thread_t *p_intf = (intf_thread_t *)p_this;

    if( p_intf->p_sys->p_input )
    {
        vlc_object_release( p_intf->p_sys->p_input );
    }

    msg_Unsubscribe( p_intf, p_intf->p_sys->p_sub );

    /* Destroy structure */
    free( p_intf->p_sys );
}

/*****************************************************************************
 * Run: wxWindows thread
 *****************************************************************************/
static void Run( intf_thread_t *p_intf )
{
    static char  *p_args[] = { "" };

    /* Hack to pass the p_intf pointer to the new wxWindow Instance object */
    wxTheApp = new Instance( p_intf );

#if defined( WIN32 )
    wxEntry( GetModuleHandle(NULL), NULL, NULL, SW_SHOW, TRUE );
#else
    wxEntry( 1, p_args );
#endif
}

/* following functions are local */

/*****************************************************************************
 * Constructors.
 *****************************************************************************/
Instance::Instance( )
{
}

Instance::Instance( intf_thread_t *_p_intf )
{
    /* Initialization */
    p_intf = _p_intf;
}

IMPLEMENT_APP_NO_MAIN(Instance)

/*****************************************************************************
 * Instance::OnInit: the parent interface execution starts here
 *****************************************************************************
 * This is the "main program" equivalent, the program execution will
 * start here.
 *****************************************************************************/
bool Instance::OnInit()
{
    /* Make an instance of your derived frame. Passing NULL (the default value
     * of Frame's constructor is NULL) as the frame doesn't have a frame
     * since it is the first window */
    Interface *MainInterface = new Interface( p_intf );

    /* Show the interface */
    MainInterface->Show(TRUE);

    SetTopWindow(MainInterface);

    /* Start timer */
    new Timer( p_intf, MainInterface );

    /* Return TRUE to tell program to continue (FALSE would terminate) */
    return TRUE;
}
