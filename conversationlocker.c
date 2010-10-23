/*
 * conversationlocker - A Pidgin plugin for preventing
 *				    accidental closing of important conversations
 * Copyright (C) 2009 - Peeyoosh Sangolekar <piyush_sangolekar@hotmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* for win32 compile, config.h includes package definition */
//#ifndef _WIN32
#define PACKAGE "pidgin-conversation-locker"
//#endif

/* internationalization headers */
#ifdef ENABLE_NLS
#  include <locale.h>
#  include <libintl.h>
#  define _(String) ((const char *)dgettext(PACKAGE, String))
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  include <locale.h>
#  define N_(String) (String)
#  ifndef _
#    define _(String) ((const char *)String)
#  endif
#  define ngettext(Singular, Plural, Number) ((Number == 1) ? ((const char *)Singular) : ((const char *)Plural))
#  define dngettext(Domain, Singular, Plural, Number) ((Number == 1) ? ((const char *)Singular) : ((const char *)Plural))
#endif


#ifndef G_GNUC_NULL_TERMINATED
#  if __GNUC__ >= 4
#    define G_GNUC_NULL_TERMINATED __attribute__((__sentinel__))
#  else
#    define G_GNUC_NULL_TERMINATED
#  endif /* __GNUC__ >= 4 */
#endif /* G_GNUC_NULL_TERMINATED */

#include "debug.h"
//#include "internal.h"

#include "gtkconv.h"
#include "gtkplugin.h"
#include "gtkprefs.h"
#include "gtkrequest.h"
#include "gtkutils.h"
#include "pidgin.h"
#include "pluginpref.h"
#include "version.h"

#include "conversationlocker-icons.h"

#define PLUGIN_AUTHOR "Peeyoosh Sangolekar <piyush_sangolekar@hotmail.com>"
#define PLUGIN_WEBSITE "http://pidgin-conversation-locker.googlecode.com"
#define PLUGIN_ID "gtk-enli-conversation-locker"
#define PLUGIN_VERSION "1.1"

#define	PREFS_PREFIX		"/plugins/gtk/" PLUGIN_ID
#define	PREFS_TEXT			PREFS_PREFIX "/text"
#define	PREFS_ICON			PREFS_PREFIX "/icon"
#define PREFS_BUTTON_ORDER	PREFS_PREFIX "/button_order"

static PurplePlugin *plugin_handle = NULL;

static GHashTable *hash_table;

static gboolean stop_window_closing_cb(GtkWidget *, GdkEventAny *, PidginWindow *);

static gulong get_handler_id(gchar *, gpointer);

static void toggle_widget_text(PidginConversation *);


/* block pidgins default signal handler and connect plugin`s */
static void
block_pidgin_handler(PidginWindow *win, gulong handler_id)
{
	handler_id = get_handler_id("delete_event", win->window);
	g_signal_handler_block (G_OBJECT(win->window), handler_id);
	g_signal_connect(G_OBJECT(win->window), "delete_event",
				       G_CALLBACK(stop_window_closing_cb), win);

	g_hash_table_insert(hash_table, win->window, (gpointer)handler_id);
}

/* disconnect plugin`s signal handler and restore pidgins`s default handler*/
static void
unblock_pidgin_handler(gulong default_handler_id, PidginWindow *win)
{
	gulong handler_id;

	handler_id = g_signal_handlers_disconnect_matched(win->window,
											G_SIGNAL_MATCH_FUNC,
											0, 0, NULL,
											G_CALLBACK(stop_window_closing_cb),
											NULL);

	purple_debug_info(PLUGIN_ID, "number of handlers disconnected = %lu\n", handler_id);

	default_handler_id = (gulong)g_hash_table_lookup(hash_table, win->window);
	g_signal_handler_unblock(G_OBJECT(win->window), default_handler_id);
	g_hash_table_remove(hash_table, win->window);
}


/* returns number of locked conversations */
static gint
get_locked_conversations(PidginWindow *win)
{
	GList *l;
	PidginConversation *gtkconv;
	gint locked_conversations = 0;

	/* gtkconv->win->gtkconvs returns list of pidginconversations under current window */
	for (l = win->gtkconvs; l!=NULL; l=l->next) {
		gtkconv = l->data;

		if (purple_conversation_get_data(gtkconv->active_conv, "conversation_locker_status") \
		     == (gchar *)"locked") {
			++locked_conversations;
		}
	}
	return locked_conversations;
}

