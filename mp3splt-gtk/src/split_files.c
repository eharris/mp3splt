/**********************************************************
 *
 * mp3splt-gtk -- utility based on mp3splt,
 *                for mp3/ogg splitting without decoding
 *
 * Copyright: (C) 2005-2012 Alexandru Munteanu
 * Contact: io_fx@yahoo.fr
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

/*!********************************************************
 * \file 
 * The split tab
 *
 * file that handles the split files tab from the main 
 * window
 **********************************************************/

#include "split_files.h"

GtkTreeView *split_tree;

//!number of rows in the split table
gint split_table_number = 0;

//!handle box for detaching window
GtkWidget *split_handle_box;
//!queue files button
GtkWidget *queue_files_button;
//!remove file button
GtkWidget *remove_file_button;
//!remove file button
GtkWidget *remove_all_files_button;

extern gint selected_player;
extern gint split_files;

extern ui_state *ui;

//!split files enumeration
enum
{
  COL_NAME,
  COL_FILENAME,
  SPLIT_COLUMNS
};

//!creates the model for the split tree
static GtkTreeModel *create_split_model()
{
  GtkListStore *model = gtk_list_store_new(SPLIT_COLUMNS, G_TYPE_STRING, G_TYPE_STRING);
  return GTK_TREE_MODEL(model);
}

//!creates the tree
GtkTreeView *create_split_files_tree()
{
  return GTK_TREE_VIEW(gtk_tree_view_new_with_model(create_split_model()));
}

//!creates split columns
void create_split_columns(GtkTreeView *split_tree)
{
  GtkCellRendererText *renderer =
    GTK_CELL_RENDERER_TEXT(gtk_cell_renderer_text_new());
  g_object_set_data(G_OBJECT(renderer), "col", GINT_TO_POINTER(COL_NAME));
  GtkTreeViewColumn *name_column = gtk_tree_view_column_new_with_attributes 
    (_("Filename"), GTK_CELL_RENDERER(renderer), "text", COL_NAME, NULL);

  //appends columns to the list of columns of split_tree
  gtk_tree_view_insert_column(split_tree, GTK_TREE_VIEW_COLUMN(name_column),COL_NAME);

  gtk_tree_view_column_set_alignment(GTK_TREE_VIEW_COLUMN(name_column), 0.5);
  gtk_tree_view_column_set_sizing(GTK_TREE_VIEW_COLUMN(name_column),
      GTK_TREE_VIEW_COLUMN_AUTOSIZE);
  gtk_tree_view_column_set_resizable(name_column, TRUE);
}

//!removes all rows from the split files table
void remove_all_split_rows ()
{
  GtkTreeModel *model = gtk_tree_view_get_model(split_tree);
  while (split_table_number > 0)
  {
    GtkTreeIter iter;
    gtk_tree_model_get_iter_first(model, &iter);
    gtk_list_store_remove (GTK_LIST_STORE (model), &iter);

    split_table_number--;
  }
}

/*! finding the real name of the file, without the path
*/
const gchar *get_real_name_from_filename(const gchar *filename)
{
  const gchar *fname = filename;
  while (strchr(fname, G_DIR_SEPARATOR) != NULL)
  {
    fname = strchr(fname, G_DIR_SEPARATOR) + 1;
  }

  return fname;
}

//!add a row to the table
void add_split_row(const gchar *name)
{
  GtkTreeIter iter;
  GtkTreeModel *model = gtk_tree_view_get_model(split_tree);
  gtk_list_store_append(GTK_LIST_STORE(model), &iter);

  gtk_list_store_set(GTK_LIST_STORE(model), &iter,
      COL_NAME, get_real_name_from_filename(name),
      COL_FILENAME, name, -1);
  split_table_number++;
}

//!return the n_th filename from the split files
gchar *get_filename_from_split_files(gint number)
{
  gchar *filename = NULL;

  GtkTreeModel *model = gtk_tree_view_get_model(split_tree);
  GtkTreePath *path = gtk_tree_path_new_from_indices (number-1 ,-1);

  GtkTreeIter iter;
  if (gtk_tree_model_get_iter(model, &iter, path))
  {
    gtk_tree_model_get(model, &iter, COL_FILENAME, &filename, -1);
  }
  gtk_tree_path_free(path);

  return filename;
}

