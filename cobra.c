#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>

void writecmd(char* cmd,int tty_fd){
   char buf[50];
   write(tty_fd,"5",1);//Aktion anfordern
   while(buf[0] != '6'){//wait for bereit
      read(tty_fd,buf,1);
   }
   strcpy(buf,cmd);
   strcat(buf,"\r\n");
   write(tty_fd,buf,strlen(buf));//Befehl senden
   while(buf[0] != '3'){//wait for Verstanden
      read(tty_fd,buf,1);
   }
}

void readcmd(char* cmd,int tty_fd){
   writecmd("G",tty_fd);//request command
   usleep(200000);//wait for cobra to write data
   fgets(cmd,50,fdopen(tty_fd,"r"));
}
 
int main(int argc,char** argv)
{
   struct termios tio;
   struct termios stdio;
   int tty_fd;
   FILE *file;
   fd_set rdset;
   
   char cmd[100];
   char buf[100];
   
   if(argc < 3){
      printf("usage: %s device [read|write file|cal|home|clear|start|lbl num|D mot1 mot2 mot3 mot4 mot5 mot6]\r\n",argv[0]);
      exit(EXIT_FAILURE);
   }
   
   memset(&tio,0,sizeof(tio));
   tio.c_iflag=0;
   tio.c_oflag=0;
   tio.c_cflag=CS8|CREAD|CLOCAL|CSTOPB;//8n2
   tio.c_lflag=0;
   tio.c_cc[VMIN]=1;
   tio.c_cc[VTIME]=5;
   
   tty_fd=open(argv[1], O_RDWR | O_NONBLOCK);   
   if(tty_fd == -1){
      printf("cannot open device\r\n");
      exit(EXIT_FAILURE);
   }  
    
   cfsetospeed(&tio,B4800);
   cfsetispeed(&tio,B4800);
   tcsetattr(tty_fd,TCSANOW,&tio);
   
   if(strcmp(argv[2],"read") == 0){
      writecmd("C",tty_fd);//set program counter to 0
      while(cmd[0] != '*'){
         readcmd(cmd,tty_fd);
         if(cmd[0] != '*'){
            printf("%s",cmd);
         }
      }
   }
   else if(strcmp(argv[2],"cal") == 0){
      writecmd("N",tty_fd);
   }
   else if(strcmp(argv[2],"home") == 0){
      writecmd("H",tty_fd);
   }
   else if(strcmp(argv[2],"clear") == 0){
      writecmd("R",tty_fd);
   }
   else if(strcmp(argv[2],"start") == 0){
      writecmd("C",tty_fd);//set program counter to 0
      writecmd("E",tty_fd);//start
   }
   else if(argv[2][0] == 'D' && argc == 9){
      strcpy(cmd,"D ");
      strcat(cmd,argv[3]);
      strcat(cmd," ");
      strcat(cmd,argv[4]);
      strcat(cmd," ");
      strcat(cmd,argv[5]);
      strcat(cmd," ");
      strcat(cmd,argv[6]);
      strcat(cmd," ");
      strcat(cmd,argv[7]);
      strcat(cmd," ");
      strcat(cmd,argv[8]);
      writecmd(cmd,tty_fd);//set program counter to 0
   }
   else if(strcmp(argv[2],"write") == 0 && argc == 4){
      file = fopen(argv[3],"r");
      if(!file){
         printf("cannot open file\r\n");
         exit(EXIT_FAILURE);
      }
      writecmd("R",tty_fd);//clear
      writecmd("C",tty_fd);//set program counter to 0
      while (fgets(buf, 50, file)){
         strcpy(cmd,"S");
         strcat(cmd,buf);
         writecmd(cmd,tty_fd);
      }
      fclose(file);
   }
   else if(strcmp(argv[2],"lbl") == 0 && argc == 4){
      strcpy(cmd,"E ");
      strcat(cmd,argv[3]);
      writecmd(cmd,tty_fd);//start
   
   }
   
   close(tty_fd);
   exit(EXIT_SUCCESS);
}