static void
close_window(PidginWindow *win, gboolean emit)
{
	gulong default_handler_id = (gulong)g_hash_table_lookup(hash_table, win->window);

	unblock_pidgin_handler(default_handler_id, win);

	/* used as emit=FALSE for unlock_all_conversations_action(), as we don't want window to close on us */
	if (emit) {
		purple_debug_info(PLUGIN_ID, "Emiting delete_event on window.");
		g_signal_emit_by_name(G_OBJECT(win->window), "delete_event", win, NULL);
	}
}

/* user chose to close window */
static void
option_yes_cb(PidginWindow *win)
{
	close_window(win, TRUE);
}


/* if no locked conversations are found, disconnect and restore handles *
 * else, ask user
 */
static gboolean
stop_window_closing_cb(GtkWidget *w, GdkEventAny *e, PidginWindow* win)
{
	/*
	gint locked_conversations;

	if((locked_conversations = get_locked_conversations(win)) != 0){
	*/

    gboolean windows_style_button_order = purple_prefs_get_bool(PREFS_BUTTON_ORDER) ? TRUE: FALSE;

	if(windows_style_button_order) {
		purple_request_action(plugin_handle, "Conversation Locker",
					 	 "Close locked conversations?",
						 "There are few conversations which are still locked." \
						 " By choosing to close window, those locked conversations will be lost." \
						 "\n\nDo you wish to close all conversations including locked conversations?",
						 1, NULL, NULL, NULL, win, 3,
						 _("_Cancel"), NULL, _("_No"), NULL, _("_Yes"), (option_yes_cb));

	} else {
		purple_request_action(plugin_handle, "Conversation Locker",
					 	 "Close locked conversations?",
						 "There are few conversations which are still locked." \
						 " By choosing to close window, those locked conversations will be lost." \
						 "\n\nDo you wish to close all conversations including locked conversations?",
						 2, NULL, NULL, NULL, win, 3,
						 _("_Yes"), (option_yes_cb), _("_Cancel"), NULL, _("_No"), NULL);
	}
	//} else
		/* It would have been efficient just to call  option_yes_cb(win),
		 * but that crashes pidgin mysteriously. The problem is with g_signal_emit()
		 * Disable it temporary until problem is pinpointed out.
		 * Due to this workaround, for closing a window user needs to press close button
		 * twice on window from which locked tab was detached.
		 *
		 * REASON : when pidgins default "delete_event" signal handler gets executed,
		 * it in turns executes pidgin_conv_window_destroy(PidginWindow *win)
		 * which disconnects all handles of win, using purple_signals_disconnect_by_handle(win)
		 * refer v2.6.2 gtkconv.c #9361, #9384
		 */

		/*
			option_yes_cb(win);
	}*/

	/* stop window from closing */
	return TRUE;
}


static gulong
get_handler_id(gchar *gtkevent, gpointer window)
{
	guint signal_id;
	gulong handler_id;

	signal_id = g_signal_lookup(gtkevent, GTK_TYPE_WINDOW);
	handler_id = g_signal_handler_find((gpointer)window,
							G_SIGNAL_MATCH_ID, signal_id, 0, NULL, NULL, NULL );

	return handler_id;
}


