// Space Shuttle
//

//#include <wiringPiSPI.h>
//#include <bcm2835.h>

#include "libfreenect.hpp"
#include <iostream>
#include <vector>
#include <cmath>

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include <fcntl.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include <wiringSerial.h>

#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <unistd.h>
#include <pthread.h>

#include "mongoose.h"

int fd_serial = 0;
int fd_lidar = 0;

char lidarbuf[10000];
int  lidarbuflen = 0;
#define MAXLIDARDATA 360
int  lidardata[MAXLIDARDATA];
int	lidardataTimeout[MAXLIDARDATA];
int VBat = 0;

pthread_mutex_t serialQueueMutex;

Freenect::Freenect freenect;
MyFreenectDevice* device;
double freenect_angle(0);
freenect_video_format requested_format(FREENECT_VIDEO_RGB);

// command will be send if first byte is not 0 
struct serialQueue_s {
  char cmdMove[30];
  char cmdLook[30];
  char cmdColor1[30];
} serialQueue;

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float mapf(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int _getch ( void )
 {
  struct termios org_mode, cur_mode;  
  int tty_desc;
  char ch;
  
  tty_desc = open ( "/dev/tty", O_RDWR );
  
  ioctl ( tty_desc, TCGETS, &org_mode );
  ioctl ( tty_desc, TCGETS, &cur_mode );

  cur_mode.c_iflag &= ~ICRNL;
  cur_mode.c_lflag &= ~(ICANON | ECHO);
  cur_mode.c_cc[VTIME] = 0;
  cur_mode.c_cc[VMIN] = 1;
  ioctl ( tty_desc, TCSETS, &cur_mode );
  
  read ( tty_desc, &ch, 1 );
      
  ioctl ( tty_desc, TCSETS, &org_mode );
  close ( tty_desc );
  
  return ch;
}

int _kbhit ( void )
 {
  struct termios org_mode, cur_mode;  
  int tty_desc;
  int nchars, result = 0;
  
  tty_desc = open ( "/dev/tty", O_RDWR );
  
  ioctl ( tty_desc, TCGETS, &org_mode );
  ioctl ( tty_desc, TCGETS, &cur_mode );

  cur_mode.c_iflag &= ~ICRNL;
  cur_mode.c_lflag &= ~(ICANON | ECHO);
  cur_mode.c_cc[VTIME] = 0;
  cur_mode.c_cc[VMIN] = 1;
  ioctl ( tty_desc, TCSETS, &cur_mode );
  
  ioctl ( tty_desc, FIONREAD, &nchars );
  if ( nchars > 0 ) result = 1;
      
  ioctl ( tty_desc, TCSETS, &org_mode );
  close ( tty_desc );
  
  return result;
}

unsigned int micros (void)
{
  struct timeval tv ;
  uint64_t now ;

  gettimeofday (&tv, NULL) ;
  now  = (uint64_t)tv.tv_sec * (uint64_t)1000000 + (uint64_t)tv.tv_usec ;

  return (uint32_t)(now) ;
}

void delay (unsigned int howLong)
{
  struct timespec sleeper, dummy ;

  sleeper.tv_sec  = (time_t)(howLong / 1000) ;
  sleeper.tv_nsec = (long)(howLong % 1000) * 1000000 ;

  nanosleep (&sleeper, &dummy) ;
}

void delayMicrosecondsHard (unsigned int howLong)
{
  struct timeval tNow, tLong, tEnd ;

  gettimeofday (&tNow, NULL) ;
  tLong.tv_sec  = howLong / 1000000 ;
  tLong.tv_usec = howLong % 1000000 ;
  timeradd (&tNow, &tLong, &tEnd) ;

  while (timercmp (&tNow, &tEnd, <))
    gettimeofday (&tNow, NULL) ;
}

void delayMicroseconds (unsigned int howLong)
{
  struct timespec sleeper ;
  unsigned int uSecs = howLong % 1000000 ;
  unsigned int wSecs = howLong / 1000000 ;

  /**/ if (howLong ==   0)
    return ;
  else if (howLong  < 100)
    delayMicrosecondsHard (howLong) ;
  else
  {
    sleeper.tv_sec  = wSecs ;
    sleeper.tv_nsec = (long)(uSecs * 1000L) ;
    nanosleep (&sleeper, NULL) ;
  }
}


static int begin_request_handler(struct mg_connection *conn) {
  const struct mg_request_info *ri = mg_get_request_info(conn);
  char post_data[1024], input1[sizeof(post_data)], input2[sizeof(post_data)];
  int post_data_len, ret = 0, i;
  long Color1; 
  char next_cmd[30] = "";
  static int shutdown_cnt = 0;

  if ((!strcmp(ri->uri, "/cgi")) || (!strcmp(ri->uri, "/cgilong"))  || (!strcmp(ri->uri, "/lidar"))) {
    // User has submitted a form, show submitted data and a variable value
    post_data_len = mg_read(conn, post_data, sizeof(post_data));

    // Parse form data. input1 and input2 are guaranteed to be NUL-terminated
    
    // Scan for match with all registered query handlers
//  for(i=0; i<QueryListEntryCount; i++) {
//      if(mg_get_var(post_data, post_data_len, QueryList[i]->SearchString, input1, QueryList[i]->ValMaxLen) >= 0) {
//          strcpy(QueryList[i]->Val, input1);
//            QueryList[i]->Flags |= 1;
//          if(QueryList[i]->ChangeCallback != NULL) QueryList[i]->ChangeCallback(QueryList[i]->SearchString);
//      }
//  }   
         
     
    if(mg_get_var(post_data, post_data_len, "cmd", input1, sizeof(input1)) >= 0) { 
      if(!strcmp(input1, "move")) {
        shutdown_cnt = 0;  
        short x=0,y=0,r=0;
        if(mg_get_var(post_data, post_data_len, "x", input1, sizeof(input1)) >= 0) {
          long l = strtol(input1, NULL, 10);
          if(l > 32000L ) l =  32000L;
          if(l < -32000L) l = -32000L;
          x = (short)l;
        }  
        if(mg_get_var(post_data, post_data_len, "y", input1, sizeof(input1)) >= 0) {
          long l = strtol(input1, NULL, 10);
          if(l > 32000L ) l =  32000L;
          if(l < -32000L) l = -32000L;
          y = (short)l;
        }
        if(mg_get_var(post_data, post_data_len, "r", input1, sizeof(input1)) >= 0) {
          long l = strtol(input1, NULL, 10);
          if(l > 32000L ) l =  32000L;
          if(l < -32000L) l = -32000L;
          r = (short)l;
        }        
        sprintf(next_cmd, "m %d %d %d\n", x,y,r);      
        printf("%s\n", next_cmd);
        pthread_mutex_lock(&serialQueueMutex);
        strcpy(serialQueue.cmdMove, next_cmd);
        pthread_mutex_unlock(&serialQueueMutex);   
          
      } 
      if(!strcmp(input1, "extra")) {
        shutdown_cnt = 0;
        short data=0;
        if(mg_get_var(post_data, post_data_len, "data", input1, sizeof(input1)) >= 0) {
          long l = strtol(input1, NULL, 10);
          if(l > 32000L ) l =  32000L;
          if(l < -32000L) l = -32000L;
          data = (short)l;
        }  
        
        if((data > 0) && (data < 10)) {
            char cmd[100];
            sprintf(cmd, "./skripts/%d.sh", data);
            printf("Executing: %s\n", cmd);
            system(cmd);
        }
        
        /*
        sprintf(next_cmd, "m %d %d %d\n", x,y,r);      
        printf("%s\n", next_cmd);
        pthread_mutex_lock(&serialQueueMutex);
        strcpy(serialQueue.cmdMove, next_cmd);
        pthread_mutex_unlock(&serialQueueMutex);   
        */  
      }
      if(!strcmp(input1, "shutdown")) {
          // must be triggered 3x 
          shutdown_cnt++;
          if(shutdown_cnt > 3) {
              shutdown_cnt = 0;
              printf("Shutting down!");
              sprintf(next_cmd, "p!999\n");      
              printf("%s\n", next_cmd);
              pthread_mutex_lock(&serialQueueMutex);
              strcpy(serialQueue.cmdMove, next_cmd);
              pthread_mutex_unlock(&serialQueueMutex);              
              system("./skripts/off.sh"); 
          }
      }      
    }
    
    if(mg_get_var(post_data, post_data_len, "Color1", input1, sizeof(input1)) >= 0) { 
      Color1 = strtol(input1, NULL, 16);
      sprintf(next_cmd, "c %d %d %d\n", (Color1 >> 16) & 0xff, (Color1 >> 8) & 0xff, (Color1 >> 0) & 0xff);      
      // printf("%s %x\n", input1, Color1);
      pthread_mutex_lock(&serialQueueMutex);
      strcpy(serialQueue.cmdColor1, next_cmd);
      pthread_mutex_unlock(&serialQueueMutex);  
    }

    if(mg_get_var(post_data, post_data_len, "Look", input1, sizeof(input1)) >= 0) { 
      long l = strtol(input1, NULL, 10);
      sprintf(next_cmd, "N %d %d\n", (l >> 16) & 0x7fff, l & 0x7fff);      
      //printf("%s\n", next_cmd);
      pthread_mutex_lock(&serialQueueMutex);
      strcpy(serialQueue.cmdLook, next_cmd);
      pthread_mutex_unlock(&serialQueueMutex);  
    }    
    
    
    if(!strcmp(ri->uri, "/cgilong")) {
        // Send reply to the client, showing submitted form values.
        mg_printf(conn, "HTTP/1.0 200 OK\r\n"
              "Content-Type: text/plain\r\n\r\n"
              "cgi Submitted data: [%.*s]\n"
              "Submitted data length: %d bytes\n"
              "VBat: %d mV\n"
              , post_data_len, post_data, post_data_len, VBat);  
        printf("[%.*s]\n", post_data_len, post_data);
        fflush(stdout);
    } else if(!strcmp(ri->uri, "/cgi")) {
        // Send short JSON answer
        mg_printf(conn, "HTTP/1.0 200 OK\r\n"
              "Content-Type: text/plain\r\n\r\n"
              "{\"VBat\":%d}\n"
              , VBat);  
        printf("[%.*s]\n", post_data_len, post_data);
        fflush(stdout);
    } else if(!strcmp(ri->uri, "/lidar")) {
        char buf[MAXLIDARDATA*8];
        int iBuf;
        char bufInt[12];
        buf[0] = 0;
        for(iBuf = 0; iBuf<MAXLIDARDATA; iBuf++) {
          // ltoa(lidardata[iBuf],bufInt,10);
          sprintf(bufInt,"%d,",lidardata[iBuf]);
          strcat(buf,bufInt);
        }
        strcat(buf,"0");
        mg_printf(conn, "HTTP/1.0 200 OK\r\n"
              "Content-Type: text/plain\r\n\r\n"
              "{\"VBat\":%d,\"lidar\":[%s]}\n"
              , VBat,buf);  

        //printf("lidar[%.*s]\n", post_data_len, post_data);
        //fflush(stdout);
    }
    ret = 1;
  } else {
    // Show HTML form.
 /*   mg_printf(conn, "HTTP/1.0 200 OK\r\n"
              "Content-Length: %d\r\n"
              "Content-Type: text/html\r\n\r\n%s",
              (int) strlen(html_form), html_form);
              */
  }
  return ret;  // Mark request as processed
}

/*
extern void  serialFlush     (const int fd) ;
extern void  serialPutchar   (const int fd, const unsigned char c) ;
extern void  serialPuts      (const int fd, const char *s) ;
extern void  serialPrintf    (const int fd, const char *message, ...) ;
extern int   serialDataAvail (const int fd) ;
extern int   serialGetchar   (const int fd) ;

Kommandos absetzen, ggf Antwort abwarten.

*/

void doLidar(void) {
  int i;
  static int lidarmax = 0;
  if(!fd_lidar) return;
  int recCnt = serialDataAvail(fd_lidar);
  if(recCnt<=0) return;
  if(lidarbuflen + recCnt >= 10000) recCnt = 10000 - lidarbuflen;
  
  for(i=0; i<recCnt; i++) {
    char c = serialGetchar(fd_lidar);
    if(c>=' ') {
      lidarbuf[lidarbuflen++] = c;
    } else if(c == 10) {

      lidarbuf[lidarbuflen] = 0;
      // parse lidar line
      int arg1, arg2;
      //printf(lidarbuf);
      //printf("\n");
      //fflush(stdout);
      if(sscanf(lidarbuf,"s%d,%de",&arg1,&arg2) == 2) {
        if(arg2>lidarmax) {
          lidarmax = arg2;
          printf("max: %d\n",lidarmax);
        } 
        // printf("arg1: %d arg2: %d\n", arg1,arg2);
        // fflush(stdout);
        arg2 = (arg2*MAXLIDARDATA)/22900;
        if(arg2 < MAXLIDARDATA) {
		lidardata[arg2] = arg1;
		lidardataTimeout[arg2] = 0;
	}
      } 
      lidarbuflen = 0;
    }
  }

	for(i=0; i<MAXLIDARDATA; i++) {
		lidardataTimeout[i]++;
		if(lidardataTimeout[i]>300) {
			lidardata[i] = 0;
			lidardataTimeout[i] = 0;
		}
}

}

// *s should point to buffer with 32 chars
int serialGetLine(char *msgBuf) {
  if(!fd_serial) return -1;
  msgBuf[0] = 0;
  int receiveCnt = 0;
  uint32_t starttime = micros();
  int notDone = 1;
  // wait with time-out for data arrival
  while(notDone && ((micros() - starttime) < 100000L) && (receiveCnt<30)) {
    if(serialDataAvail(fd_serial)) {
      char c = serialGetchar(fd_serial);
      if(c>=' ') {
        msgBuf[receiveCnt++] = c;
      } else if(c == 10) {
        msgBuf[receiveCnt] = 0;
        notDone = 0;        
      }
    }    
  }
  if(notDone) return -2;  
  return receiveCnt;
}

int serialGetPrompt(void) {
  if(!fd_serial) return -1;
  uint32_t starttime = micros();
  int notDone = 1;
  // wait with time-out for data arrival
  while(notDone && ((micros() - starttime) < 100000L)) {
    if(serialDataAvail(fd_serial)) {
      char c = serialGetchar(fd_serial);
      if(c=='>') notDone = 0;        
    }    
  }
  if(notDone) return -2;  
  return 0;
}

int getVBat(void) {
  if(!fd_serial) return -1;
  int retVal;
  char msgBuf[32];
  //delay(200);
  while(serialDataAvail(fd_serial)) serialGetchar(fd_serial);
  serialPuts(fd_serial, "v\n");
  retVal = serialGetLine(msgBuf);
  //printf("mB: %s\n", msgBuf);
  if(retVal > 0) {
    retVal = atoi(msgBuf);
  } 
  serialGetPrompt();
  return retVal;
}

void doSerialQueue(void) {
  char next_cmd[30] = "";
  if(!fd_serial) return;
  
  // get last move command
  pthread_mutex_lock(&serialQueueMutex);
  strcpy(next_cmd, serialQueue.cmdMove);
  serialQueue.cmdMove[0] = 0;
  pthread_mutex_unlock(&serialQueueMutex);
  if(next_cmd[0] != 0) {
    serialPuts(fd_serial, next_cmd);
    serialGetPrompt();
  }

  // get last look command
  pthread_mutex_lock(&serialQueueMutex);
  strcpy(next_cmd, serialQueue.cmdLook);
  serialQueue.cmdLook[0] = 0;
  pthread_mutex_unlock(&serialQueueMutex);
  if(next_cmd[0] != 0) {
    serialPuts(fd_serial, next_cmd);
    serialGetPrompt();
  }  

  // get last color1 command
  pthread_mutex_lock(&serialQueueMutex);
  strcpy(next_cmd, serialQueue.cmdColor1);
  serialQueue.cmdColor1[0] = 0;
  pthread_mutex_unlock(&serialQueueMutex);
  if(next_cmd[0] != 0) {
    serialPuts(fd_serial, next_cmd);
    //printf("C1: %s\n", next_cmd);
    serialGetPrompt();
  }    
  
}

int main(int argc, char **argv)
{

  //Get Kinect Device
  device = &freenect.createDevice<MyFreenectDevice>(0);
  //Start Kinect Device
  device->setTiltDegrees(10);
  //device->startVideo();
  // device->startDepth();
  //handle Kinect Device Data
  device->setLed(LED_RED);
  // displayKinectData(device);
  //Stop Kinect Device
  //device->stopVideo();
  // device->stopDepth();
  // device->setLed(LED_OFF);


  struct mg_context *ctx;
  const char *options[] = {"listening_ports", "5580", "document_root", "sites", NULL};
  struct mg_callbacks callbacks;

  int i;
  uint32_t starttime, difftime;
  
  serialQueue.cmdColor1[0] = 0;
  serialQueue.cmdLook[0] = 0;
  serialQueue.cmdMove[0] = 0;
  pthread_mutex_init (&serialQueueMutex, NULL);
  
  memset(&callbacks, 0, sizeof(callbacks));
  callbacks.begin_request = begin_request_handler;
  ctx = mg_start(&callbacks, NULL, options);
  
  printf("Server started.\n       ");
  
  if ((fd_serial = serialOpen ("/dev/ttyUSB0", 38400)) < 0)
  {
    printf("Unable to open serial device: %s\n", strerror (errno)) ;
  } else {
    serialFlush(fd_serial);
    serialPuts(fd_serial,"\r\ncx\r\n");
    serialFlush(fd_serial);    
  }
  
  if ((fd_lidar = serialOpen ("/dev/ttyS0", 115200)) < 0)
  {
    printf("Unable to open serial device for LIDAR: %s\n", strerror (errno)) ;
  } else {
    serialFlush(fd_lidar);
    serialPuts(fd_lidar,"\r\nr\r\n");
    serialFlush(fd_lidar);    
  }
  
  starttime = micros();
  i = 0;

  while(_kbhit() == 0 /* _getch() != 'q' */) {
      
      // fps estimation
      if((micros() - starttime) >= 1000000) {
          starttime = micros();
      //    printf("\b\b\b\b\b\b\b%03d fps", i);
      //    fflush(stdout);
          i = 0;
          //serialFlush(fd_serial);
          VBat = getVBat();
          // VBat = 0;
          
      }
      i++;
      doSerialQueue();
      doLidar();
      usleep(1000);
  }
  
  printf("done.\n");
  printf("VBat: %d\n", VBat);

  mg_stop(ctx);
  if(fd_serial) { serialPuts(fd_serial, "\n \n"); delay(300); serialClose(fd_serial); }   
  if(fd_lidar) { serialClose(fd_lidar);}
  return 0;
}

