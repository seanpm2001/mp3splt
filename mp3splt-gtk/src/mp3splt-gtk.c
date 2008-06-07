/**********************************************************
 *
 * mp3splt-gtk -- utility based on mp3splt,
 *                for mp3/ogg splitting without decoding
 *
 * Copyright: (C) 2005-2006 Munteanu Alexandru
 * Contact: io_alex_2002@yahoo.fr
 *
 * http://mp3splt.sourceforge.net/
 *
 *********************************************************/

/**********************************************************
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 * USA.
 *
 *********************************************************/

/**********************************************************
 * Filename: mp3splt-gtk.c
 *
 * main file of the program that does the effective split of the
 * mp3/ogg and initialises gtk..
 *
 *********************************************************/

#include <signal.h>
#include <gtk/gtk.h>
#include <libmp3splt/mp3splt.h>
#include <locale.h>
#include <glib/gi18n.h>
#include <string.h>
#include <glib.h>

#include "util.h"
#include "special_split.h"
#include "player.h"
#include "utilities.h"
#include "tree_tab.h"
#include "main_win.h"
#include "snackamp_control.h"
#include "splitted_files.h"
#include "preferences_tab.h"

//the state
splt_state *the_state = NULL;

//the progress bar
GtkWidget *progress_bar;

extern GArray *splitpoints;
extern gint splitnumber;
extern GList *player_pref_list;
extern GtkWidget *queue_files_button;
extern gchar *filename_to_split;
extern gchar *filename_path_of_split;

//EXTERNAL OPTIONS
//frame mode option
extern GtkWidget *frame_mode;
//auto-adjust option
extern GtkWidget *adjust_mode;
//seekable option
extern GtkWidget *seekable_mode;
//gap parameter
extern GtkWidget *spinner_adjust_gap;
//offset parameter
extern GtkWidget *spinner_adjust_offset;
//threshold parameter
extern GtkWidget *spinner_adjust_threshold;

//silence mode parameters
//number of tracks parameter
extern GtkWidget *spinner_silence_number_tracks;
//number of tracks parameter
extern GtkWidget *spinner_silence_minimum;
//offset parameter
extern GtkWidget *spinner_silence_offset;
//threshold parameter
extern GtkWidget *spinner_silence_threshold;
//remove silence check button (silence mode parameter
extern GtkWidget *silence_remove_silence;

//spinner time
extern GtkWidget *spinner_time;
//the selected split mode
extern gint selected_split_mode;

//player
extern gint selected_player;

//if we are currently splitting
extern gint we_are_splitting;
//if we quit the main program while splitting
extern gint we_quit_main_program;
//the percent progress bar
extern GtkWidget *percent_progress_bar;

//stop button to cancel the split
extern GtkWidget *cancel_button;

//the output entry
extern GtkWidget *output_entry;

extern GtkWidget *split_button;

extern GtkWidget *remove_all_files_button;

//how many splitted files
gint splitted_files = 0;

//put the splitted file in the splitted_file tab
void put_splitted_filename(char *filename,int progress_data)
{
  //lock gtk
  gdk_threads_enter();
  
  if (!GTK_WIDGET_SENSITIVE(queue_files_button))
    {
      gtk_widget_set_sensitive(queue_files_button, TRUE);
    }
  if (!GTK_WIDGET_SENSITIVE(remove_all_files_button))
    {
      gtk_widget_set_sensitive(remove_all_files_button,TRUE);
    }
  
  add_splitted_row(filename);
  splitted_files++;
  
  //unlock gtk
  gdk_threads_leave();
}

