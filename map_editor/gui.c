#ifdef GTK2

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include "../platform.h"
#include "global.h"

#ifdef EYE_CANDY
#include "eye_candy_window.h"
#endif

char map_file_name[256]={0};
char particle_file_name[256]={0};

char map_folder[256];
char obj_2d_folder[256];
char obj_3d_folder[256];
char particles_folder[256];

char * selected_file = NULL;
GtkWidget * gtk_open_win = NULL;
GtkWidget * gtk_save_win = NULL;
GtkWidget * gtk_effect_win = NULL;
GtkWidget * gtk_effect_list_box = NULL;
GtkWidget * gtk_effect_hue_box = NULL;
GtkWidget * gtk_effect_saturation_box = NULL;
GtkWidget * gtk_effect_scale_box = NULL;
GtkWidget * gtk_effect_density_box = NULL;
GtkWidget * gtk_effect_base_height_box = NULL;
GtkWidget * gtk_effect_list = NULL;
GtkWidget * gtk_effect_hue = NULL;
GtkWidget * gtk_effect_saturation = NULL;
GtkWidget * gtk_effect_scale = NULL;
GtkWidget * gtk_effect_density = NULL;
GtkWidget * gtk_effect_base_height = NULL;
GtkObject * gtk_effect_hue_obj = NULL;
GtkObject * gtk_effect_saturation_obj = NULL;
GtkObject * gtk_effect_scale_obj = NULL;
GtkObject * gtk_effect_density_obj = NULL;
GtkFileFilter * e3d_filter = NULL;
GtkFileFilter * e2d_filter = NULL;
GtkFileFilter * map_filter = NULL;
GtkFileFilter * part_filter = NULL;

void init_filters()
{	
	e3d_filter=gtk_file_filter_new();
	gtk_file_filter_set_name(e3d_filter, "3D object");
	gtk_file_filter_add_pattern(e3d_filter, "*.e3d");
	gtk_file_filter_add_pattern(e3d_filter, "*.e3d.gz");
	
	e2d_filter=gtk_file_filter_new();
	gtk_file_filter_set_name(e2d_filter, "2D object");
	gtk_file_filter_add_pattern(e2d_filter, "*.2d0");
	
	map_filter=gtk_file_filter_new();
	gtk_file_filter_set_name(map_filter, "Map file");
	gtk_file_filter_add_pattern(map_filter, "*.gz");
	gtk_file_filter_add_pattern(map_filter, "*.elm");
	gtk_file_filter_add_pattern(map_filter, "*.elm.gz");
	
	part_filter=gtk_file_filter_new();
	gtk_file_filter_set_name(part_filter, "Particle file");
	gtk_file_filter_add_pattern(part_filter, "*.part");
	
	//Now set the correct dir names
	strcpy(obj_3d_folder, datadir);
	strcat(obj_3d_folder, "/3dobjects/");

	strcpy(obj_2d_folder, datadir);
	strcat(obj_2d_folder, "/2dobjects/");
	
	strcpy(map_folder, datadir);
	strcat(map_folder, "/maps/");
	
	strcpy(particles_folder, datadir);
	strcat(particles_folder, "/particles/");
}

void copy_folder(char * folder,  const char * file)
{
	int i;

	for (i = 0; file[i] != '\0'; i++)
		folder[i] = file[i];

	for ( ; file[i] != '/'; i--);

	folder[i] = '\0';
}

void open_button_clicked()
{
	GtkFileFilter * filter=gtk_file_chooser_get_filter(GTK_FILE_CHOOSER(gtk_open_win));
	selected_file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(gtk_open_win));
	if(selected_file){
		//What should we do next...
		if((point)filter==(point)map_filter){
			strcpy(map_file_name, selected_file);
			copy_folder(map_folder, selected_file);
			open_map_file_continued();
		} else if((point)filter==(point)e3d_filter){
			copy_folder(obj_3d_folder, selected_file);
			open_3d_obj_continued();
		} else if((point)filter==(point)e2d_filter){
			copy_folder(obj_2d_folder, selected_file);
			open_2d_obj_continued();
		} else if((point)filter==(point)part_filter){
			strcpy(particle_file_name, selected_file);
			copy_folder(particles_folder, selected_file);
			open_particles_obj_continued();
		}
		
		g_free(selected_file);
	}
	
	gtk_widget_hide(gtk_open_win);
}

