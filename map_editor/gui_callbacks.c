#ifndef GTK2
#include	"global.h"
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

//#include "gui_callbacks.h"
//#include "gui_support.h"

int continue_with;
GtkWidget* file_selector;

void
on_ok_button1_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
  selected_file = gtk_file_selection_get_filename (GTK_FILE_SELECTION(file_selector));
    gtk_widget_hide(file_selector);
    switch(continue_with)
      {
      case OPEN_3D_OBJ:
	open_3d_obj_continued();
	break;
      case OPEN_2D_OBJ:
	open_2d_obj_continued();
	break;
      case OPEN_MAP:
	open_map_file_continued();
	break;
      case SAVE_MAP:
	save_map_file_continued();
	break;
      case OPEN_PARTICLES_OBJ:
	open_particles_obj_continued();
	break;
      case SAVE_PARTICLE_DEF:
	save_particle_def_file_continued();
	break;
      case OPEN_EYE_CANDY_OBJ:
	open_eye_candy_obj_continued();
	break;
      }
}


void
on_cancel_button1_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
    gtk_widget_hide(file_selector);  
}
#endif