//put the options from the preferences
void put_options_from_preferences()
{
  //preferences options
  //
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(frame_mode)))
    {
      mp3splt_set_int_option(the_state, SPLT_OPT_FRAME_MODE,
                             SPLT_TRUE);
    }
  else
    {
      mp3splt_set_int_option(the_state, SPLT_OPT_FRAME_MODE,
                             SPLT_FALSE);
    }
  //adjust option
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(adjust_mode)))
    {
      mp3splt_set_int_option(the_state, SPLT_OPT_AUTO_ADJUST,
                             SPLT_TRUE);
      //adjust spinners
      mp3splt_set_float_option(the_state, SPLT_OPT_PARAM_OFFSET,
                               gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(spinner_adjust_offset)));
      mp3splt_set_int_option(the_state, SPLT_OPT_PARAM_GAP,
                             gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinner_adjust_gap)));
      mp3splt_set_float_option(the_state, SPLT_OPT_PARAM_THRESHOLD,
                               gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(spinner_adjust_threshold)));
    }
  else
    {
      mp3splt_set_int_option(the_state, SPLT_OPT_AUTO_ADJUST,
                             SPLT_FALSE);
    }
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(seekable_mode)))
    {
      mp3splt_set_int_option(the_state, SPLT_OPT_INPUT_NOT_SEEKABLE,
                             SPLT_TRUE);
    }
  else
    {
      mp3splt_set_int_option(the_state, SPLT_OPT_INPUT_NOT_SEEKABLE,
                             SPLT_FALSE);
    }
  
  //we set default option;
  mp3splt_set_int_option(the_state, SPLT_OPT_SPLIT_MODE,
                         SPLT_OPTION_NORMAL_MODE);
  
  //we get the split modes
  switch (selected_split_mode)
    {
    case SELECTED_SPLIT_NORMAL:
      mp3splt_set_int_option(the_state, SPLT_OPT_SPLIT_MODE,
                             SPLT_OPTION_NORMAL_MODE);
      break;
    case SELECTED_SPLIT_WRAP:
      mp3splt_set_int_option(the_state, SPLT_OPT_SPLIT_MODE,
                             SPLT_OPTION_WRAP_MODE);
      break;
    case SELECTED_SPLIT_TIME:
      mp3splt_set_int_option(the_state, SPLT_OPT_SPLIT_MODE,
                             SPLT_OPTION_TIME_MODE);
      //we set the time option
      mp3splt_set_float_option(the_state, SPLT_OPT_SPLIT_TIME,
                               gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinner_time)));
      break;
    case SELECTED_SPLIT_SILENCE:
      mp3splt_set_int_option(the_state, SPLT_OPT_SPLIT_MODE,
                             SPLT_OPTION_SILENCE_MODE);
      //we set the silence parameters
      mp3splt_set_float_option(the_state, SPLT_OPT_PARAM_THRESHOLD,
                               gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(spinner_silence_threshold)));
      mp3splt_set_float_option(the_state, SPLT_OPT_PARAM_OFFSET,
                               gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(spinner_silence_offset)));
      mp3splt_set_int_option(the_state, SPLT_OPT_PARAM_NUMBER_TRACKS,
                             gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinner_silence_number_tracks)));
      mp3splt_set_float_option(the_state, SPLT_OPT_PARAM_MIN_LENGTH,
                               gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(spinner_silence_minimum)));
      if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(silence_remove_silence)))
        {
          mp3splt_set_int_option(the_state, SPLT_OPT_PARAM_REMOVE_SILENCE,
                                 SPLT_TRUE);
        }
      else
        {
          mp3splt_set_int_option(the_state, SPLT_OPT_PARAM_REMOVE_SILENCE,
                                 SPLT_FALSE);
        }
      break;
    case SELECTED_SPLIT_ERROR:
      mp3splt_set_int_option(the_state, SPLT_OPT_SPLIT_MODE,
                             SPLT_OPTION_ERROR_MODE);
      break;
    default:
      break;
    }
  
  //tag options
  //0 = No tags
  if (get_checked_tags_radio_box() == 0)
    {
      mp3splt_set_int_option(the_state, SPLT_OPT_TAGS,
                             SPLT_NO_TAGS);
    }
  else
    {
      if (get_checked_tags_radio_box() == 1)
        {
          mp3splt_set_int_option(the_state, SPLT_OPT_TAGS,
                                 SPLT_CURRENT_TAGS);
        }
      else
        {
          if (get_checked_tags_radio_box() == 2)
            {
              mp3splt_set_int_option(the_state, SPLT_OPT_TAGS,
                                     SPLT_TAGS_ORIGINAL_FILE);
            }
        }
    }
}