void hide_open_win(GtkWidget * widget, GtkWidget * win)
{
	gtk_widget_hide(win);
}

void show_open_window(char * name, char * folder, GtkFileFilter * filter)
{
	if(!gtk_open_win) {
		GList *buttons;
		GtkWidget *cancel, *ok_button;
		
		gtk_open_win=gtk_file_chooser_dialog_new(name, NULL, GTK_FILE_CHOOSER_ACTION_OPEN, 
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, //Cancel button
				GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,//Open button
				NULL);
		
		buttons=((GtkHButtonBox*)(((GtkDialog*)gtk_open_win)->action_area))->button_box.box.children;
		cancel=((GtkBoxChild*)buttons->data)->widget;
		ok_button=((GtkBoxChild*)buttons->next->data)->widget;

		g_signal_connect ((gpointer) cancel, "clicked", G_CALLBACK (hide_open_win), gtk_open_win);
		g_signal_connect ((gpointer) ok_button, "clicked", G_CALLBACK (open_button_clicked), NULL);
		
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(gtk_open_win), e3d_filter);
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(gtk_open_win), e2d_filter);
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(gtk_open_win), map_filter);
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(gtk_open_win), part_filter);

	} else {
		gtk_window_set_title(GTK_WINDOW(gtk_open_win), name);
	}
	
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(gtk_open_win), filter);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(gtk_open_win), folder);
	
	gtk_widget_show(gtk_open_win);
}

void save_button_clicked(GtkWidget * widget, void ** filter)
{
	selected_file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(gtk_save_win));
	if(selected_file){
		//What should we do next...
		if((point)*filter==(point)map_filter){
			strcpy(map_file_name, selected_file);
			save_map_file_continued();
		} else if((point)*filter==(point)part_filter){
			strcpy(particle_file_name, selected_file);
			save_particle_def_file_continued();
		}

		gtk_file_chooser_unselect_filename(GTK_FILE_CHOOSER(gtk_save_win), selected_file);
		g_free(selected_file);
	}

	gtk_widget_hide(gtk_save_win);
}

void show_save_window(char * name, char * folder, char * select, GtkFileFilter * filter)
{
	static void * cur_filter;
	
	cur_filter=filter;
	
	if(!gtk_save_win) {
		GList *buttons;
		GtkWidget *cancel, *ok_button;
		
		gtk_save_win=gtk_file_chooser_dialog_new(name, NULL, GTK_FILE_CHOOSER_ACTION_SAVE,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, //Cancel button
			GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,//Open button
			NULL);
		
		buttons=((GtkHButtonBox*)(((GtkDialog*)gtk_save_win)->action_area))->button_box.box.children;
		cancel=((GtkBoxChild*)buttons->data)->widget;
		ok_button=((GtkBoxChild*)buttons->next->data)->widget;

		g_signal_connect ((gpointer) cancel, "clicked", G_CALLBACK (hide_open_win), gtk_save_win);
		g_signal_connect ((gpointer) ok_button, "clicked", G_CALLBACK (save_button_clicked), &cur_filter);
	} else {
		gtk_window_set_title(GTK_WINDOW(gtk_save_win), name);
	}
	
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(gtk_save_win), filter);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(gtk_save_win), folder);
	if(select[0])gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(gtk_save_win), select);
	else gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(gtk_save_win), "my_map.elm");

	gtk_widget_show(gtk_save_win);
}

