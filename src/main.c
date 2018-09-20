/* main.c
 *
 * Copyright 2018 VCSocial
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib/gi18n.h>

#include "gatekeeper-config.h"
#include "gatekeeper-window.h"

#include <unistd.h>

/* Parts of this code use snippets from Erik Wallstrom's Crypto program
 * help the author better understand gtk3 development. The file 'main.c'
 * was specifically used and can be found at:
 *
 * https://github.com/ErikWallstrom/Crypto/blob/master/main.c
 */

// GLOBAL VARS
static int is_fullscreen = 0;

static void
about_action (GSimpleAction *action, GVariant *param, gpointer app)
{
  (void) action;
  (void) param;

  gtk_show_about_dialog(
    app,
    "program-name", "Gatekeeper",
		"copyright", "Copyright © 2018 VCSocial",
		"authors", (char*[]){"VCSocial", NULL},
		"website", "https://github.com/VCSocial/Gatekeeper",
		"website_label", "git Repository",
		"version", "0.1 (pre-alpha)",
		"comments", "Easily manage permissions for flatpak applications",
		"license-type", GTK_LICENSE_AGPL_3_0,
    NULL
  );
}

static void
quit_action (GSimpleAction *action, GVariant *param, gpointer app)
{
	(void) action;
	(void) param;

	g_application_quit(G_APPLICATION(app));
}

static void
key_pressed (GtkWidget *widget, GdkEventKey *event)
{
  if (event->keyval == GDK_KEY_F11) {
    is_fullscreen = 1 - is_fullscreen;
    GtkWindow *window = GTK_WIDGET(widget);
    if (is_fullscreen)
      gtk_window_unfullscreen(window);
    else
      gtk_window_fullscreen(window);
  } // End if
}

static gboolean
search_key_press (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  GtkSearchBar *bar = GTK_SEARCH_BAR (user_data);
  return gtk_search_bar_handle_event (bar, event);
}

static void
register_icon (void)
{
  // Set a default icon
  GError *err = NULL;
  GdkPixbuf *icon = NULL;
  icon = gdk_pixbuf_new_from_file ("./Documents/Gatekeeper/data/lock.png"
                                   , &err);

  // Report error if found
  if (err != NULL) {
    fprintf (stderr, "Unable to read icon file: %s\n", err->message);
  } else {
    gtk_window_set_default_icon (icon);
  }
}

static void
on_startup (GtkApplication *app)
{
  GActionEntry action_options[] = {
		{"about", about_action, NULL, NULL, NULL, {0}},
    {"quit", quit_action, NULL, NULL, NULL, {0}}
	};

	g_action_map_add_action_entries(
		G_ACTION_MAP(app),
		action_options,
		sizeof(action_options) / sizeof(*action_options),
		app
  );

  GMenu* menu = g_menu_new();
	g_menu_append(menu, "About", "app.about");
  g_menu_append(menu, "Quit" , "app.quit");
  gtk_application_set_app_menu(app, G_MENU_MODEL(menu));

  const gchar *quit_accels[2] = {"<Ctrl>Q", NULL};

  gtk_application_set_accels_for_action (GTK_APPLICATION (app), "app.quit"
                                         , quit_accels);
}
static void
on_activate (GtkApplication *app)
{
	GtkWindow *window;
  GtkWidget *search_bar;
  GtkWidget *search_entry;
  GtkWidget *content_box;
	/* It's good practice to check your parameters at the beginning of the
	 * function. It helps catch errors early and in development instead of
	 * by your users.
	 */
	g_assert (GTK_IS_APPLICATION (app));

	/* Get the current window or create one if necessary. */
	window = gtk_application_get_active_window (app);
	if (window == NULL)
		window = g_object_new (GATEKEEPER_TYPE_WINDOW,
		                       "application", app,
		                       "default-width", 600,
		                       "default-height", 300,
		                       NULL);

  content_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 1);
  //gtk_container_add (GTK_WINDOW (window), content_box);



  register_icon ();
  search_bar = gtk_search_bar_new ();
  if (search_bar == NULL)
    fprintf (stderr, "Unable to create searchbar\n");

  search_entry = gtk_search_entry_new ();
  gtk_search_bar_connect_entry (search_bar, search_entry);


  // Checks for F11 input for fullscreen
  g_signal_connect(window, "key-press-event", G_CALLBACK(key_pressed), NULL);

  // Check for search event
  g_signal_connect(window, "key-press-event", G_CALLBACK (search_key_press)
                  , search_bar);

  /* Ask the window manager/compositor to present the window. */
	gtk_window_present (window);
}

int
main (int argc, char *argv[])
{
	g_autoptr(GtkApplication) app = NULL;
	int ret;

	/* Set up gettext translations */
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	/*
	 * Create a new GtkApplication. The application manages our main loop,
	 * application windows, integration with the window manager/compositor, and
	 * desktop features such as file opening and single-instance applications.
	 */
	app = gtk_application_new ("org.gnome.Gatekeeper", G_APPLICATION_FLAGS_NONE);

	/*
	 * We connect to the activate signal to create a window when the application
	 * has been lauched. Additionally, this signal notifies us when the user
	 * tries to launch a "second instance" of the application. When they try
	 * to do that, we'll just present any existing window.
	 *
	 * Because we can't pass a pointer to any function type, we have to cast
	 * our "on_activate" function to a GCallback.
	 */
	g_signal_connect (app, "startup" , G_CALLBACK (on_startup) , NULL);
  g_signal_connect (app, "activate", G_CALLBACK (on_activate), NULL);

	/*
	 * Run the application. This function will block until the applicaiton
	 * exits. Upon return, we have our exit code to return to the shell. (This
	 * is the code you see when you do `echo $?` after running a command in a
	 * terminal.
	 *
	 * Since GtkApplication inherits from GApplication, we use the parent class
	 * method "run". But we need to cast, which is what the "G_APPLICATION()"
	 * macro does.
	 */
	ret = g_application_run (G_APPLICATION (app), argc, argv);

	return ret;
}
