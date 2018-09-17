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

/* Parts of this code use snippets from Erik Wallstrom's Crypto program
 * help the author better understand gtk3 development. The file 'main.c'
 * was specifically used and can be found at:
 *
 * https://github.com/ErikWallstrom/Crypto/blob/master/main.c
 */

// GLOBAL VARS
static int is_fullscreen = 0;

static void
about_action (GSimpleAction *action, GVariant *param, void *userdata)
{
  (void) action;
  (void) param;

  GtkWindow* window = gtk_application_get_active_window(
		GTK_APPLICATION(userdata)
  );

  gtk_show_about_dialog(
    window,
    "program-name", "Gatekeeper",
		"copyright", "Copyright © 2018 VCSocial",
		"authors", (char*[]){"VCSocial", NULL},
		"website", "",
		"website_label", "Repository",
		"version", "0.1 (pre-alpha)",
		"comments", "Easily manage permissions for flatpak applications",
		"license-type", GTK_LICENSE_AGPL_3_0,
		"logo", gdk_pixbuf_new_from_file("../data/2000px-Lock_font_awesome.png", NULL),
    NULL
  );
}

static void
quit_action (GSimpleAction *action, GVariant *param, void *userdata)
{
	(void) action;
	(void) param;

	GtkWindow* window = gtk_application_get_active_window(
		GTK_APPLICATION(userdata)
	);

	gtk_widget_destroy(GTK_WIDGET(window));
	g_application_quit(G_APPLICATION(userdata));
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

static void
on_activate (GtkApplication *app)
{
	GtkWindow *window;

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

  // Checks for F11 input for fullscreen
  g_signal_connect(window, "key-press-event", G_CALLBACK(key_pressed), NULL);

	GtkWidget* stack = gtk_stack_new();
	  gtk_stack_set_transition_duration(GTK_STACK(stack), 800);
	  gtk_stack_set_transition_type(
		  GTK_STACK(stack),
		  GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT
  );
  /* Ask the window manager/compositor to present the window. */
	gtk_window_present (window);
}

int
main (int   argc,
      char *argv[])
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