void show_eye_candy_window()
{
#ifdef	EYE_CANDY
        eye_candy_confirmed = 0;
	if(!gtk_effect_win) {
		GtkWidget* label;
		gtk_effect_win=gtk_dialog_new_with_buttons("Select eye candy effect",
		                NULL,
		                GTK_DIALOG_DESTROY_WITH_PARENT, 
				GTK_STOCK_OK, //Cancel button
				GTK_RESPONSE_ACCEPT,//Open button
				NULL);

		g_signal_connect_swapped (gtk_effect_win,
				"response", 
				G_CALLBACK (confirm_eye_candy_effect),
				gtk_effect_win);

		gtk_effect_list = gtk_combo_box_new_text();
		gtk_combo_box_append_text(GTK_COMBO_BOX(gtk_effect_list), "Fire");
		gtk_combo_box_append_text(GTK_COMBO_BOX(gtk_effect_list), "Cloud/Fog");
		gtk_combo_box_append_text(GTK_COMBO_BOX(gtk_effect_list), "Fireflies");
		gtk_combo_box_append_text(GTK_COMBO_BOX(gtk_effect_list), "Fountain");
		gtk_combo_box_append_text(GTK_COMBO_BOX(gtk_effect_list), "Torch");
		gtk_combo_box_append_text(GTK_COMBO_BOX(gtk_effect_list), "Magic Protection");
		gtk_combo_box_append_text(GTK_COMBO_BOX(gtk_effect_list), "Shield");
		gtk_combo_box_append_text(GTK_COMBO_BOX(gtk_effect_list), "Magic Immunity");
		gtk_combo_box_append_text(GTK_COMBO_BOX(gtk_effect_list), "Poison");
		gtk_combo_box_append_text(GTK_COMBO_BOX(gtk_effect_list), "Smoke");
		gtk_combo_box_append_text(GTK_COMBO_BOX(gtk_effect_list), "Teleporter");
		gtk_combo_box_append_text(GTK_COMBO_BOX(gtk_effect_list), "Leaves");
		gtk_combo_box_append_text(GTK_COMBO_BOX(gtk_effect_list), "Flower Petals");
		gtk_combo_box_append_text(GTK_COMBO_BOX(gtk_effect_list), "Waterfall");
		gtk_combo_box_append_text(GTK_COMBO_BOX(gtk_effect_list), "Bees");
		gtk_combo_box_append_text(GTK_COMBO_BOX(gtk_effect_list), "Portal");
		gtk_combo_box_append_text(GTK_COMBO_BOX(gtk_effect_list), "Candle");
		g_signal_connect_swapped (gtk_effect_list,
				"changed", 
				G_CALLBACK (change_eye_candy_effect),
				gtk_effect_list);
		gtk_widget_show(gtk_effect_list);
		gtk_effect_list_box = gtk_hbox_new(FALSE, 10);
		gtk_box_pack_start(GTK_BOX(gtk_effect_list_box), gtk_effect_list, TRUE, TRUE, 0);
		label = gtk_label_new("Select your effect:");
		gtk_widget_show(label);
		gtk_box_pack_start(GTK_BOX(gtk_effect_list_box), label, TRUE, TRUE, 0);
		gtk_widget_show(gtk_effect_list_box);
		gtk_box_pack_start(GTK_BOX(GTK_DIALOG(gtk_effect_win)->vbox), gtk_effect_list_box, TRUE, TRUE, 0);

		gtk_effect_hue = gtk_hscale_new(GTK_ADJUSTMENT(gtk_effect_hue_obj = gtk_adjustment_new(0.0, 0.0, 1.01, 0.01, 0.01, 0.01)));
		g_signal_connect_swapped (gtk_effect_hue,
				"value_changed", 
				G_CALLBACK (change_eye_candy_effect),
				gtk_effect_list);
		gtk_scale_set_digits(GTK_SCALE(gtk_effect_hue), 2);
		gtk_widget_show(gtk_effect_hue);
		gtk_effect_hue_box = gtk_hbox_new(FALSE, 10);
		gtk_widget_set_size_request(gtk_effect_hue, 220, -1);
		label = gtk_label_new("Hue:");
		gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
		gtk_widget_set_size_request(label, 85, -1);
		gtk_widget_show(label);
		gtk_box_pack_start(GTK_BOX(gtk_effect_hue_box), label, TRUE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(gtk_effect_hue_box), gtk_effect_hue, TRUE, TRUE, 0);
		gtk_widget_hide(gtk_effect_hue_box);
		gtk_box_pack_start(GTK_BOX(GTK_DIALOG(gtk_effect_win)->vbox), gtk_effect_hue_box, TRUE, TRUE, 0);
		
		gtk_effect_saturation = gtk_hscale_new(GTK_ADJUSTMENT(gtk_effect_saturation_obj = gtk_adjustment_new(1.0, 0.0, 16.01, 0.01, 0.01, 0.01)));
		g_signal_connect_swapped (gtk_effect_saturation,
				"value_changed", 
				G_CALLBACK (change_eye_candy_effect),
				gtk_effect_list);
		gtk_scale_set_digits(GTK_SCALE(gtk_effect_saturation), 2);
		gtk_widget_show(gtk_effect_saturation);
		gtk_effect_saturation_box = gtk_hbox_new(FALSE, 10);
		gtk_widget_set_size_request(gtk_effect_saturation, 220, -1);
		label = gtk_label_new("Saturation:");
		gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
		gtk_widget_set_size_request(label, 85, -1);
		gtk_widget_show(label);
		gtk_box_pack_start(GTK_BOX(gtk_effect_saturation_box), label, TRUE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(gtk_effect_saturation_box), gtk_effect_saturation, TRUE, TRUE, 0);
		gtk_widget_hide(gtk_effect_saturation_box);
		gtk_box_pack_start(GTK_BOX(GTK_DIALOG(gtk_effect_win)->vbox), gtk_effect_saturation_box, TRUE, TRUE, 0);
		
		gtk_effect_scale = gtk_hscale_new(GTK_ADJUSTMENT(gtk_effect_scale_obj = gtk_adjustment_new(1.0, 0.01, 20.01, 0.02, 1.0, 0.01)));
		g_signal_connect_swapped (gtk_effect_scale,
				"value_changed", 
				G_CALLBACK (change_eye_candy_effect),
				gtk_effect_list);
		gtk_scale_set_digits(GTK_SCALE(gtk_effect_scale), 1);
		gtk_widget_show(gtk_effect_scale);
		gtk_effect_scale_box = gtk_hbox_new(FALSE, 10);
		gtk_widget_set_size_request(gtk_effect_scale, 220, -1);
		label = gtk_label_new("Scale:");
		gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
		gtk_widget_set_size_request(label, 85, -1);
		gtk_widget_show(label);
		gtk_box_pack_start(GTK_BOX(gtk_effect_scale_box), label, TRUE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(gtk_effect_scale_box), gtk_effect_scale, TRUE, TRUE, 0);
		gtk_widget_hide(gtk_effect_scale_box);
		gtk_box_pack_start(GTK_BOX(GTK_DIALOG(gtk_effect_win)->vbox), gtk_effect_scale_box, TRUE, TRUE, 0);
		
		gtk_effect_density = gtk_hscale_new(GTK_ADJUSTMENT(gtk_effect_density_obj = gtk_adjustment_new(1.0, 0.01, 20.01, 0.02, 1.0, 0.01)));
		g_signal_connect_swapped (gtk_effect_density,
				"value_changed", 
				G_CALLBACK (change_eye_candy_effect),
				gtk_effect_list);
		gtk_scale_set_digits(GTK_SCALE(gtk_effect_density), 1);
		gtk_widget_show(gtk_effect_density);
		gtk_effect_density_box = gtk_hbox_new(FALSE, 10);
		gtk_widget_set_size_request(gtk_effect_density, 220, -1);
		label = gtk_label_new("Density:");
		gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
		gtk_widget_set_size_request(label, 85, -1);
		gtk_widget_show(label);
		gtk_box_pack_start(GTK_BOX(gtk_effect_density_box), label, TRUE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(gtk_effect_density_box), gtk_effect_density, TRUE, TRUE, 0);
		gtk_widget_hide(gtk_effect_density_box);
		gtk_box_pack_start(GTK_BOX(GTK_DIALOG(gtk_effect_win)->vbox), gtk_effect_density_box, TRUE, TRUE, 0);
		
		gtk_effect_base_height = gtk_entry_new_with_max_length(30);
		g_signal_connect_swapped (gtk_effect_base_height,
				"changed", 
				G_CALLBACK (change_eye_candy_effect),
				gtk_effect_list);
		gtk_entry_set_text(GTK_ENTRY(gtk_effect_base_height), "0.0");
		gtk_widget_show(gtk_effect_base_height);
		gtk_effect_base_height_box = gtk_hbox_new(FALSE, 10);
		gtk_widget_set_size_request(gtk_effect_base_height, 220, -1);
		label = gtk_label_new("Base Height:");
		gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
		gtk_widget_set_size_request(label, 85, -1);
		gtk_widget_show(label);
		gtk_box_pack_start(GTK_BOX(gtk_effect_base_height_box), label, TRUE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(gtk_effect_base_height_box), gtk_effect_base_height, TRUE, TRUE, 0);
		gtk_widget_hide(gtk_effect_base_height_box);
		gtk_box_pack_start(GTK_BOX(GTK_DIALOG(gtk_effect_win)->vbox), gtk_effect_base_height_box, TRUE, TRUE, 0);
	} else {
		gtk_window_set_title(GTK_WINDOW(gtk_effect_win), "Select eye candy effect");
	}
	
	gtk_widget_show(gtk_effect_win);
	eye_candy_initialized = 1;
#endif	//EYE_CANDY
}