//changes the progress bar
void change_window_progress_bar(splt_progress *p_bar)
{
  gchar progress_text[1024] = " ";
  
  switch (p_bar->progress_type)
    {
    case SPLT_PROGRESS_PREPARE :
      g_snprintf(progress_text,1023,
                 _(" preparing \"%s\" (%d of %d)"),
                 p_bar->filename_shorted,
                 p_bar->current_split,
                 p_bar->max_splits);
      break;
    case SPLT_PROGRESS_CREATE :
      g_snprintf(progress_text,1023,
                 _(" creating \"%s\" (%d of %d)"),
                 p_bar->filename_shorted,
                 p_bar->current_split,
                 p_bar->max_splits);
      break;
    case SPLT_PROGRESS_SEARCH_SYNC :
      g_snprintf(progress_text,1023,
                 _(" searching for sync errors..."));
      break;
    case SPLT_PROGRESS_SCAN_SILENCE :
      g_snprintf(progress_text,2047,
          "S: %02d, Level: %.2f dB; scanning for silence...",
          p_bar->silence_found_tracks, p_bar->silence_db_level);
      break;
    default:
      g_snprintf(progress_text,1023, " ");
      break;
    }
      
  gchar printed_value[1024] = { '\0' };
  
  //lock gtk
  gdk_threads_enter();
      
  //we update the progress
  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(percent_progress_bar),
                                p_bar->percent_progress);
  g_snprintf(printed_value,1023,"%6.2f %% %s",
             p_bar->percent_progress * 100,
             progress_text);
  
  //we write the progress on the bar
  gtk_progress_bar_set_text(GTK_PROGRESS_BAR(percent_progress_bar),
                            printed_value);
      
  //unlock gtk
  gdk_threads_leave();
}

//effective split of the file
gpointer split_it(gpointer data)
{
  //lock gtk
  gdk_threads_enter();
  
  gint confirmation;
  
  gtk_widget_set_sensitive(GTK_WIDGET(split_button),
                           FALSE);
  
  //remove old splitted files
  remove_all_splitted_rows();  
  
  gint err = SPLT_OK;
  
  //erase previous splitpoints
  mp3splt_erase_all_splitpoints(the_state,&err);
  //we erase previous tags if we don't have the option
  //splt_current_tags
  if (mp3splt_get_int_option(the_state, SPLT_OPT_TAGS,&err)
      != SPLT_CURRENT_TAGS)
    {
      mp3splt_erase_all_tags(the_state,&err);
    }
  print_status_bar_confirmation(err);
  
  //we put the splitpoints in the state only if the normal mode
  if (mp3splt_get_int_option(the_state, SPLT_OPT_SPLIT_MODE,
                             &err) == SPLT_OPTION_NORMAL_MODE)
    {
      put_splitpoints_in_the_state(the_state);
    }
  
  //we put the output format
  gchar *format = strdup(gtk_entry_get_text(GTK_ENTRY(output_entry)));
  mp3splt_set_oformat(the_state, format, &err);
  free(format);
  
  //unlock gtk
  gdk_threads_leave();
  
  //put filename and path of split
  mp3splt_set_filename_to_split(the_state,filename_to_split);
  mp3splt_set_path_of_split(the_state,filename_path_of_split);
  
  //if we have the normal split mode, enable default output
  gint output_filenames = 
    mp3splt_get_int_option(the_state, SPLT_OPT_OUTPUT_FILENAMES,&err);
  if (mp3splt_get_int_option(the_state, SPLT_OPT_SPLIT_MODE,&err)
      == SPLT_OPTION_NORMAL_MODE)
    {
      mp3splt_set_int_option(the_state, SPLT_OPT_OUTPUT_FILENAMES,
                             SPLT_OUTPUT_CUSTOM);
    }
  
  //effective split, returns confirmation or error;
  confirmation = mp3splt_split(the_state);
  
  //reenable default output if necessary
  mp3splt_set_int_option(the_state, SPLT_OPT_OUTPUT_FILENAMES,
                         output_filenames);
  
  //lock gtk
  gdk_threads_enter();
  
  //we show infos about the splitted action
  print_status_bar_confirmation(confirmation);
  
  //see the cancel button
  gtk_widget_set_sensitive(GTK_WIDGET(cancel_button),
                           FALSE);
  
  we_are_splitting = FALSE;
  
  //we look if we have pushed the exit button
  if (we_quit_main_program)
    quit(NULL,NULL);
  
  if (confirmation >= 0)
    {
      //we have finished, put 100 to the progress bar
      //we update the progress
      gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(percent_progress_bar),
                                    1.0);
      //we write the progress on the bar
      gtk_progress_bar_set_text(GTK_PROGRESS_BAR(percent_progress_bar),
                                _(" finished "));
    }
  
  gtk_widget_set_sensitive(GTK_WIDGET(split_button),
                           TRUE);
  //unlock gtk
  gdk_threads_leave();
  
  return NULL;
}