//!queue button event
void queue_files_button_event(GtkWidget *widget, gpointer data)
{
  if (!ui->status->timer_active)
  {
    connect_button_event(NULL,NULL);
  }

  GList *file_list = NULL;
  gchar *filename;

  GtkTreeModel *model = gtk_tree_view_get_model(split_tree);

  gint number = split_files;
  while (number >= 0)
  {
    GtkTreePath *path = gtk_tree_path_new_from_indices (number ,-1);

    GtkTreeIter iter;
    if (gtk_tree_model_get_iter(model, &iter, path))
    {
      gtk_tree_model_get(model, &iter, COL_FILENAME, &filename, -1);
      file_list = g_list_append(file_list, strdup(filename));
      g_free(filename);
    }
    gtk_tree_path_free(path);
    number--;
  }

  if (file_list != NULL)
  {
    player_add_files(file_list);
  }

  g_list_foreach(file_list, (GFunc)g_free, NULL);
  g_list_free(file_list);
}

//!event for the remove file button
void remove_file_button_event(GtkWidget *widget, gpointer data)
{
  GtkTreeModel *model = gtk_tree_view_get_model(split_tree);
  GtkTreeSelection *selection = gtk_tree_view_get_selection(split_tree);
  GList *selected_list = gtk_tree_selection_get_selected_rows(selection, &model);

  gchar *filename;

  while (g_list_length(selected_list) > 0)
  {
    GList *current_element = g_list_last(selected_list);
    GtkTreePath *path = current_element->data;

    GtkTreeIter iter;
    gtk_tree_model_get_iter(model, &iter, path);
    gtk_tree_model_get(model, &iter, COL_FILENAME, &filename, -1);
    g_remove(filename);

    //remove the path from the selected list
    gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
    selected_list = g_list_remove(selected_list, path);

    //remove 1 to the row number of the table
    split_table_number--;

    gtk_tree_path_free(path);
    g_free(filename);
  }

  if (split_table_number == 0)
  {
    gtk_widget_set_sensitive(queue_files_button, FALSE);
    gtk_widget_set_sensitive(remove_all_files_button, FALSE);
  }

  gtk_widget_set_sensitive(remove_file_button,FALSE);

  g_list_foreach(selected_list, (GFunc)gtk_tree_path_free, NULL);
  g_list_free(selected_list);
}

//!event for the remove file button
void remove_all_files_button_event(GtkWidget *widget, gpointer data)
{
  GtkTreeModel *model = gtk_tree_view_get_model(split_tree);

  gchar *filename;
  while (split_table_number > 0)
  {
    GtkTreeIter iter;
    gtk_tree_model_get_iter_first(model, &iter);
    gtk_tree_model_get(model, &iter, COL_FILENAME, &filename, -1);
    g_remove(filename);
    gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
    split_table_number--;
    g_free(filename);
  }

  gtk_widget_set_sensitive(remove_all_files_button,FALSE);
  gtk_widget_set_sensitive(remove_file_button,FALSE);
  gtk_widget_set_sensitive(queue_files_button,FALSE);
}

//!creates the horizontal queue buttons horizontal box
GtkWidget *create_queue_buttons_hbox()
{
  //our horizontal box
  GtkWidget *hbox = wh_hbox_new();

  //button for queueing all files
  queue_files_button =
    wh_create_cool_button(GTK_STOCK_UNINDENT, _("_Queue files to player"),FALSE);
  gtk_box_pack_start (GTK_BOX (hbox),
                      queue_files_button, FALSE, FALSE, 5);
  gtk_widget_set_sensitive(queue_files_button, FALSE);
  g_signal_connect (G_OBJECT (queue_files_button), "clicked",
                    G_CALLBACK (queue_files_button_event), NULL);
  
  //button for removing a file
  remove_file_button =
    wh_create_cool_button(GTK_STOCK_DELETE, _("_Delete selected files"),FALSE);
  gtk_box_pack_start (GTK_BOX (hbox),
                      remove_file_button, FALSE, FALSE, 5);
  gtk_widget_set_sensitive(remove_file_button,FALSE);
  g_signal_connect (G_OBJECT (remove_file_button), "clicked",
                    G_CALLBACK (remove_file_button_event), NULL);
  
  //button for removing a file
  remove_all_files_button =
    wh_create_cool_button(GTK_STOCK_DELETE, _("D_elete all files"),FALSE);
  gtk_box_pack_start (GTK_BOX (hbox),
                      remove_all_files_button, FALSE, FALSE, 5);
  gtk_widget_set_sensitive(remove_all_files_button,FALSE);
  g_signal_connect (G_OBJECT (remove_all_files_button), "clicked",
                    G_CALLBACK (remove_all_files_button_event), NULL);
  
  return hbox;
}

