/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 * Copyright (C) 2001 SuSE Linux AG
 * Author: Martin Baulig <baulig@suse.de>
 *
 * This file is part of the Gnome Library.
 *
 * The Gnome Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * The Gnome Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
/*
  @NOTATION@
 */

#include <config.h>

#include <stdio.h>
#include <string.h>
#include <gobject/gsignal.h>
#include <libgnome/gnome-marshal.h>
#include <libgnome/gnome-directory-filter.h>
#include <libgnome/gnome-async-context.h>

struct _GnomeDirectoryFilterPrivate {
    BonoboEventSource *event_source;
    GnomeAsyncContext *async_ctx;
};

static void gnome_directory_filter_class_init   (GnomeDirectoryFilterClass *class);
static void gnome_directory_filter_init         (GnomeDirectoryFilter      *filter);
static void gnome_directory_filter_finalize     (GObject                   *object);

#define PARENT_TYPE BONOBO_OBJECT_TYPE
BonoboObjectClass *gnome_directory_filter_parent_class = NULL;

enum {
    CHECK_URI_SIGNAL,
    SCAN_DIRECTORY_SIGNAL,
    LAST_SIGNAL
};

static int gnome_directory_filter_signals [LAST_SIGNAL] = {0};

static Bonobo_EventSource
impl_GNOME_DirectoryFilter_getEventSource (PortableServer_Servant servant,
					   CORBA_Environment * ev)
{
    GnomeDirectoryFilter *filter = GNOME_DIRECTORY_FILTER (bonobo_object_from_servant (servant));
    Bonobo_EventSource corba_es;

    corba_es = BONOBO_OBJREF (filter->_priv->event_source);

    return bonobo_object_dup_ref (corba_es, ev);
}

static void
check_uri_async_cb (GnomeAsyncContext *async_ctx, GnomeAsyncHandle *async_handle,
		    GnomeAsyncType async_type, GObject *object,
		    const gchar *uri, const gchar *error, gboolean success, gpointer user_data)
{
    g_message (G_STRLOC);
}

static void
impl_GNOME_DirectoryFilter_checkURI (PortableServer_Servant servant,
				     const CORBA_char * uri, CORBA_Environment * ev)
{
    GnomeDirectoryFilter *filter = GNOME_DIRECTORY_FILTER (bonobo_object_from_servant (servant));
    GnomeAsyncHandle *async_handle;

    async_handle = gnome_async_context_get (filter->_priv->async_ctx, 0, check_uri_async_cb,
					    G_OBJECT (filter), NULL, NULL, NULL);

    g_signal_emit (G_OBJECT (filter),
		   gnome_directory_filter_signals [CHECK_URI_SIGNAL], 0,
		   uri, async_handle);
}

static void
impl_GNOME_DirectoryFilter_scanDirectory (PortableServer_Servant servant,
					  const CORBA_char * directory,
					  CORBA_Environment * ev)
{
}

static void
gnome_directory_filter_check_uri (GnomeDirectoryFilter *filter, const gchar *uri,
				  GnomeAsyncHandle *async_handle)
{
    g_message (G_STRLOC ": `%s'", uri);

    gnome_async_handle_completed (async_handle, FALSE);
}

static void
gnome_directory_filter_class_init (GnomeDirectoryFilterClass *klass)
{
    GObjectClass *object_class;
    POA_GNOME_DirectoryFilter__epv *epv = &klass->epv;

    object_class = (GObjectClass *) klass;

    gnome_directory_filter_parent_class = g_type_class_peek_parent (klass);

    gnome_directory_filter_signals [CHECK_URI_SIGNAL] =
	g_signal_newc ("check_uri",
		       G_TYPE_FROM_CLASS (object_class),
		       G_SIGNAL_RUN_LAST,
		       G_STRUCT_OFFSET (GnomeDirectoryFilterClass,
					check_uri),
		       NULL, NULL,
		       gnome_marshal_VOID__STRING_BOXED,
		       G_TYPE_NONE, 2,
		       G_TYPE_STRING,
		       GNOME_TYPE_ASYNC_HANDLE);

    gnome_directory_filter_signals [SCAN_DIRECTORY_SIGNAL] =
	g_signal_newc ("scan_directory",
		       G_TYPE_FROM_CLASS (object_class),
		       G_SIGNAL_RUN_LAST,
		       G_STRUCT_OFFSET (GnomeDirectoryFilterClass,
					check_uri),
		       NULL, NULL,
		       gnome_marshal_VOID__STRING_BOXED,
		       G_TYPE_NONE, 2,
		       G_TYPE_STRING,
		       GNOME_TYPE_ASYNC_HANDLE);

    epv->getEventSource = impl_GNOME_DirectoryFilter_getEventSource;
    epv->checkURI = impl_GNOME_DirectoryFilter_checkURI;
    epv->scanDirectory = impl_GNOME_DirectoryFilter_scanDirectory;

    klass->check_uri = gnome_directory_filter_check_uri;

    object_class->finalize = gnome_directory_filter_finalize;
}

static void
gnome_directory_filter_init (GnomeDirectoryFilter *filter)
{
    filter->_priv = g_new0 (GnomeDirectoryFilterPrivate, 1);
}

static void
gnome_directory_filter_finalize (GObject *object)
{
    GnomeDirectoryFilter *filter;

    g_return_if_fail (object != NULL);
    g_return_if_fail (GNOME_IS_DIRECTORY_FILTER (object));

    filter = GNOME_DIRECTORY_FILTER (object);

    g_free (filter->_priv);
    filter->_priv = NULL;

    if (G_OBJECT_CLASS (gnome_directory_filter_parent_class)->finalize)
	(* G_OBJECT_CLASS (gnome_directory_filter_parent_class)->finalize) (object);
}

GnomeDirectoryFilter *
gnome_directory_filter_new (void)
{
    GnomeDirectoryFilter *filter;

    filter = g_object_new (gnome_directory_filter_get_type (), NULL);

    filter->_priv->event_source = bonobo_event_source_new ();
    filter->_priv->async_ctx = gnome_async_context_new ();

    return filter;
}

BONOBO_TYPE_FUNC_FULL (GnomeDirectoryFilter, GNOME_DirectoryFilter,
		       PARENT_TYPE, gnome_directory_filter);