/* visibility=TRUE, unlock conversation, else lock them*/
static void
toggle_conversation_lock(PidginConversation *gtkconv, gboolean visibility)
{
	GtkItemFactory *factory;
	gulong handler_id;
	gulong default_handler_id;
	gulong handler_found;
	gint locked_conversations;

	factory = gtk_item_factory_from_path("<main>");
	locked_conversations = get_locked_conversations(gtkconv->win);

	/* toggle Conversation > Close */
	gtk_widget_set_sensitive((GtkWidget *)gtk_item_factory_get_widget(factory,
							        "<main>/Conversation/Close"), visibility);

	/* toggle X from tabs */
	gtk_widget_set_sensitive((GtkWidget *)gtkconv->close, visibility);

	/* get current window`s delete_event handler */
	handler_id = get_handler_id("delete_event", G_OBJECT(gtkconv->win->window));

	/* check if plugin has replaced pidgins "delete_event" handle */
	handler_found = g_signal_handler_find(gtkconv->win->window, G_SIGNAL_MATCH_FUNC, 0, 0,
                                    NULL, stop_window_closing_cb, NULL);

	/* check if current windows handle was replaced and handle id exists */
	default_handler_id = (gulong)g_hash_table_lookup(hash_table, gtkconv->win->window);

	if (!visibility && default_handler_id == 0)
		block_pidgin_handler(gtkconv->win, handler_id);

	/*if (!visibility && default_handler_id != NULL)*/
	/* conversation has to be locked, but default_handler_id was found which suggests that other conv might be locked
	* so check if any conversation is locked, if no conversation is locked disconect and unblock pidgin handler
	*/
	else if (!visibility && default_handler_id != 0 && handler_found == 0)
				block_pidgin_handler(gtkconv->win, handler_id);

	else if (visibility && locked_conversations == 0)
				unblock_pidgin_handler(default_handler_id, gtkconv->win);
}


static void
lock_button_cb (GtkButton *button, PidginConversation *gtkconv)
{
	GtkWidget *label;
	GtkWidget *icon;
	GdkPixbuf *pixbuf;
	gboolean visibility; 		/* if close options are visible */
	PurpleConversation *conv = gtkconv->active_conv;

	label = g_object_get_data(G_OBJECT(gtkconv->toolbar), "conversation_locker_label");
	icon = g_object_get_data(G_OBJECT(gtkconv->toolbar), "conversation_locker_icon");

	if(purple_conversation_get_data(conv, "conversation_locker_status") != (gchar *)"locked"){
		/* previous status was unlocked, so now lock conversations */
		visibility = FALSE;

		purple_conversation_set_data(conv, "conversation_locker_status", "locked" );

		toggle_conversation_lock(gtkconv, visibility);

		gtk_label_set_markup(GTK_LABEL(label), "<span color=\"#047B00\"> Chat locked </span>");
		pixbuf = gdk_pixbuf_new_from_inline (-1, locked, FALSE, NULL);
		gtk_image_set_from_pixbuf(GTK_IMAGE(icon), pixbuf);

	} else {
		/* unlock previously locked conversations */
		visibility = TRUE;

		purple_conversation_set_data(conv, "conversation_locker_status", NULL);

		toggle_conversation_lock(gtkconv, visibility);

		gtk_label_set_markup(GTK_LABEL(label), "<span color=\"#FF0000\"> Chat unlocked </span>");
		pixbuf = gdk_pixbuf_new_from_inline (-1, unlocked, FALSE, NULL);
		gtk_image_set_from_pixbuf(GTK_IMAGE(icon), pixbuf);
	}

		g_object_unref(pixbuf);
}


static void
create_lock_button_pidgin(PidginConversation *gtkconv)
{
	GtkWidget *lock_button;
	GtkWidget *label;
	GtkWidget *hbox, *hbox1;
	GdkPixbuf *pixbuf;
	GtkWidget *icon;

	lock_button = g_object_get_data(G_OBJECT(gtkconv->toolbar),"conversation_locker_button");

	if (lock_button != NULL) {
        toggle_widget_text(gtkconv);
		return;
    }

	hbox = gtk_hbox_new(FALSE, 0);
	hbox1 = gtk_hbox_new(FALSE, 0);

	lock_button = gtk_button_new();
	gtk_button_set_relief(GTK_BUTTON(lock_button), GTK_RELIEF_NONE);
	g_signal_connect(G_OBJECT(lock_button), "clicked", G_CALLBACK(lock_button_cb), gtkconv);

	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), "<span color=\"#FF0000\"> Chat Unlocked </span>");
	gtk_box_pack_end(GTK_BOX(hbox1), label, FALSE, FALSE, 0);

	pixbuf = gdk_pixbuf_new_from_inline (-1, unlocked, FALSE, NULL);
	icon = gtk_image_new_from_pixbuf(pixbuf);
	gtk_box_pack_start(GTK_BOX(hbox1), icon, FALSE, FALSE, 0);

	gtk_container_add(GTK_CONTAINER(lock_button), hbox1);

	gtk_box_pack_start(GTK_BOX(hbox), lock_button, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(gtkconv->toolbar), hbox, FALSE, FALSE, 0);

	gtk_widget_show_all(hbox);

	g_object_set_data(G_OBJECT(gtkconv->toolbar), "conversation_locker_button", lock_button);
	g_object_set_data(G_OBJECT(gtkconv->toolbar), "conversation_locker_label", label);
	g_object_set_data(G_OBJECT(gtkconv->toolbar), "conversation_locker_icon", icon);

    toggle_widget_text(gtkconv);
}


