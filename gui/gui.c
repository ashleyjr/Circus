#include <gtk/gtk.h>

#define  APP_ID         "org.gtk.circus"
#define  APP_NAME       "Circus"
#define  AGENT_ID_START 1
#define  AGENT_ID_END   10
#define  AGENT_NUM      10

static void clear_agent_check_boxes (  GtkWidget   *button,  
                                       gpointer    *check   ){
   
   int         i;
   GtkWidget   *check_widget; 
   for(i=0;i<(AGENT_ID_END+1);i++){
      check_widget = (GtkWidget *)check;
      gtk_check_button_set_active
      *check_widget.set_active((gboolean)0);
      *check++;
   }
}

static void activate (  GtkApplication *app,
                        gpointer        user_data){
   int         i;
   char        str[32];
   GtkWidget   *window,
               *top_box,
               *agent_check_button_box,
               *agent_check_button[AGENT_NUM],
               *clear_button_box,
               *clear_button;

   window = gtk_application_window_new (app);
   gtk_window_set_title (GTK_WINDOW (window), APP_NAME);
   gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);
  
   // Top level box
   top_box = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
   gtk_container_add (GTK_CONTAINER (window), top_box); 

   // Add all the agent check buttons in a box
   agent_check_button_box = gtk_button_box_new (GTK_ORIENTATION_VERTICAL);
   gtk_container_add (GTK_CONTAINER (top_box), agent_check_button_box); 
   for(i=0;i<(AGENT_ID_END+1);i++){ 
      sprintf(str, "%d", i);
      agent_check_button[i] = gtk_check_button_new_with_label (str);
      gtk_container_add (GTK_CONTAINER (agent_check_button_box), agent_check_button[i]);
   }
  
   // Add clear check boxes button
   clear_button_box = gtk_button_box_new (GTK_ORIENTATION_VERTICAL);
   gtk_container_add (GTK_CONTAINER (top_box), clear_button_box); 
   clear_button = gtk_button_new_with_label ("Clear agents");
   g_signal_connect_swapped (clear_button, "clicked", G_CALLBACK (clear_agent_check_boxes), (gpointer)agent_check_button);
   gtk_container_add (GTK_CONTAINER (clear_button_box), clear_button);

   gtk_widget_show_all (window);
}

int main (  int    argc,
            char **argv){
   GtkApplication *app;
   int status;
   
   app = gtk_application_new (APP_ID, G_APPLICATION_FLAGS_NONE);
   g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
   status = g_application_run (G_APPLICATION (app), argc, argv);
   g_object_unref (app);
   
   return status;
}