#else
/*
 * DO NOT EDIT THIS FILE - it is generated by Glade.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "gui_callbacks.h"
#include "gui.h"
#include "gui_support.h"

GtkWidget*
create_fileselection (void)
{
  GtkWidget *fileselection;
  GtkWidget *ok_button1;
  GtkWidget *cancel_button1;

  fileselection = gtk_file_selection_new ("Select File");
  gtk_object_set_data (GTK_OBJECT (fileselection), "fileselection", fileselection);
  gtk_container_set_border_width (GTK_CONTAINER (fileselection), 10);
  GTK_WINDOW (fileselection)->type = GTK_WINDOW_DIALOG;

  ok_button1 = GTK_FILE_SELECTION (fileselection)->ok_button;
  gtk_object_set_data (GTK_OBJECT (fileselection), "ok_button1", ok_button1);
  gtk_widget_show (ok_button1);
  GTK_WIDGET_SET_FLAGS (ok_button1, GTK_CAN_DEFAULT);

  cancel_button1 = GTK_FILE_SELECTION (fileselection)->cancel_button;
  gtk_object_set_data (GTK_OBJECT (fileselection), "cancel_button1", cancel_button1);
  gtk_widget_show (cancel_button1);
  GTK_WIDGET_SET_FLAGS (cancel_button1, GTK_CAN_DEFAULT);

  gtk_signal_connect (GTK_OBJECT (ok_button1), "clicked",
                      GTK_SIGNAL_FUNC (on_ok_button1_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (cancel_button1), "clicked",
                      GTK_SIGNAL_FUNC (on_cancel_button1_clicked),
                      NULL);

  gtk_file_selection_hide_fileop_buttons( GTK_FILE_SELECTION (fileselection));

  return fileselection;
}
#endif