static void
conversation_switched_cb(PurpleConversation *conv)
{
	PidginConversation *gtkconv = PIDGIN_CONVERSATION(conv);
	GtkItemFactory *factory = gtk_item_factory_from_path("<main>");
	gulong plugin_handler_found;
	gulong default_handler_id;
	GtkWidget *lock_button = NULL;

	lock_button = g_object_get_data(G_OBJECT(gtkconv->toolbar), "lock_button");

	if (lock_button == NULL)
		create_lock_button_pidgin(gtkconv);

	plugin_handler_found = g_signal_handler_find(gtkconv->win->window, G_SIGNAL_MATCH_FUNC,
								  		0, 0, NULL, stop_window_closing_cb, gtkconv);

	default_handler_id = (gulong)g_hash_table_lookup(hash_table, gtkconv->win->window);

	purple_debug_info(PLUGIN_ID, "\nswitched=%s found replaced handle=%lu default handler = %lu\n",
				    conv->title, plugin_handler_found, default_handler_id);

	/* enable/disable Main Window -> Close menu, depending on conversation lock status */
	if (purple_conversation_get_data(conv, "conversation_locker_status") == (gchar *)"locked")
		gtk_widget_set_sensitive((GtkWidget *)gtk_item_factory_get_widget(factory,
						     "<main>/Conversation/Close"), FALSE);
	else
		gtk_widget_set_sensitive((GtkWidget *)gtk_item_factory_get_widget(factory,
					          "<main>/Conversation/Close"), TRUE);

	/* We couldn't find plugin handler for newly detached tab which should be locked */
	if (purple_conversation_get_data(conv, "conversation_locker_status") == (gchar *)"locked" \
	    && plugin_handler_found == 0 && default_handler_id == 0) {
			toggle_conversation_lock(gtkconv, FALSE);
	}
}


static void
lock_all_conversations_action(PurplePluginAction *action)
{
	GList *convs;
	PurpleConversation *conv;
	PidginConversation *gtkconv;

	for (convs = purple_get_conversations(); convs != NULL; convs = convs->next) {
		conv = (PurpleConversation *)convs->data;
		gtkconv = PIDGIN_CONVERSATION(conv);

		if (!PIDGIN_IS_PIDGIN_CONVERSATION(conv))
			continue;

		if(purple_conversation_get_data(conv, "conversation_locker_status") != (gchar *)"locked")
			/* this unlocks all locked conversations and also takes relative actions */
			lock_button_cb(NULL, gtkconv);
	}

}

static void
unlock_all_conversations_action(PurplePluginAction *action)
{
	GList *convs;
	GList *gtkwins;
	PurpleConversation *conv;
	PidginConversation *gtkconv;

	for (convs = purple_get_conversations(); convs != NULL; convs = convs->next) {
		conv = convs->data;
		gtkconv = PIDGIN_CONVERSATION(conv);

		if (!PIDGIN_IS_PIDGIN_CONVERSATION(conv))
			continue;

		if(purple_conversation_get_data(conv, "conversation_locker_status") == (gchar *)"locked")
			/* this unlocks all locked conversations and also takes relative actions */
			lock_button_cb(NULL, gtkconv);
	}

	/* WORKAROUND : when locked tab is detached, the parent window is still locked,
	*			 even if it has zero locked conversations
	*/
	for (gtkwins = pidgin_conv_windows_get_list(); gtkwins != NULL; gtkwins = gtkwins->next) {
		PidginWindow *gtkwin = gtkwins->data;
		gulong handler_found;

		handler_found = g_signal_handler_find(gtkwin->window, G_SIGNAL_MATCH_FUNC, 0, 0,
                                    NULL, G_CALLBACK(stop_window_closing_cb), NULL);

		if (handler_found != 0)
		/* restore pidgins default "delete_signal" handler and dont emit signal*/
			close_window(gtkwin, FALSE);
	}

/* glib - since 2.12, might not compile on windows / with migw32 cross compilation
 * anyway, we don't need to purge table as after each unblocking the relative key is deleted
 * so at the end of unlocking all windows, hash table is already empty
 */
#ifndef _WIN32
	g_hash_table_remove_all(hash_table);
#endif
}


