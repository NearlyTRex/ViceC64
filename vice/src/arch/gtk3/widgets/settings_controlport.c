/** \file   settings_controlport.c
 * \brief   Widget to control settings for control ports
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES JoyPort1Device  x64 x64sc x64dtv xscpu64 x128 xcbm5x0 xplus4 xvic
 * $VICERES JoyPort2Device  x64 x64sc x64dtv xscpu64 x128 xcbm5x0 xplus4
 * $VICERES JoyPort3Device  x64 x64sc x64dtv xscpu64 x128 xcbm2 xvic
 * $VICERES JoyPort4Device  x64 x64sc xscpu64 x128 xcbm2 xpet xvice
 * $VICERES JoyPort5Device  xplus4
 * $VICERES BBRTCSave
 * $VICERES ps2mouse            x64dtv
 * $VICERES SmartMouseRTCSave   x64 x64sc xscpu64 x128 xvic xplus4 xcbm5x0
 * $VICERES UserportJoy     -xcbm5x0 -vsid
 */

/*
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "vice.h"

#include <gtk/gtk.h>
#include <stdlib.h>

#include "vice_gtk3.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "resources.h"
#include "joyport.h"
#include "uisettings.h"

#include "settings_controlport.h"


/*
 * Forward declarations
 */

static void joyport_devices_list_shutdown(void);
static void free_combo_list(int port);



/** \brief  Lists of valid devices for each joyport
 */
static joyport_desc_t *joyport_devices[JOYPORT_MAX_PORTS];


/** \brief  Combo box entry lists for each joyport
 */
static vice_gtk3_combo_entry_int_t *joyport_combo_lists[JOYPORT_MAX_PORTS];


/** \brief  Reference to userport widget */
static GtkWidget *userportjoy_widget = NULL;


/** \brief  Handler for the "destroy" event of the main widget
 *
 * \param[in]   widget      the main widget (unused)
 * \param[in]   user_data   extra data (unused)
 */
static void on_destroy(GtkWidget *widget, gpointer user_data)
{
    int port;

    joyport_devices_list_shutdown();
    for (port = 0; port < JOYPORT_MAX_PORTS; port++) {
        free_combo_list(port);
    }
}


/** \brief  Create a check button to enable "Userport joystick adapter"
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_userportjoy_enable_checkbox(void)
{
    GtkWidget *check;

    check = vice_gtk3_resource_check_button_new("UserportJoy",
            "Enable userport joysticks");
    return check;
}


/** \brief  Dynamically generate a list of joyport devices for \a port
 *
 * \param[in]   port    port number
 *
 * \return  TRUE if the list was generated
 */
static gboolean create_combo_list(int port)
{
    int num;
    int i;
    joyport_desc_t *dev;

    dev = joyport_devices[port];
    if (dev == NULL) {
        joyport_combo_lists[port] = NULL;
        return FALSE;
    }

    /* calculate size of list to create */
    num = 0;
    while (dev->name != NULL) {
        dev++;
        num++;
    }

    /* allocate memory for list */
    joyport_combo_lists[port] = lib_malloc((size_t)(num + 1) *
            sizeof *joyport_combo_lists[port]);

    /* populate list */
    i = 0;
    dev = joyport_devices[port];
    while (dev->name != NULL) {
        joyport_combo_lists[port][i].name = dev->name;
        joyport_combo_lists[port][i].id = dev->id;
        dev++;
        i++;
    }
    /* terminate list */
    joyport_combo_lists[port][i].name = NULL;
    joyport_combo_lists[port][i].id = -1;
    return TRUE;
}


/** \brief  Free memory used by the combo box entry list for \a port
 *
 * \param[in]   port    index in the combo box lists (0 == JoyPort1Device)
 */
static void free_combo_list(int port)
{
    if (joyport_combo_lists[port] != NULL) {
        lib_free(joyport_combo_lists[port]);
    }
}


/** \brief  Create combo box for joyport \a port
 *
 * \param[in]   port    Joyport number (0-4, 0 == JoyPort1Device)
 * \param[in]   title   widget title
 *
 * \return  GtkGrid
 */