//handler for the SIGPIPE signal
void sigpipe_handler(gint sig)
{
  if (player_is_running() &&
      selected_player == PLAYER_SNACKAMP)
    {
      disconnect_snackamp();
    }
}

gboolean sigint_called = FALSE;
//handler for the SIGINT signal
void sigint_handler(gint sig)
{
  if (!sigint_called)
    {
      sigint_called = TRUE;
      we_quit_main_program = TRUE;
      quit(NULL,NULL);
    }
}

//prints a message from the library
void put_message_from_library(gchar *message)
{
  gchar *mess = g_strdup(message);
  if (mess)
  {
    gint i = 0;
    //replace '\n' with ' '
    for (i = 0;i < strlen(mess);i++)
    {
      if (mess[i] == '\n')
      {
        mess[i] = ' ';
      }
    }
    gdk_threads_enter();
    put_status_message(mess);
    gdk_threads_leave();
    g_free(mess);
    mess = NULL;
  }
}

gint main (gint argc, gchar *argv[], gchar **envp)
{
  //init threads
  g_thread_init(NULL);
  gdk_threads_init();
  
  gint error = 0;
  
  //close nicely
  signal (SIGINT, sigint_handler);
  
#ifndef __WIN32__
  signal (SIGPIPE, sigpipe_handler);
#endif
  
  //used for gettext
  setlocale (LC_ALL, "");
  textdomain ("mp3splt-gtk");
  
#ifdef __WIN32__
  bindtextdomain ("mp3splt-gtk", "locale");
  bind_textdomain_codeset ("mp3splt-gtk", "UTF-8");
#else
  bindtextdomain ("mp3splt-gtk", LOCALEDIR);
#endif
  
  //create new state
  the_state = mp3splt_new_state(&error);
  
  gtk_init (&argc, &argv);
  
  //we initialise the splitpoints array
  splitpoints = g_array_new (FALSE, FALSE, sizeof (Split_point));
  
  //checks if preferences file exists
  //and if it does not, it creates it
  check_pref_file();
  
  //put the callback progress bar function
  mp3splt_set_progress_function(the_state,change_window_progress_bar);
  //put the callback function to receive the splitted file
  mp3splt_set_splitted_filename_function(the_state,put_splitted_filename);
  //put the callback function for miscellaneous messages
  mp3splt_set_message_function(the_state, put_message_from_library);
  //debug on or off
  mp3splt_set_int_option(the_state,SPLT_OPT_DEBUG_MODE,SPLT_FALSE);
  //main program
  create_all();
  error = mp3splt_find_plugins(the_state);
  if (error < 0)
  {
    print_status_bar_confirmation(error);
  }
  
  //loop
  gdk_threads_enter();
  gtk_main ();
  gdk_threads_leave();
  
  return 0;
}