/* Create UI action list specific to plguin, displayed in "Tools" and tray*/
static GList *
plugin_actions(PurplePlugin *plugin, gpointer context)
{
	GList *list = NULL;
	PurplePluginAction *action = NULL;

	action = purple_plugin_action_new("Lock all conversations", lock_all_conversations_action);
	list = g_list_append(list, action);

	action = purple_plugin_action_new("Unlock all conversations", unlock_all_conversations_action);
	list = g_list_append(list, action);

	return list;
}


static void
remove_plugin_widgets(PidginConversation *gtkconv)
{
	GtkWidget *lock_button = g_object_get_data(G_OBJECT(gtkconv->toolbar),
									   "conversation_locker_button");

	if (lock_button != NULL) {
		gtk_widget_destroy(lock_button);

		g_object_set_data(G_OBJECT(gtkconv->toolbar), "conversation_locker_button", NULL);
		g_object_set_data(G_OBJECT(gtkconv->toolbar), "conversation_locker_label", NULL);
		g_object_set_data(G_OBJECT(gtkconv->toolbar), "conversation_locker_icon", NULL);
	}
}


/* cleanup the mess plugin has done */
static gboolean
plugin_unload(PurplePlugin *plugin)
{
	void *gtk_conv_handle = pidgin_conversations_get_handle();
	GList *convs = purple_get_conversations();

	for(; convs!= NULL; convs=convs->next) {
		PurpleConversation *conv = (PurpleConversation *)convs->data;

		if (PIDGIN_IS_PIDGIN_CONVERSATION(conv))
			remove_plugin_widgets(PIDGIN_CONVERSATION(conv));

	}

	/* unlock all conversations */
	unlock_all_conversations_action(NULL);

	g_hash_table_destroy(hash_table);

	purple_signal_disconnect(gtk_conv_handle, "conversation-switched", plugin_handle,
					  PURPLE_CALLBACK(conversation_switched_cb));

	g_hash_table_destroy(hash_table);

	/* it is shame that user unloaded plugin, but let plugin system know that we are just fine */
	return TRUE;
}


static gboolean
plugin_load(PurplePlugin *plugin)
{
	GList *convs = purple_get_conversations();	/* returns conversations list  */
	void *gtk_conv_handle = pidgin_conversations_get_handle();

	/* Emmited when conversations are switched.
	 * This is particularely of great interest because there is no other way to detect
	 * if tab is detached from main window. After tab is detached the purple data is copied
	 * into new tab. So if old tab was locked, we need to detect this and lock new window.
	 */
	purple_signal_connect(gtk_conv_handle, "conversation-switched", plugin,
					  PURPLE_CALLBACK(conversation_switched_cb), NULL);

	/* create lock button on all pidgin windows */
	while (convs){
		PurpleConversation *conv = (PurpleConversation *)convs->data;

		if (PIDGIN_IS_PIDGIN_CONVERSATION(conv)) {
			create_lock_button_pidgin(PIDGIN_CONVERSATION(conv));
		}
		convs = convs->next;
	}

	hash_table = g_hash_table_new(g_direct_hash, g_direct_equal);

	plugin_handle = plugin;

	return TRUE;	/* tell plugin system that we are stable as rock */
}