//! Issued when a row is clicked on
void split_tree_row_activated(GtkTreeView *split_tree, GtkTreePath *arg1,
    GtkTreeViewColumn *arg2, gpointer data)
{
  GtkTreeModel *model = gtk_tree_view_get_model(split_tree);
  GtkTreeSelection *selection = gtk_tree_view_get_selection(split_tree);
  GList *selected_list = gtk_tree_selection_get_selected_rows(selection, &model);

  GList *current_element = g_list_first(selected_list);
  GtkTreePath *path = current_element->data;

  GtkTreeIter iter;
  gtk_tree_model_get_iter(model, &iter, path);

  gchar *filename = NULL;

  gtk_tree_model_get(model, &iter, COL_FILENAME, &filename, -1);

  connect_button_event (NULL, NULL);
  change_current_filename(filename);
  connect_to_player_with_song(0, ui);

  gtk_tree_path_free(path);
  if (filename)
  {
    g_free(filename);
    filename = NULL;
  }
}

//!split selection has changed
void split_selection_changed(GtkTreeSelection *selec, gpointer data)
{
  GtkTreeModel *model = gtk_tree_view_get_model(split_tree);
  GtkTreeSelection *selection = gtk_tree_view_get_selection(split_tree);
  GList *selected_list = gtk_tree_selection_get_selected_rows(selection, &model);

  if (g_list_length(selected_list) > 0)
  {
    gtk_widget_set_sensitive(remove_file_button,TRUE);
  }
}

//!Issued when closing the new window after detaching
void close_split_popup_window_event(GtkWidget *window, gpointer data )
{
  GtkWidget *window_child;

  window_child = gtk_bin_get_child(GTK_BIN(window));

  gtk_widget_reparent(GTK_WIDGET(window_child), 
                      GTK_WIDGET(split_handle_box));

  gtk_widget_destroy(window);
}

//!Issued when we detach the handle
void handle_split_detached_event (GtkHandleBox *handlebox,
                                     GtkWidget *widget,
                                     gpointer data)
{
  //new window
  GtkWidget *window;

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  gtk_widget_reparent(GTK_WIDGET(widget), GTK_WIDGET(window));

  g_signal_connect (G_OBJECT (window), "delete_event",
                    G_CALLBACK (close_split_popup_window_event),
                    NULL);

  gtk_widget_show(GTK_WIDGET(window));
}

//!creates the split files tab
GtkWidget *create_split_files()
{
  //our vertical box
  GtkWidget *vbox = wh_vbox_new();

  /* handle box for detaching */
  split_handle_box = gtk_handle_box_new();
  gtk_container_add(GTK_CONTAINER (split_handle_box), 
                    GTK_WIDGET(vbox));
  //handle event
  g_signal_connect(split_handle_box, "child-detached",
                   G_CALLBACK(handle_split_detached_event),
                   NULL);

  // scrolled window and the tree 
  //create the tree and add it to the scrolled window
  split_tree = create_split_files_tree();
  //scrolled window for the tree
  GtkWidget *scrolled_window;
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(scrolled_window), GTK_SHADOW_NONE);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (vbox),
                      scrolled_window, TRUE, TRUE, 0);
  create_split_columns(split_tree);
  gtk_container_add(GTK_CONTAINER(scrolled_window), GTK_WIDGET(split_tree));
  g_signal_connect(G_OBJECT(split_tree), "row-activated", G_CALLBACK(split_tree_row_activated), NULL);
  
  //selection for the tree
  GtkWidget *split_tree_selection = 
    (GtkWidget *)gtk_tree_view_get_selection(split_tree);
  g_signal_connect(G_OBJECT(split_tree_selection), "changed", G_CALLBACK(split_selection_changed), NULL);
  gtk_tree_selection_set_mode(GTK_TREE_SELECTION(split_tree_selection), GTK_SELECTION_MULTIPLE);
  
  // horizontal box with queue buttons
  GtkWidget *queue_buttons_hbox;
  queue_buttons_hbox = 
    (GtkWidget *)create_queue_buttons_hbox();
  gtk_box_pack_start (GTK_BOX (vbox),
                      queue_buttons_hbox, FALSE, FALSE, 2);
  
  return split_handle_box;
}