static GtkWidget *create_joyport_widget(int port, const char *title)
{
    GtkWidget *grid;
    GtkWidget *combo;

    /* generate combo box list */
    if (!create_combo_list(port)) {
        log_error(LOG_ERR,
                "failed to generate joyport devices list for port %d",
                port + 1);
        return NULL;
    }

    grid = vice_gtk3_grid_new_spaced_with_label(-1, -1, title, 1);

    combo = vice_gtk3_resource_combo_box_int_new_sprintf(
            "JoyPort%dDevice",
            joyport_combo_lists[port],
            port + 1);
    g_object_set(combo, "margin-left", 16, NULL);
    gtk_widget_set_hexpand(combo, TRUE);

    gtk_grid_attach(GTK_GRID(grid), combo, 0, 1, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create checkbox for the battery-backed RTC save option
 *
 * \return  GtkCheckButton
 */
static void *create_bbrtc_widget(void)
{
    GtkWidget *check;

    check = vice_gtk3_resource_check_button_new("BBRTCSave",
            "Save battery-backed real time clock data when changed");
    g_object_set(check, "margin-left", 16, NULL);
    return check;
}


/** \brief  Retrieve valid devices for each joyport
 *
 * joyport_get_valid_devices() returns an empty list for unsupported devices,
 * so no need to check for machine type.
 */
static void joyport_devices_list_init(void)
{
    int i;

    for (i = 0; i < JOYPORT_MAX_PORTS; i++) {
        joyport_devices[i] = joyport_get_valid_devices(i);
    }
}


/** \brief  Clean up memory used by the valid devices list
 */
static void joyport_devices_list_shutdown(void)
{
    int i;

    for (i = 0; i < JOYPORT_MAX_PORTS; i++) {
        if (joyport_devices[i] != NULL) {
            lib_free(joyport_devices[i]);
            joyport_devices[i] = NULL;
        }
    }
}


/** \brief  Create layout for x64, x64sc, x64dtv, xscpu64, xcbm5x0 and x128
 *
 * Two control ports and eight joystick adapter ports.
 *
 * \param[in,out]   grid    main widget grid
 *
 * \return  number of rows used in the \a grid
 */
static int create_c64_layout(GtkGrid *grid)
{
    gtk_grid_attach(grid,
            create_joyport_widget(
                JOYPORT_1,
                "Control port 1"),
            0, 0, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(
                JOYPORT_2,
                "Control port 2"),
            1, 0, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(
                JOYPORT_3,
                "Joystick adapter port 1"),
            0, 1, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(
                JOYPORT_4,
                "Joystick adapter port 2"),
            1, 1, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(
                JOYPORT_5,
                "Joystick adapter port 3"),
            0, 2, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(
                JOYPORT_6,
                "Joystick adapter port 4"),
            1, 2, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(
                JOYPORT_7,
                "Joystick adapter port 5"),
            0, 3, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(
                JOYPORT_8,
                "Joystick adapter port 6"),
            1, 3, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(
                JOYPORT_9,
                "Joystick adapter port 7"),
            0, 4, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(
                JOYPORT_10,
                "Joystick adapter port 8"),
            1, 4, 1, 1);
    return 5;
}


/** \brief  Create layout for xvic
 *
 * One control port and eight joystick adapter ports.
 *
 * \param[in,out]   grid    main widget grid
 *
 * \return  number of rows used in the \a grid
 */
static int create_vic20_layout(GtkGrid *grid)
{
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_1, "Control port"),
            0, 0, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_3,
                "Joystick adapter port 1"),
            0, 1, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_4,
                "Joystick adapter port 2"),
            1, 1, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_5,
                "Joystick adapter port 3"),
            0, 2, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_6,
                "Joystick adapter port 4"),
            1, 2, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_7,
                "Joystick adapter port 5"),
            0, 3, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_8,
                "Joystick adapter port 6"),
            1, 3, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_9,
                "Joystick adapter port 7"),
            0, 4, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_10,
                "Joystick adapter port 8"),
            1, 4, 1, 1);
    return 5;
}


/** \brief  Create layout for xplus4
 *
 * Two control ports, two userport adapter ports and one SIDCard control port.
 *
 * \param[in,out]   grid    main widget grid
 *
 * \return  number of rows used in the \a grid
 */
static int create_plus4_layout(GtkGrid *grid)
{
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_1, "Control port 1"),
            0, 0, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_2, "Control port 2"),
            1, 0, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_3,
                "Userport joystick adapter port 1"),
            0, 1, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_4,
                "Userport joystick adapter port 2"),
            1, 1, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_5, "SIDCard control port"),
            0, 2, 1, 1);
    return 3;
}


/** \brief  Create layout for xpet
 *
 * Two userport adapter ports.
 *
 * \param[in,out]   grid    main widget grid
 *
 * \return  number of rows used in the \a grid
 */
static int create_pet_layout(GtkGrid *grid)
{
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_3,
                "Userport joystick adapter port 1"),
            0, 0, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_4,
                "Userport joystick adapter port 2"),
            1, 0, 1, 1);
    return 1;
}



