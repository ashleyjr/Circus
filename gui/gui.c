/* Include */
#include <gtk/gtk.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

/* Define */
#define  BAUDRATE       B115200 
#define  PORTNAME       "/dev/ttyUSB0"
#define  APP_ID         "org.gtk.circus"
#define  APP_NAME       "Circus"
#define  AGENT_ID_START 1
#define  AGENT_ID_END   10
#define  AGENT_NUM      10
#define  BUFFER_TIME_S  0.01
#define  BUFFER_TX_SIZE 128

/* Typedefs */
typedef  unsigned char   uint8_t;
typedef  unsigned short  uint16_t;

/* Globals */
static   volatile int               fd;
static   volatile uint8_t           tx_buffer[BUFFER_TX_SIZE];
static   volatile uint16_t          tx_buffer_head,
                                    tx_buffer_tail;
/* Open a port to the UART */
int openport (    int fd, 
                  int speed, 
                  int parity
){
   struct termios tty;
   memset (&tty, 0, sizeof tty);
   if (tcgetattr (fd, &tty) != 0){ 
      return -1;
   }
   cfsetospeed (&tty, speed);
   cfsetispeed (&tty, speed);
   tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
   //disable IGNBRK for mismatched speed tests; otherwise receive break
   // as \000 chars
   tty.c_iflag &= ~IGNBRK;                         // disable break processing
   tty.c_lflag = 0;                                // no signaling chars, no echo,
   // no canonical processing
   tty.c_oflag = 0;                                // no remapping, no delays
   tty.c_cc[VMIN]  = 0;                            // read doesn't block
   tty.c_cc[VTIME] = 5;                            // 0.5 seconds read timeout 
   tty.c_iflag &= ~(IXON | IXOFF | IXANY);         // shut off xon/xoff ctrl
   tty.c_cflag |= (CLOCAL | CREAD);                // ignore modem controls,
   // enable reading
   tty.c_cflag &= ~(PARENB | PARODD);              // shut off parity
   tty.c_cflag |= parity;
   tty.c_cflag &= ~CSTOPB;
   tty.c_cflag &= ~CRTSCTS; 
   if (tcsetattr (fd, TCSANOW, &tty) != 0){ 
      return -1;
   }
   return 0;
}
/* Timer function called to unload the buffer
   and transmit data */
static gboolean unload_tx_buffer( gpointer textview  
){
   char str[2];
   
   while(tx_buffer_head != tx_buffer_tail){ 
      sprintf(str, "%c", tx_buffer[tx_buffer_tail]);
      g_print(str);
      write(fd, str, 2);
      tx_buffer_tail++; 
      tx_buffer_tail %= BUFFER_TX_SIZE;
   } 
   return TRUE;
}

/* Called by user and loads the buffer */
static void load_tx_buffer(   uint8_t data 
){
   tx_buffer[tx_buffer_head] = data;
   tx_buffer_head++;
   tx_buffer_head %= BUFFER_TX_SIZE;
}

/* Clear a toggle box callback*/
static void clear_agent_toggle_box_cb (  GtkButton   *button,  
                                          gpointer    check   
){ 
   gtk_toggle_button_set_active((GtkToggleButton *)check, FALSE); 
}

/* Set a toggle box callback*/
static void set_agent_toggle_box_cb (  GtkButton   *button,  
                                       gpointer    check   
){ 
   gtk_toggle_button_set_active((GtkToggleButton *)check, TRUE); 
}

/* Key press callback for all active keys */
static void  keypress_cb (GtkWidget *widget, GdkEventKey *event, gpointer data) {
   switch (event->keyval){
      case GDK_KEY_Left:   load_tx_buffer('A');                           
                           break; 
   }      
}

/* A widgets and main functions */
static void activate (  GtkApplication *app,
                        gpointer        user_data
){
   int         i;
   char        str[32];
   GtkWidget   *window,
               *top_box,
               *agent_toggle_button_box,
               *agent_toggle_button[AGENT_NUM],
               *all_button_box,
               *all_clear_button,
               *all_set_button,
               *textview_box,
               *textview;

   window = gtk_application_window_new (app);
   gtk_window_set_title (GTK_WINDOW (window), APP_NAME);
   gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);
  
   // Top level box
   top_box = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
   gtk_container_add (GTK_CONTAINER (window), top_box); 

   // Add all the agent toggle buttons in a box
   agent_toggle_button_box = gtk_button_box_new (GTK_ORIENTATION_VERTICAL);
   gtk_container_add (GTK_CONTAINER (top_box), agent_toggle_button_box); 
   for(i=0;i<(AGENT_ID_END+1);i++){ 
      sprintf(str, "%d", i);
      agent_toggle_button[i] = gtk_toggle_button_new_with_label (str);
      gtk_container_add (GTK_CONTAINER (agent_toggle_button_box), agent_toggle_button[i]);
   }
  
   // Add set/clear all toggle buttons box
   all_button_box = gtk_button_box_new (GTK_ORIENTATION_VERTICAL);
   gtk_container_add (GTK_CONTAINER (top_box), all_button_box); 
   all_clear_button = gtk_button_new_with_label ("Clear all agents");
   all_set_button = gtk_button_new_with_label ("Set all agents");
   for(i=0;i<(AGENT_ID_END+1);i++){
      g_signal_connect(all_clear_button, "clicked", G_CALLBACK (clear_agent_toggle_box_cb), agent_toggle_button[i]);
      g_signal_connect(all_set_button, "clicked", G_CALLBACK (set_agent_toggle_box_cb), agent_toggle_button[i]);
   }
   gtk_container_add (GTK_CONTAINER (all_button_box), all_clear_button);
   gtk_container_add (GTK_CONTAINER (all_button_box), all_set_button);

   // Add the text view  
   textview_box = gtk_button_box_new (GTK_ORIENTATION_VERTICAL);
   gtk_container_add (GTK_CONTAINER (top_box), textview_box); 
   textview = gtk_text_view_new ();
   gtk_container_add (GTK_CONTAINER (textview_box), textview);

   // Key press events
   gtk_widget_add_events(window, GDK_KEY_PRESS_MASK);
   g_signal_connect (G_OBJECT (window), "key_press_event", G_CALLBACK (keypress_cb), NULL);

   // Connect timer for transmit
   g_timeout_add_seconds(BUFFER_TIME_S, unload_tx_buffer, textview);

   // Added
   gtk_widget_show_all (window);
}

int main (  int    argc,
            char **argv
){
   GtkApplication *app;
   int status;
   
   // Init buffer
   tx_buffer_head = 0;
   tx_buffer_tail = 0;

   // Connect UART
   fd = open (PORTNAME, O_RDWR | O_NOCTTY | O_SYNC);
   if((fd < 0) || (openport(fd, BAUDRATE, 0) < 0)){
      printf("\n\rUnable to open serial port\n\r"); 
   }

   // Init app
   app = gtk_application_new (APP_ID, G_APPLICATION_FLAGS_NONE);
   g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
   status = g_application_run (G_APPLICATION (app), argc, argv);
   g_object_unref (app);
   
   return status;
}