static void
toggle_widget_text(PidginConversation *gtkconv)
{
    GList *convs = purple_get_conversations();
    GtkWidget *label;
    gboolean visibility;

    visibility = purple_prefs_get_bool(PREFS_TEXT) ? TRUE: FALSE;

    if(gtkconv != NULL) {
        label = g_object_get_data(G_OBJECT(gtkconv->toolbar), "conversation_locker_label");
        if (visibility)
            gtk_widget_show(label);
        else
            gtk_widget_hide(label);

        return;
    }

    while (convs) {
        PurpleConversation *conv = (PurpleConversation *)convs->data;

		    if (PIDGIN_IS_PIDGIN_CONVERSATION(conv)) {
		        gtkconv = PIDGIN_CONVERSATION(conv);
				label = g_object_get_data(G_OBJECT(gtkconv->toolbar), "conversation_locker_label");
                if (visibility)
                    gtk_widget_show(label);
                else
                    gtk_widget_hide(label);
		    }

        convs = convs->next;
    }
}

static void
disconnect_prefs_cb(GtkObject *object, gpointer data)
{

   	PurplePlugin *plugin = (PurplePlugin *)data;
   	purple_prefs_disconnect_by_handle(plugin);

    toggle_widget_text(NULL);
}

static GtkWidget *
get_config_frame(PurplePlugin *plugin)
{
	GtkWidget *ret;
	GtkWidget *frame;

	ret = gtk_vbox_new(FALSE, PIDGIN_HIG_CAT_SPACE);
	gtk_container_set_border_width(GTK_CONTAINER(ret), PIDGIN_HIG_BORDER);

	g_signal_connect(GTK_OBJECT(ret), "destroy", G_CALLBACK(disconnect_prefs_cb), plugin);

	frame = pidgin_make_frame(ret, _("General"));
	pidgin_prefs_checkbox(_("Show \"Chat locked/unlocked\" text."), PREFS_TEXT, frame);

	pidgin_prefs_checkbox(_("Use Windows styled(Yes/No/Cancel) button order for \"Close locked conversations?\"prompt\n"
							"Non-windows style - No/Cancel/Yes)"), PREFS_BUTTON_ORDER, frame);

	gtk_widget_show_all(ret);
	return ret;
}

static PidginPluginUiInfo ui_info =
{
	get_config_frame,
	0,
	/* padding */
	NULL,
	NULL,
	NULL,
	NULL
};

/* Plugin Stuff */
static PurplePluginInfo info =
{
	PURPLE_PLUGIN_MAGIC,
	PURPLE_MAJOR_VERSION,                           /* major version */
	PURPLE_MINOR_VERSION,                           /* minor version */
	PURPLE_PLUGIN_STANDARD,                         /* type */
	PIDGIN_PLUGIN_TYPE,                             /* ui_requirement */
	0,                                              /* flags */
	NULL,                                           /* dependencies */
	PURPLE_PRIORITY_DEFAULT,                        /* priority */

	PLUGIN_ID,							            /* id */
	N_("Conversation Locker"),                      /* name */
	PLUGIN_VERSION,                                 /* version */
	N_("Restricts closing of important conversations"
	   "by locking(preventing from closing) them."),/* summary */
	N_("Shows lock/unlock button on each "
	"conversation window/tab with which important "
	"conversations can be locked(prevented from closing). "
	"Also shows warning if user tries to close window "
	"containing locked conversations."),		    /* description */
	PLUGIN_AUTHOR,						    	    /* author */
	PLUGIN_WEBSITE,                                 /* homepage */
	plugin_load,                                    /* load */
	plugin_unload,                                  /* unload */
	NULL,                                           /* destroy */
	&ui_info,                                       /* ui_info */
	NULL,                                           /* extra_info */
	NULL,                                           /* prefs_info */
	plugin_actions,                                 /* actions */

	/* padding */
	NULL,
	NULL,
	NULL,
	NULL
};


static void
init_plugin(PurplePlugin *plugin)
{
	purple_prefs_add_none(PREFS_PREFIX);

	purple_prefs_add_bool(PREFS_TEXT, TRUE);
	purple_prefs_add_bool(PREFS_ICON, TRUE);
	purple_prefs_add_bool(PREFS_BUTTON_ORDER, FALSE);
}

PURPLE_INIT_PLUGIN(conversation-locker, init_plugin, info)