/** \brief  Create layout for xcbm2
 *
 * Eight joystick adapter ports.
 *
 * \param[in,out]   grid    main widget grid
 *
 * \return  number of rows used in the \a grid
 */
static int create_cbm6x0_layout(GtkGrid *grid)
{
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_3,
                "Joystick adapter port 1"),
            0, 0, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_4,
                "Joystick adapter port 2"),
            1, 0, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_5,
                "Joystick adapter port 3"),
            0, 1, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_6,
                "Joystick adapter port 4"),
            1, 1, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_7,
                "Joystick adapter port 5"),
            0, 2, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_8,
                "Joystick adapter port 6"),
            1, 2, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_9,
                "Joystick adapter port 7"),
            0, 3, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_10,
                "Joystick adapter port 8"),
            1, 3, 1, 1);
    return 4;
}



/** \brief  Create widget to control control ports
 *
 * Creates a widget to control the settings for the control ports, userport
 * joystick adapter ports and the SIDCard control port, depending on the
 * currently emulated machine.
 *
 * \param[in]   parent  parent widget
 *
 * \return  GtkGrid
 */
GtkWidget *settings_controlport_widget_create(GtkWidget *parent)
{
    GtkWidget *layout;
    GtkWidget *mouse_save;
    GtkWidget *ps2_enable;
    int rows = 0;

    joyport_devices_list_init();

    layout = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(layout), 8);
    gtk_grid_set_row_spacing(GTK_GRID(layout), 8);

    switch (machine_class) {
        case VICE_MACHINE_C64:      /* fall through */
        case VICE_MACHINE_C64SC:    /* fall through */
        case VICE_MACHINE_SCPU64:   /* fall through */
        case VICE_MACHINE_C128:     /* fall through */
        case VICE_MACHINE_C64DTV:   /* fall through */
        case VICE_MACHINE_CBM5x0:
            rows = create_c64_layout(GTK_GRID(layout));
            break;
        case VICE_MACHINE_VIC20:
            rows = create_vic20_layout(GTK_GRID(layout));
            break;
        case VICE_MACHINE_PLUS4:
            rows = create_plus4_layout(GTK_GRID(layout));
            break;
        case VICE_MACHINE_PET:
            rows = create_pet_layout(GTK_GRID(layout));
            break;
        case VICE_MACHINE_CBM6x0:
            rows = create_cbm6x0_layout(GTK_GRID(layout));
            break;
        case VICE_MACHINE_VSID:
            break;  /* no control ports or joystick adapter ports */
        default:
            break;
    }

    /* add BBRTC checkbox */
    if (rows > 0) {
        GtkWidget *bbrtc_widget = create_bbrtc_widget();
        gtk_grid_attach(GTK_GRID(layout), bbrtc_widget, 0, rows, 2, 1);
        rows++;
    }

    /* add SmartMouseRTCSave checkbox */
    switch (machine_class) {
        case VICE_MACHINE_C64:      /* fall through */
        case VICE_MACHINE_C64SC:    /* fall through */
        case VICE_MACHINE_SCPU64:   /* fall through */
        case VICE_MACHINE_C128:     /* fall through */
        case VICE_MACHINE_VIC20:    /* fall through */
        case VICE_MACHINE_PLUS4:    /* fall through */
        case VICE_MACHINE_CBM5x0:
            mouse_save = vice_gtk3_resource_check_button_new(
                    "SmartMouseRTCSave", "Enable SmartMouse RTC Saving");
            gtk_grid_attach(GTK_GRID(layout), mouse_save, 0, rows, 2, 1);
            g_object_set(mouse_save, "margin-left", 16, NULL);
            rows++;
            break;
        default:
            /* No SmartMouse support */
            break;
    }

    /* PS/2 mouse on DTV */
    if (machine_class == VICE_MACHINE_C64DTV) {
        ps2_enable = vice_gtk3_resource_check_button_new("ps2mouse",
                "Enable PS/2 mouse on Userport");
        gtk_grid_attach(GTK_GRID(layout), ps2_enable, 0, rows, 2, 1);
        g_object_set(ps2_enable, "margin-left", 16, NULL);
        rows++;
    }

    if (machine_class != VICE_MACHINE_CBM5x0) {
        userportjoy_widget = create_userportjoy_enable_checkbox();
        g_object_set(userportjoy_widget, "margin-left", 16, NULL);
        gtk_grid_attach(GTK_GRID(layout), userportjoy_widget, 0, rows, 2, 1);
    }

    g_signal_connect_unlocked(layout, "destroy", G_CALLBACK(on_destroy), NULL);
    gtk_widget_show_all(layout);
    return layout;
}
