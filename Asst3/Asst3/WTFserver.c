#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <netinet/in.h>

/* WTF SERVER FILE

  The server is set up to send the client a number that dictates whether to continue or if the operation fails.
  Messages To Send :
      0 = Operation Was Successful
      1 = Operation Failed

  Server will read a single char from client depending on what argument client gives.
  Table for operations by client :
      x = ERROR
      a = Create
      b = Destroy
      c = Add
      d = Remove
      e = CurrentVersion
      f = Update
      g = Checkout
      h = History
      i =
      j =
      o = Commit

  File Sending Protocol :
    First, send client # of files being recieved (will be just a single int)
        For each file being sent, send client...
        ____
        |    # Bytes in file name
        |    File name
        |    # Bytes in file
        |___      ---> Then send bytes of file one character at a time.

        Repeat Bracket For Each File.


*/

//-----------------------------------------------------------------------------------------| FUNCTION DELCARATIONS

void create(char* projectName, int clientfd);
int getfilelength(char* filepath);
int checkIfProjectDirExists(char* projectName);
void destroy(char* projectName);
void sendcurrentversion(char* projName, int clientfd);
void history(char* projectName, int clientfd);
void sendManifest(char* projectName, int clientfd);
void checkout(char* projectName, int clientfd);
void upgrade(char* projectName, int clientfd);
void download_files(int clientfd);
void push(char* projectName, int clientfd);
void rollback(char* projectName, char* backupname, char* rollbackversion, int clientfd);
void quitfunc();

//-----------------------------------------------------------------------------------------| GLOBAL VARS

int socketfd = 0;
int clientfd = 0;
char operation = 'x';

//-----------------------------------------------------------------------------------------| MAIN

int main(int argc, char** argv){
  printf("WTF Server Started\n\n");
  atexit(quitfunc);

//------------------------------------------------| SERVER SETUP / CREATION

  int portnum = 0;
  int clientlength = 0;
  int wr;
  struct sockaddr_in serv_addr, cli_addr;

  if(argc == 1){
    printf("Error, Port Was Not Supplied To Server!\n");
    return EXIT_FAILURE;
  }

  if(argc > 2){
      printf("Error, Too Many Server Arguments Were Given!\n");
      return EXIT_FAILURE;
  }
  else{
            int iz = 0;
            for(iz = 0; iz < strlen(argv[1]); iz++){
                //printf("Rollback[%d] = %c\n", i, rollbackversion[i]);
                if((argv[1][iz] != '1') &&
                    (argv[1][iz] != '2') &&
                    (argv[1][iz] != '3') &&
                    (argv[1][iz] != '4') &&
                    (argv[1][iz] != '5') &&
                    (argv[1][iz] != '6') &&
                    (argv[1][iz] != '7') &&
                    (argv[1][iz] != '8') &&
                    (argv[1][iz] != '9') &&
                    (argv[1][iz] != '0')){//IF
                        printf("The Port Number Contains Illegal Characters (Not 0-9). Shutting Server Down\n");
                        return EXIT_FAILURE;
                }//IF
              }//FOR

                portnum = atoi(argv[1]);
                if(portnum > 65535){
                      printf("The Port Number Entered Is Too High. A Server Port Can Range Between 0 - 65535 (%d Is Not A Valid Port).\n", portnum);
                      return EXIT_FAILURE;
                }

  }//else
  socketfd = socket(AF_INET, SOCK_STREAM, 0);

    if(socketfd < 0){
         printf("Socket Creation Failed!\n");
         return EXIT_FAILURE;
    }

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portnum);
      if(bind(socketfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
         printf("Binding Socket Failed!\n");
         return EXIT_FAILURE;
      }

//------------------------------------------------| LISTENER FOR CONNECTIONS

  listen(socketfd, 10);
  clientlength = sizeof(cli_addr);

  mkdir("./Server/", 0777); //Will fail (= -1) in most cases as server folder will probably be present. Mainly to be succeessful on the first occasion of server being ran... just in case
  chdir("./Server/");
      mkdir(".Backups", 0777);

//------------------------------------------------| LOOP AWAITING INPUT FROM CLIENT

while(1){

  clientfd = accept(socketfd, (struct sockaddr *)&cli_addr, &clientlength);
  int pid = 1;

  if(clientfd < 0){
     printf("Error Accepting Client\n");
     return EXIT_FAILURE;
  }

      printf("\n| CLIENT CONNECTED |\n");

        pid = fork();
        if(pid != 0){//PARENT PROCESS THREAD IS EXECUTING

        }

  else{//CHILD IS BEING EXECUTED

  //printf("(A New Process Was Created With PID %d).\n", pid);

  //READ FROM CLIENT
  operation = 'x';
  wr = read(clientfd, &operation, 1);

  //printf("Operation Sent From Client : %c\n", operation);

                                        if(operation == 'a'){ //CREATE
                                          printf("Client Requested CREATE\n");
                                          char projectName[256];
                                          bzero(projectName, 256);
                                          wr = read(clientfd, projectName, 255);
                                          //printf("Project Name : %s\n", projectName);
                                          int check = checkIfProjectDirExists(projectName);
                                            if(check == -1){//Proj doesnt exist
                                                  mkdir(projectName, 0777);
                                                  printf("New Project '%s' Created\n", projectName);
                                                  wr = write(clientfd, "0", 1);
                                                  create(projectName, clientfd);
                                            }
                                            else{
                                                  printf("Failed To Make New Project (One With The Same Name Already Exists).\n");
                                                  wr = write(clientfd, "1", 1);
                                            }
                                        }//CREATE

      else if(operation == 'b'){ //DESTROY
        printf("Client Requested DESTROY\n");
        char projectName[256];
        bzero(projectName, 256);
        wr = read(clientfd, projectName, 255);
        char pathname[1024] = "./";
        strcat(pathname, projectName);

        int check = checkIfProjectDirExists(projectName);
            if(check == 0){
              destroy(pathname);
                    //NEED TO DESTROY BACKUPS
                    char backupspath[1024] = "./.Backups/";
                    strcat(backupspath, projectName);
                    destroy(backupspath);

              wr = write(clientfd, "0", 2);
              printf("Destroyed Project '%s'\n", projectName);
              wr = write(clientfd, "Project Was Successfully Destroyed.\n", 37);
            }
            else{
              printf("Failed To Destroy Project, As It Does Not Exist.\n");
              wr = write(clientfd, "1", 2);
            }
      }//DESTROY

                                                        if(operation == 'c'){ //ADD
                                                            printf("Client Requested ADD\n");
                                                            wr = write(clientfd, "0", 2); //Server saw that add happened
                                                            printf("An Entry Was Added To A Client-Side Manifest\n");
                                                        }//ADD

      if(operation == 'f'){ //UPDATE
        printf("Client Requested UPDATE\n");
        int canupdate;
        wr = read(clientfd, &canupdate, sizeof(canupdate));
        //printf("Value Of Cancheck = '%d'\n", cancheckout);

        if(canupdate == 0){//CLIENT HAS PROJECT ---> SO CHECKOUT CAN BE DONE
                char projectName[256];
                bzero(projectName, 256);
                wr = read(clientfd, projectName, 255);
                //printf("Project Name : %s\n", projectName);

                      int check = checkIfProjectDirExists(projectName);
                              if(check == 0){//PROJ DIR EXISTS ON SERVER
                                    wr = write(clientfd, "0", 2);
                                    printf("Updating Project '%s'...\n", projectName);
                                    sendManifest(projectName, clientfd);
                                    printf("Client Completed Update.\n");

                              }
                              else{//PROJ DIR DOESNT EXIST ON SERVER
                                    wr = write(clientfd, "1", 2);
                                    printf("Failed To Update Project, As One With The Name '%s' Does Not Exist.\n", projectName);
                              }
        }//IF
        else{//CLIENT HAS NO PROJECT WITH THAT NAME
              printf("An Upgrade Was Requested, But Could Not Be Completed (The Client Does Not Has An Existing Project With The Name Requested).\n");
        }
      }//UPDATE

                                                  if(operation == 'd'){ //REMOVE
                                                      printf("Client Requested REMOVE\n");
                                                      wr = write(clientfd, "0", 2); //Server saw that remove happened
                                                      printf("An Entry Was Removed From A Client-Side Manifest\n");
                                                  }//REMOVE

      else if(operation == 'e'){ //CURRENTVERSION
          printf("Client Requested CURRENTVERSION\n");
          char projectName[256];
          bzero(projectName, 256);
          wr = read(clientfd, projectName, 255);

          int check = checkIfProjectDirExists(projectName);
              if(check == 0){
                wr = write(clientfd, "0", 2);
                sendcurrentversion(projectName, clientfd);
                printf("Sent Client The Current Version.\n");
              }
              else{
                wr = write(clientfd, "1", 2);
                printf("Failed To Send Client The Current Project Version, As The Project Does Not Exist.\n");
              }
      }//CURRENTVERSION

                                                            else if(operation == 'g'){ //CHECKOUT
                                                              printf("Client Requested CHECKOUT\n");
                                                                int cancheckout;
                                                                wr = read(clientfd, &cancheckout, sizeof(cancheckout));
                                                                //printf("Value Of Cancheck = '%d'\n", cancheckout);

                                                                if(cancheckout == -1){//CLIENT DOESNT HAVE PROJECT ---> SO CHECKOUT CAN BE DONE
                                                                        char projectName[256];
                                                                        bzero(projectName, 256);
                                                                        wr = read(clientfd, projectName, 255);
                                                                        //printf("Project Name : %s\n", projectName);

                                                                              int check = checkIfProjectDirExists(projectName);
                                                                                      if(check == 0){//PROJ DIR EXISTS ON SERVER
                                                                                            wr = write(clientfd, "0", 2);
                                                                                            checkout(projectName, clientfd);
                                                                                            printf("Completed A Checkout With Project '%s'.\n", projectName);
                                                                                      }
                                                                                      else{//PROJ DIR DOESNT EXIST ON SERVER
                                                                                            wr = write(clientfd, "1", 2);
                                                                                            printf("Could Not Complete A Checkout With Project '%s', As It Does Not Exist.\n", projectName);
                                                                                      }
                                                                }//IF
                                                                else{//CLIENT HAS A PROJECT WITH THAT NAME
                                                                      printf("A Checkout Was Requested, But Could Not Be Completed (The Client Already Has An Existing Project With The Name Requested).\n");
                                                                }
                                                            }//CHECKOUT

      if(operation == 'i'){ //UPGRADE
        printf("Client Requested UPGRADE\n");
        int canupgrade;
        wr = read(clientfd, &canupgrade, sizeof(canupgrade));
        //printf("Value Of Cancheck = '%d'\n", cancheckout);

        if(canupgrade == 0){//CLIENT HAS PROJECT ---> SO CHECKOUT CAN BE DONE
                char projectName[256];
                bzero(projectName, 256);
                wr = read(clientfd, projectName, 255);
                //printf("Project Name : %s\n", projectName);

                      int check = checkIfProjectDirExists(projectName);
                              if(check == 0){//PROJ DIR EXISTS ON SERVER
                                    wr = write(clientfd, "0", 2);
                                    upgrade(projectName, clientfd);

                              }
                              else{//PROJ DIR DOESNT EXIST ON SERVER
                                    wr = write(clientfd, "1", 2);
                                    printf("Could Not Complete An Upgrade With Project '%s', As It Does Not Exist.\n", projectName);
                              }
        }//IF
        else{//CLIENT HAS NO PROJECT WITH THAT NAME
              printf("An Upgrade Was Requested, But Could Not Be Completed (The Client Does Not Has An Existing Project With The Name Requested).\n");
        }
      }//UPGRADE

                                      else if(operation == 'j'){ //ROLLBACK
                                        printf("Client Requested ROLLBACK\n");
                                          char projectName[256] = "";
                                          char rollbackversion[16] = "";
                                          wr = read(clientfd, projectName, 255);

                                          int check = checkIfProjectDirExists(projectName);
                                              if(check == 0){
                                                  wr = write(clientfd, "0", 2);
                                                  wr = read(clientfd, rollbackversion, 16);
                                                  printf("Client Wants To Roll Back %s To Version '%s'\n", projectName, rollbackversion);
                                                        char backupname[1024] = "./.Backups/";
                                                              strcat(backupname, projectName);
                                                              strcat(backupname, "/");
                                                              strcat(backupname, projectName);
                                                              strcat(backupname, ":");
                                                              strcat(backupname, rollbackversion);

                                                              //printf("Looking For Folder %s\n", backupname);
                                                              rollback(projectName, backupname, rollbackversion, clientfd);
                                              }
                                              else{
                                                  wr = write(clientfd, "1", 2);
                                                  printf("Failed To Rollback - The Project '%s' Does Not Exist!\n", projectName);
                                              }
                                      }//ROLLBACK

      if(operation == 'o'){ //COMMIT
        printf("Client Requested COMMIT\n");
        int canupdate;
        wr = read(clientfd, &canupdate, sizeof(canupdate));

        if(canupdate == 0){//CLIENT HAS PROJECT ---> SO CHECKOUT CAN BE DONE
                char projectName[256];
                bzero(projectName, 256);
                wr = read(clientfd, projectName, 255);
                //printf("Project Name : %s\n", projectName);

                      int check = checkIfProjectDirExists(projectName);
                              if(check == 0){//PROJ DIR EXISTS ON SERVER
                                    wr = write(clientfd, "0", 2);
                                    printf("Commiting Project '%s'...\n", projectName);
                                    sendManifest(projectName, clientfd);
                                        wr = read(clientfd, &check, sizeof(check));
                                        if(check == 0){//CHECK = 0 MEANS CLIENT COMPLETED COMMIT AND NEEDS THE SERVER TO DOWNLOAD THE COMMIT FILE
                                              printf("Downloading The Clients .Commit File...\n");
                                              chdir(projectName);
                                              download_files(clientfd);//DOWNLOAD .COMMIT
                                              chdir("..");
                                              printf("Completed Commit.\n");
                                        }
                                        else{
                                              printf("Client Could Not Successfully Complete Commit.\n");
                                        }
                              }
                              else{//PROJ DIR DOESNT EXIST ON SERVER
                                    wr = write(clientfd, "1", 2);
                                    printf("Failed To Commit Project, As One With The Name '%s' Does Not Exist.\n", projectName);
                              }
        }//IF
        else{//CLIENT HAS NO PROJECT WITH THAT NAME
              printf("A Commit Was Requested, But Could Not Be Completed (The Client Does Not Has An Existing Project With The Name Requested).\n");
        }
      }//COMMIT

                                                                if(operation == 'p'){ //PUSH
                                                                  printf("Client Requested PUSH\n");
                                                                  int canpush;
                                                                  wr = read(clientfd, &canpush, sizeof(canpush));

                                                                  if(canpush == 0){//CLIENT HAS PROJECT ---> SO CHECKOUT CAN BE DONE
                                                                          char projectName[256];
                                                                          bzero(projectName, 256);
                                                                          wr = read(clientfd, projectName, 255);
                                                                          //printf("Project Name : %s\n", projectName);

                                                                                int check = checkIfProjectDirExists(projectName);
                                                                                        if(check == 0){//PROJ DIR EXISTS ON SERVER
                                                                                              wr = write(clientfd, "0", 2);
                                                                                              printf("Pushing Project '%s'...\n", projectName);
                                                                                                  if(check == 0){//CHECK = 0 MEANS CLIENT COMPLETED COMMIT AND NEEDS THE SERVER TO DOWNLOAD THE COMMIT FILE
                                                                                                              push(projectName, clientfd);
                                                                                                  }
                                                                                                  else{
                                                                                                        printf("Client Could Not Successfully Complete Push.\n");
                                                                                                  }
                                                                                        }
                                                                                        else{//PROJ DIR DOESNT EXIST ON SERVER
                                                                                              wr = write(clientfd, "1", 2);
                                                                                              printf("Failed To Push Project, As One With The Name '%s' Does Not Exist.\n", projectName);
                                                                                        }
                                                                  }//IF
                                                                  else{//CLIENT HAS NO PROJECT WITH THAT NAME
                                                                        printf("A Push Was Requested, But Could Not Be Completed (The Client Does Not Has An Existing Project With The Name Requested).\n");
                                                                  }
                                                                }//PUSH

      else if(operation == 'h'){ //HISTORY
        printf("Client Requested HISTORY\n");
          char projectName[256] = "";
          wr = read(clientfd, projectName, 255);

          int check = checkIfProjectDirExists(projectName);
              if(check == 0){
                  wr = write(clientfd, "0", 2);
                      history(projectName, clientfd);
                      printf("Sent The Client A Copy Of The Servers Commit History.\n");
              }
              else{
                  wr = write(clientfd, "1", 2);
                  printf("Failed To Fetch History - The Project '%s' Does Not Exist!\n", projectName);
              }
      }//HISTORY

      close(clientfd);
      printf("| CLIENT DISCONNECTED |\n");
    }//ELSE IF PID == 0

      //printf("-------------------------------\n\n"); //COMMENT THIS OUT


}//WHILE

  return EXIT_SUCCESS;

}//END OF MAIN

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|

//-----------------------------------------------------------------------------------------| create

void create(char* projectName, int clientfd){ // ./WTF create <project name>

//CREATE .MANIFEST ON SERVER END
  char newprojdir[1024] = "./";
  strcat(newprojdir, projectName);
  chdir(newprojdir);
//  mkdir(projectName, 0777);

  int fd = open(".Manifest", O_CREAT | O_WRONLY | O_TRUNC, 0644); //make the manifest
      write(fd, "0          \n", 12);//PROJECT VERSION
  close(fd);

//SEND THE SERVERS .MANIFEST TO CLIENT ---> CLIENT READS THESE WRITES IN download_files() (Client Side Function)
  int filesbeingsent = 1;
  int fl = getfilelength("./.Manifest"); //File Length
  int fnl = strlen(".Manifest"); //File Name Length
      write(clientfd, &filesbeingsent, sizeof(filesbeingsent));//Sending # Files Being Sent
      write(clientfd, &fnl, sizeof(fnl));//Sending # Bytes In File Name
      write(clientfd, ".Manifest", fnl);//Sending File Name
      write(clientfd, &fl, sizeof(fl));//Sending File Length

  int i = 0;
  char toSend;
  fd = open(".Manifest", O_RDONLY);
  for(i = 0; i < fl; i++){
        read(fd, &toSend, 1);
        write(clientfd, &toSend, 1);
  }//For
  close(fd);

  chdir("..");
}//create

//-----------------------------------------------------------------------------------------| Destroy

void destroy(char* projectName){
      char newpath[1024];
      memset(newpath, '\0', 1024);
      strcpy(newpath, projectName);
      struct dirent *dent;
      DIR *dir = opendir(projectName);

      if(dir == NULL){
          printf("\tDestroying '%s'.\n", projectName);
          //THEN ITS NOT A DIR, ITS A File
          remove(projectName);
          return;
      }

      while((dent = readdir(dir)) != NULL){
          //Does not include self or parent directories
          if((strcmp(dent->d_name, "..") != 0) && (strcmp(dent->d_name, ".") != 0)){

              //UNCOMMENT THIS PRINTF AND COMMENT OUT TOKENIZE (BELOW) IF YOU WANT TO JUST SEE THE LIST OF ALL FILES IN THE SUPPLIED DIRECTORY
              //printf("\t%s\n", dent->d_name);

              //Concat Newpath To Traverse From Rootpath
              strcpy(newpath, projectName);
              strcat(newpath, "/");
              strcat(newpath, dent->d_name);

              destroy(newpath);
          }//IF NOT .. OR
          //printf("\tDestroying %s. (.)\n", projectName);
          remove(projectName);

      }//WHILE
      closedir(dir);
}//destroy

//-----------------------------------------------------------------------------------------| getfilelength

int getfilelength(char* filepath){
  char temp;
  int bytecount = 0;
  int fd = open(filepath, O_RDONLY);
    if(fd == -1){printf("ERROR OPENING FILE %s IN getfilelength()", filepath);}
            while(read(fd, &temp, 1) != 0){
              //printf("%c\n", temp);
              bytecount++;
            }
    close(fd);
  //printf("Bytes In %s : %d\n", filepath, bytecount);
  return bytecount;
}

//-----------------------------------------------------------------------------------------| checkIfProjectDirExists

int checkIfProjectDirExists(char* projectName){
  char newprojdir[1024] = "./";
  strcat(newprojdir, projectName);
  int status = chdir(newprojdir);
      if(status == -1){
          //Directory Doesn't Exist!
          return -1;
      }
      else{
          //Directory EXISTS
          chdir("..");
          return 0;
      }
}

//-----------------------------------------------------------------------------------------| SEND CURR VERSION

void sendcurrentversion(char* projName, int clientfd){
    char newprojdir[1024] = "./";
    strcat(newprojdir, projName);
    char manifestdir[1024];
    strcpy(manifestdir, newprojdir);
    strcat(manifestdir, "/.Manifest");

    //printf("Proj Directory : %s\n", newprojdir);
    //printf("Manifest Path : %s\n", manifestdir);

    int fd = open(manifestdir, O_RDONLY);
    if(fd == -1){
        printf(".Manifest File Was Not Found In %s.\n", newprojdir);
        close(fd);
        return;
    }

    //GET PROJ NUM FROM MANIFEST ON SERVER SIDE
    char buffer[256];
    bzero(buffer, 256);
    char onebyone = 'x';
        while(onebyone != '\n'){
            read(fd, &onebyone, 1);
            strncat(buffer, &onebyone, 1);
        }
    int projnum = atoi(buffer);
    //printf("Project Num = '%d'\n", projnum);
    write(clientfd, &projnum, sizeof(int)); //Write Proj Num To Client
    close(fd);

    //PUT MANIFEST FILE IN A BUFFER SO THAT WE CAN VIEW THE PATHS INSIDE OF IT
    int manfl = getfilelength(manifestdir);
    char* manifestbuffer = malloc(sizeof(char) * manfl);
    manifestbuffer[manfl] = '\0';

    fd = open(manifestdir, O_RDONLY);
    read(fd, manifestbuffer, manfl);
    close(fd);

    //FOR LOOP TO GET FILE COUNT IN MANIFEST
    int i = 0, newlncount = 0, filecount = 0;
    for(i = 0; i < manfl; i++){
      if(newlncount > 1 && (i <= manfl - 1)){ //Means we are past the .Manifest Proj Version and entering the part with
          if(manifestbuffer[i] == 'X'){// MUST ADD MORE FLAGS AS THIS PROJ GROWS
                if(manifestbuffer[i-1] == '\n'){// MUST ADD MORE FLAGS AS THIS PROJ GROWS
                      //printf("\tFile Found With Tag %c\n", manifestbuffer[i]);
                      filecount++;
                }
          }// IF F OR A
      }
      if(manifestbuffer[i] == '\n'){
        newlncount++;
      }
    }//FOR

    //printf("FILE COUNT : %d\n", filecount);
    write(clientfd, &filecount, sizeof(int)); //Write File Count Num To Client

    //FOR LOOP TO RETRIEVE THE ACTUAL FILE NAMES AND PROJECT VERSION
    char filepathstr[256]; int fpstrLEN = 0;
    char fileverstr[16];
    i = 0; newlncount = 0;

    for(i = 0; i < manfl; i++){//Loop for each file
          if(manifestbuffer[i] == 'X'){// MUST ADD MORE FLAGS AS THIS PROJ GROWS
                if((i != 0) && manifestbuffer[i-1] == '\n'){// MUST ADD MORE FLAGS AS THIS PROJ GROWS
                    bzero(filepathstr, 255);
                    bzero(fileverstr, 9);
                        int x;
                        for(i = i+2; manifestbuffer[i] != '\n'; i++){// GET PATH NAME
                              //printf("'%c'\n", manifestbuffer[i]); //TESTING PRINT
                              strncat(filepathstr, &manifestbuffer[i], 1);
                        }
                        //printf("\tPATH : %s\n", filepathstr);

                                    for(i = i+1; manifestbuffer[i] != '\n'; i++){// SKIP HASH
                                    }

                        for(i = i + 1; manifestbuffer[i] != '\n'; i++){// GET VERSION NUM
                              //printf("'%c'\n", manifestbuffer[i]); //TESTING PRINT
                              strncat(fileverstr, &manifestbuffer[i], 1);
                        }
                        //printf("\tVERSION : %s\n", fileverstr);

                        //ACTUALLY WRITING | count of bytes in pathname -> actual pathname -> versionnum
                        int filenamebytes = strlen(filepathstr);
                        int vernumbytes = strlen(fileverstr);
                        write(clientfd, &filenamebytes, sizeof(int));
                        write(clientfd, filepathstr, filenamebytes);
                        write(clientfd, &vernumbytes, sizeof(int));
                        write(clientfd, fileverstr, strlen(fileverstr));
                }//If we encountered a new path
          }// IF F OR A
      if(manifestbuffer[i] == '\n'){
        newlncount++;
      }
    }//FOR
}

//-----------------------------------------------------------| SendManifest (For Update)

 void sendManifest(char* projectName, int clientfd){

   char newprojdir[1024] = "./";
   strcat(newprojdir, projectName);
   //printf("PROJ DIR LOOKING IN : %s\n", newprojdir);
   chdir(newprojdir);

 //SEND THE SERVERS .MANIFEST TO CLIENT ---> CLIENT READS THESE WRITES IN download_files() (Client Side Function)
   int filesbeingsent = 1;
   int fl = getfilelength("./.Manifest"); //File Length
   int fnl = strlen(".ManifestServer"); //File Name Length
       write(clientfd, &filesbeingsent, sizeof(filesbeingsent));//Sending # Files Being Sent
       write(clientfd, &fnl, sizeof(fnl));//Sending # Bytes In File Name
       write(clientfd, ".ManifestServer", fnl);//Sending File Name
       write(clientfd, &fl, sizeof(fl));//Sending File Length

   int i = 0;
   char toSend;
   int fd = open(".Manifest", O_RDONLY);
             if(fd == -1){
                  printf("Manifest Could Not Be Found\n");
             }
   for(i = 0; i < fl; i++){
         read(fd, &toSend, 1);
         //printf("TS : %c\n", toSend);
         write(clientfd, &toSend, 1);
   }//For
   close(fd);

   //printf("Successfully Sent The Client The Requested .Manifest.\n");

   chdir("..");
 }//SendManifest

//-----------------------------------------------------------| CHECKOUT

 void checkout(char* projectName, int clientfd){

   sendManifest(projectName, clientfd);
   chdir(projectName);

   //Open Manifest And Inspect All File paths contained in it
   int readbytes = 0;
   int serverfdlength = getfilelength(".Manifest");
   int serverfd = open(".Manifest", O_RDONLY);

   char fullentrybuffer[1024] = "";
   char filepathbuffer[1024] = "";

   char minientrybuffer = '\0';
   int newlines = 0;
   int numfilestosend = 0;

   while(minientrybuffer != '\n'){//LOOP TO GET SERVER PROJECT NUMBER
       read(serverfd, &minientrybuffer, 1);
       //printf("|%c|\n", minientrybuffer);
       readbytes++;
   }
   read(serverfd, &minientrybuffer, 1); readbytes++; //Read the second newline (line #2 in the .ManifestServer)
   minientrybuffer = '\0';

               while(readbytes < serverfdlength){ //FIRST LOOK AT SERVER ENTRIES
                       bzero(fullentrybuffer, 1024);
                       bzero(filepathbuffer, 1024);
                       newlines = 0;

                       while(newlines != 5){//LOOP TO GET EACH ENTRY IN THE SERVER MANIFEST
                           read(serverfd, &minientrybuffer, 1);
                                     if(minientrybuffer == '\n'){
                                       newlines++;
                                     }
                           readbytes++;
                       }
                         numfilestosend++;
             }//WHILE READBYTES

  close(serverfd);
  serverfd = open(".Manifest", O_RDONLY);
  readbytes = 0; minientrybuffer = '\0';
  printf("The Client Will Be Recieving %d Files.\n", numfilestosend);
  write(clientfd, &numfilestosend, sizeof(numfilestosend));//Sending # Files Being Sent

                            while(minientrybuffer != '\n'){//LOOP TO GET SERVER PROJECT NUMBER
                                read(serverfd, &minientrybuffer, 1);
                                //printf("|%c|\n", minientrybuffer);
                                readbytes++;
                            }
                            read(serverfd, &minientrybuffer, 1); readbytes++; //Read the second newline (line #2 in the .ManifestServer)
                            minientrybuffer = '\0';

                                        while(readbytes < serverfdlength){ //FIRST LOOK AT SERVER ENTRIES
                                                bzero(fullentrybuffer, 1024);
                                                bzero(filepathbuffer, 1024);
                                                newlines = 0;

                                                while(newlines != 5){//LOOP TO GET EACH ENTRY IN THE SERVER MANIFEST
                                                    read(serverfd, &minientrybuffer, 1);
                                                        if((newlines == 1) && (minientrybuffer != '\n')){
                                                            strncat(filepathbuffer, &minientrybuffer, 1);
                                                        }
                                                              if(minientrybuffer == '\n'){
                                                                newlines++;
                                                              }
                                                    strncat(fullentrybuffer, &minientrybuffer, 1);
                                                    readbytes++;
                                                }
                                                  //printf("ENTRY ON SERVER :\n|%s|\n", fullentrybuffer);
                                                  printf("\t%s\n", filepathbuffer);
                                                  chdir("..");
                                                  int fl = getfilelength(filepathbuffer); //File Length
                                                  int fnl = strlen(filepathbuffer); //File Name Length
                                                  //printf("Length Of File : %d\n", fl);
                                                  //printf("Length Of File Name : %d\n", fnl);

                                                      write(clientfd, &fnl, sizeof(fnl));//Sending # Bytes In File Name
                                                      write(clientfd, filepathbuffer, strlen(filepathbuffer));//Sending File Name
                                                      write(clientfd, &fl, sizeof(fl));//Sending File Length

                                                      int i = 0;
                                                      char toSend;

                                                      int fpbfd = open(filepathbuffer, O_RDONLY);
                                                      for(i = 0; i < fl; i++){
                                                            read(fpbfd, &toSend, 1);
                                                            //printf("SENT : |%c|\n", toSend);
                                                            write(clientfd, &toSend, 1);
                                                      }//For
                                                      close(fpbfd);
                                                      chdir(projectName);
                            }//WHILE READBYTES

chdir("..");

 }//CHECKOUT

//-----------------------------------------------------------------------------------------| UPGRADE

void upgrade(char* projectName, int clientfd){

  char upop;
//printf("Entered Upgrade On Server End\n");
   while(1){
        char filesend[256] = "";
        bzero(filesend, 256);
        read(clientfd, &upop, 1);//READS THE OPERATION EACH TIME

        if(upop == 'A' || upop == 'M'){ //------------------------------------------------------------------------
                //printf("%c\n", upop);
                sendManifest(projectName, clientfd);
                read(clientfd, filesend, 256);

                if(upop == 'A'){
                    printf("Client Requested File '%s' To Add (During Upgrade)\n", filesend);
                }
                else if(upop == 'M'){
                    printf("Client Requested File '%s' To Modify (During Upgrade)\n", filesend);
                }

                chdir(projectName);
                int filesbeingsent = 1;
                int filelength = getfilelength(filesend); //File Length
                int namelength = strlen(filesend); //File Name Length
                    write(clientfd, &filesbeingsent, sizeof(filesbeingsent));//Sending # Files Being Sent
                    write(clientfd, &namelength, sizeof(namelength));//Sending # Bytes In File Name
                    write(clientfd, filesend, namelength);//Sending File Name
                    write(clientfd, &filelength, sizeof(filelength));//Sending File Length

                    int i = 0;
                    char toSend;
                    int fd = open(filesend, O_RDONLY);
                              if(fd == -1){
                                   printf("Error Upgrading - Could Not Find File %s To Add!\n", filesend);
                              }
                              for(i = 0; i < filelength; i++){
                                    read(fd, &toSend, 1);
                                    //printf("TS : %c\n", toSend);
                                    write(clientfd, &toSend, 1);
                              }//For
                    close(fd);
                    chdir("..");
        }

        if(upop == 'D'){ //------------------------------------------------------------------------
                //printf("D\n");
        }

        if(upop == 'V'){
                //printf("V\n");
                printf("Client Completed Upgrade.\n");
                break;
        }
   }//WHILE

}//UPGRADE

//------------------------------------------------------------------------------------------------------------------------ | DOWNLOAD FILES

void download_files(int clientfd){

/*
          For each file being sent from server, client recieves...

          # Files
          ____
          |    # Bytes in file name
          |    File name
          |    # Bytes in file
          |___      ---> Then send bytes of file one character at a time.

          Repeat Bracket For Each File.
*/

  char buffer[256];
  int i = 0, j = 0, k = 0;

  int numfiles = 0;
  read(clientfd, &numfiles, sizeof(numfiles));
  //printf("Number Of Files Downloading : %d\n", numfiles);

  for(i = 0; i < numfiles; i++){//LOOP FOR EACH FILE BEING DOWNLOADED
      int filenamebytes;
      char* filename;
      int filebytes;
      int fd;

      read(clientfd, &filenamebytes, sizeof(int));
      filename = malloc((sizeof(char) * filenamebytes) + 1);
      read(clientfd, filename, filenamebytes);
      filename[filenamebytes] = '\0';
      read(clientfd, &filebytes, sizeof(int));

                      if(operation == 'p'){//PUSH ---> NEED TO MAKE DIRECTORIES WHEN DOWNLOADING FILES!!
                              int i = 0; int slashcount = 0; int lastslash = -1;
                              while(i < filenamebytes - 1){
                                  if(filename[i] == '/'){
                                      lastslash = i;
                                      slashcount++;
                                  }
                                  //printf("|%c|\n", filename[i]);
                                  i++;
                          }//WHILE
                          //printf("File '%s' Has %d Slashes In Its Name (With The Last One At %d)\n", filename, slashcount, lastslash);

                          int max = 0;

                          if(slashcount > max){//Directories need to be made!
                               i = 0;
                               int slsh2 = 0;
                               char create[256] = "";
                               char projName[256] = "";

                                     while(i <= lastslash){
                                             if(filename[i] == '/'){
                                                    if(slsh2 == 1){
                                                        //printf(" | Project Name : %s\n", projName);
                                                        chdir(projName);
                                                    }
                                                    if(slsh2 >= max){
                                                        mkdir(create, 0777);
                                                        chdir(create);
                                                        //printf("Dir To Create : %s\n", create);
                                                        bzero(create, 256);
                                                    }
                                                    slsh2++;
                                             }
                                            else if(slsh2 >= max){
                                                  strncat(create, &filename[i], 1);
                                            }
                                            if(slsh2 < max){
                                                  printf("%c", filename[i]);
                                                  strncat(projName, &filename[i], 1);
                                            }
                                            i++;
                                     }//WHILE

                                      int looptil = slashcount - 1;
                                      if(slashcount == 1){
                                          looptil = 1;
                                      }

                                     for(i = 0; i < looptil; i++){
                                        chdir("..");
                                     }
                          }
                      }//PUSH

      //printf("Filename : %s\n", filename);
      fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
      if(fd == -1){
        //printf("\tCould Not Open %s!\n", filename);
      }
      char recieved;

              for(j = 0; j < filebytes; j++){//LOOP FOR EACH BYTE IN FILE CURRENTLY BEING PROCESSED
                  read(clientfd, &recieved, 1);
                  //printf("Recieved Byte : |%c|\n", recieved);
                  write(fd, &recieved, 1);
              }//LOOP FOR EACH BYTE IN FILE CURRENTLY BEING PROCESSED

      //printf("\tDownloaded '%s'\n", filename);
      close(fd);
      free(filename);

  }//LOOP FOR EACH FILE BEING DOWNLOADED

}//download_files

//-----------------------------------------------------------------------------------------| Push

void push(char* projectName, int clientfd){

        chdir(projectName);

        int status = 1;
        int fd = open(".Commit", O_RDONLY);
        if(fd == -1){
            write(clientfd, &status, sizeof(status)); //WRITE 1
            printf("The Server Could Not Execute A Push Because The Server Did Not Have A Commit File To Compare To The Client With.\n");
            close(fd);
            chdir("..");
            return;
        }
        else{
            status = 0;
            write(clientfd, &status, sizeof(status)); //WRITE 0
        }
        close(fd);

        status = 1;
        read(clientfd, &status, sizeof(status));
        if(status == 1){//NO COMMIT ON CLIENT
                printf("The Server Could Not Execute A Push Because The Client Did Not Have A Commit File To Send.\n");
                chdir("..");
                return;

        }
        else{//COMMIT ON CLIENT
                printf("(Downloading The Clients .Commit File...)\n");
                download_files(clientfd);//DOWNLOAD .COMMITCOMPARE
        }

        int commitcomparelen = getfilelength(".CommitCompare");
        int commitlen = getfilelength(".Commit");

        //CHECK IF FILES ARE THE Same
        if(commitcomparelen == commitlen){//SAME LENGTH
              char c;
              char cc;
              int i;

                  int commitcompare = open(".CommitCompare", O_RDONLY);
                  int commit = open(".Commit", O_RDONLY);

                        for(i = 0; i < commitlen; i++){
                              read(commit, &c, 1);
                              read(commitcompare, &cc, 1);
                                    if(c != cc){//Letter is NOT the same
                                            printf("The Commit That The Server Had Was Not The Same As The One The Client Sent! Could Not Push.\n");
                                            close(commitcompare);
                                            close(commit);
                                            status = 1;
                                            write(clientfd, &status, sizeof(status)); //WRITE 1
                                            remove(".CommitCompare");
                                            chdir("..");
                                            return;
                                    }
                        }//FOR, IF IT PASSES THIS THE FILES ARE THE Same

                        status = 0;
                        write(clientfd, &status, sizeof(status)); //WRITE 0 TO CONTINUE
                        close(commitcompare);
                        close(commit);
                        remove(".CommitCompare");
              //printf("The Commit That The Server Had WAS The Same As The One The Client Sent!\n");

        }//IF
        else{//DIFF FILE LENGTHS
              printf("The Commit That The Server Had Was Not The Same As The One The Client Sent! Could Not Push.\n");
              status = 1;
              write(clientfd, &status, sizeof(status)); //WRITE 1

                  remove(".CommitCompare");
              chdir("..");
              return;
        }

        //AT THIS POINT, BOTH COMMIT AND COMMITCOMPARE ARE THE SAME. --------------------------------------------------------------------------------------------------------------------------------
        //GET PROJECT VERSION AND PUT OLD PROJECT IN A BACKUP;

        fd = open(".Manifest", O_RDONLY);
                char observed;
                char serverversion[16] = "";
                read(fd, &observed, 1);
                            while(observed != ' ' && observed != '\n'){
                                  strncat(serverversion, &observed, 1);
                                  //printf("READ : %c\n", observed);
                                  read(fd, &observed, 1);
                            }
                            //printf("SERVER VERSION : %s\n", serverversion);
        close(fd);

                          char command[1024] = "cp -r ./";
                                strcat(command, projectName);
                                strcat(command, "/. ./");
                                strcat(command, projectName);
                                strcat(command, ":");
                                strcat(command, serverversion);
                                strcat(command, "/");

                            char command2[1024] = "mv ./"; // mv ./x:0 ./.Backups/x/x:0
                                strcat(command2, projectName);
                                strcat(command2, ":");
                                strcat(command2, serverversion);
                                strcat(command2, " ./.Backups/");
                                strcat(command2, projectName);
                                strcat(command2, "/");
                                strcat(command2, projectName);
                                strcat(command2, ":");
                                strcat(command2, serverversion);

                                //printf("Command1 For Terminal : '%s'\n", command);
                                //printf("Command2 For Terminal : '%s'\n", command2);
                                    chdir("..");
                                    chdir(".Backups");
                                    mkdir(projectName, 0777);
                                    chdir("..");
                                    system(command);//MAKES THE BACKUP, ENDS AT SERVER DIR
                                    system(command2);//MOVES BACKUP TO BACKUP FOLDER
                                chdir(projectName);//MOVE FROM SERVER DIR BACK TO PROJECT DIR

        //NOW INCREMENT MANIFEST FILE VERSION : --------------------------------------------------------------------------------------------------------------------------------------------
        fd = open(".Manifest", O_WRONLY);
            write(fd, "           ", 11);
        close(fd);

                  fd = open(".Manifest", O_WRONLY);
                      int ver_int = atoi(serverversion);
                      ver_int++;
                      char strincrem[16];
                      snprintf(strincrem, 16, "%d", ver_int);
                      //printf("STRING FOR FILE INCREMEMNT = %s\n", strincrem);
                      write(fd, strincrem, strlen(strincrem));//WRITE INCREMENT TO MANIFEST
                  close(fd);

        //SEND CLIENT THE VERSION NUM
        write(clientfd, &ver_int, sizeof(ver_int));//SEND THE CLIENT THE INCREMENT NUMBER

        //NOW WE CAN ACTUALLY GO THROUGH THE .COMMIT AND DO THE CHANGES!!!! -------------------------------------------------------------------------------------------------------------------------------

          fd = open(".Commit", O_RDONLY);
                          int readbytes = 0;
                          while(readbytes < commitlen){
                                char commitoperation = 'x';
                                char obs = 'x';

                                char filepath[256] = "";
                                char hash[33] = "";
                                char versionbuff[16] = "";
                                    bzero(filepath, 256);
                                    bzero(hash, 33);
                                    bzero(versionbuff, 16);

                                            //GETS THE OPERATION, PATH, AND HASH FROM .COMMIT
                                            read(fd, &commitoperation, 1);//GET OPERATION
                                            readbytes++;

                                            //ACTUAL UPDATE REACHED --------------------------------------------------------------------------------------------------------------
                                                  int i = 0;
                                                  for(i = 0; i < (4 + strlen(projectName)); i++){
                                                      read(fd, &obs, 1);//SKIP SPACE + PROJECT NAME
                                                      readbytes++;
                                                  }

                                                  obs = 'x';//RESET OBSERVED
                                                  while(obs != ' '){//FILE PATH
                                                        read(fd, &obs, 1);
                                                        readbytes++;
                                                        if(obs != ' '){
                                                            strncat(filepath, &obs, 1);
                                                        }
                                                  }
                                                  obs = 'x';//RESET OBSERVED
                                                  while(obs != ' '){//HASH
                                                        read(fd, &obs, 1);
                                                        readbytes++;
                                                        if(obs != ' '){
                                                            strncat(hash, &obs, 1);
                                                        }
                                                  }//Hash

                                                  obs = 'x';//RESET OBSERVED
                                                  while(obs != '\n'){//VERSION FOR FILE
                                                        read(fd, &obs, 1);
                                                        readbytes++;
                                                        if(obs != '\n'){
                                                            strncat(versionbuff, &obs, 1);
                                                        }
                                                  }//Hash
/*
                                                  printf("\tOPERATION : |%c|\n", commitoperation);
                                                  printf("\tFILE PATH/NAME : |%s|\n", filepath);
                                                  printf("\tHASH : |%s|\n", hash);
                                                  printf("\tVersion Num : |%s|\n", versionbuff);
                                                  printf("\tREADBYTES / UPDATE LENGTH = %d / %d\n\n", readbytes, commitlen);
*/

                write(clientfd, &commitoperation, 1);
                //------------------------------------- PARSE DIFFERENT MODES
                      if(commitoperation == 'A'){//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------
                            //  printf("A\n");
                            printf("\tAdded '%s'\n", filepath);

                                            //ADD FILE TO SERVER MANIFEST
                                            int mfd = open(".Manifest", O_WRONLY | O_APPEND, 0644);
                                                  write(mfd, "\nX\n", 3);
                                                  write(mfd, "./", 2);
                                                  write(mfd, projectName, strlen(projectName));
                                                  write(mfd, "/", 1);
                                                  write(mfd, filepath, strlen(filepath));
                                                  write(mfd, "\n", 1);
                                                  write(mfd, hash, strlen(hash)); //Create hash function
                                                  write(mfd, "\n", 1);
                                                  write(mfd, versionbuff, strlen(versionbuff));
                                                  write(mfd, "\n", 1);
                                            close(mfd);

                              //NOW CAN ADD FILE FROM THE SERVER;
                              write(clientfd, filepath, strlen(filepath));//SEND FILE NAME
                              download_files(clientfd);//DOWNLOADS A FILE WITH THE FILEPATH GIVEN

                      }//ADD

                      else if(commitoperation == 'M'){//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
                            //  printf("M\n");
                            printf("\tModified '%s'\n", filepath);
                                                      int servermanifestlength = getfilelength(".Manifest");
                                                      char* servermanifestbuffer = malloc(sizeof(char) * servermanifestlength);
                                                            int smbfd = open(".Manifest", O_RDONLY);
                                                                  read(smbfd, servermanifestbuffer, servermanifestlength);
                                                            close(smbfd);

                                                            char* ptr = strstr(servermanifestbuffer, filepath) + strlen(filepath) + 1;//PASS /N AND NAME
                                                                      int i = 0;
                                                                      for(i = 0; i < 32; i++){
                                                                            ptr[i] = hash[i];
                                                                      }//FOR
                                                                      ptr = ptr + 33;

                                                                      char clifv[16] = "";
                                                                      while(ptr[0] != '\n'){
                                                                        strncat(clifv, &ptr[0], 1);
                                                                        ptr = ptr + 1;
                                                                      }
                                                                      //printf("SERVER FILE VER = %s\n", clifv);
                                                                      //ptr = ptr - strlen(clifv);
                                                                      //printf("PTR LOCATION = |%s|\n", ptr);

                                                            //WRITE THE HASH IN PUSH TO THE .MANIFEST
                                                            smbfd = open(".Manifest", O_CREAT | O_WRONLY | O_TRUNC, 0644);
                                                                  write(smbfd, servermanifestbuffer, strlen(servermanifestbuffer));
                                                            close(smbfd);

                                                            //GRAB INDEX OF THE START OF THE FILE VERSION SO NO OVERWRITE HAPPENS
                                                            int index = ptr ? ptr - servermanifestbuffer : -1; //THIS CLEARS MANIFEST BUFFER SO WE NEED TO READ IT AGAIN
                                                            index = index - strlen(clifv);
                                                                  bzero(servermanifestbuffer, servermanifestlength);
                                                                      smbfd = open(".Manifest", O_RDONLY);
                                                                      read(smbfd, servermanifestbuffer, servermanifestlength);
                                                                      close(smbfd);

                                                            smbfd = open(".Manifest", O_CREAT | O_WRONLY | O_TRUNC, 0644);
                                                                  write(smbfd, servermanifestbuffer, index);
                                                                  //printf("WROTE THE FIRST %d bytes of man buff\n", index);
                                                            close(smbfd);

                                                            smbfd = open(".Manifest", O_CREAT | O_WRONLY | O_APPEND, 0644);
                                                                  write(smbfd, versionbuff, strlen(versionbuff));
                                                                  write(smbfd, "\n", 1);
                                                                  write(smbfd, ptr, (strlen(servermanifestbuffer) - index - strlen(versionbuff) - 1));
                                                                  //printf("WROTE THE FILE VERSION\n");
                                                            close(smbfd);

                                                      free(servermanifestbuffer);

                                                      //printf("DOWNLOADING %s\n", filepath);
                                                      //NOW CAN ADD FILE FROM THE SERVER;
                                                      write(clientfd, filepath, strlen(filepath));
                                                      download_files(clientfd);//DOWNLOADS A FILE WITH THE FILEPATH GIVEN

                      }//MODIFY

                      else if(commitoperation == 'D'){//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
                            //  printf("D\n");
                            printf("\tDeleted '%s'\n", filepath);

                                      //PUT SERVER MANIFEST INTO A BUFFER ---> GOING TO SEW PARTS AROUND THE ENTRY INTO A NEW MANIFEST TO SAVE
                                      int servermanifestlength = getfilelength(".Manifest");
                                      char* servermanifestbuffer = malloc(sizeof(char) * servermanifestlength);
                                      int smbfd = open(".Manifest", O_RDONLY);
                                      read(smbfd, servermanifestbuffer, servermanifestlength);
                                      close(smbfd);

                                      char* start = strstr(servermanifestbuffer, filepath) - 3 - strlen(projectName) - 3;// PTR TO GIVES START OF ENTRY
                                      char* end = start;
                                      int index = start ? start - servermanifestbuffer : -1; //THIS CLEARS MANIFEST BUFFER SO WE NEED TO READ IT AGAIN
                                            bzero(servermanifestbuffer, servermanifestlength);
                                                smbfd = open(".Manifest", O_RDONLY);
                                                read(smbfd, servermanifestbuffer, servermanifestlength);
                                                close(smbfd);

                                      start = strstr(servermanifestbuffer, filepath) - 3 - strlen(projectName) - 3;
                                      end = start;

                                    //  printf("START : \n|%s|\n", start);
                                      //printf("INDEX OF START = %d\n", index);
                                                int newlines = 0;
                                                int read = 0;
                                                while(newlines != 5){
                                                      if(end[0] == '\n'){
                                                          newlines++;
                                                      }
                                                      end = end + 1;
                                                      read++;
                                                }

                                      //printf("END Is %d Bytes Ahead of start : \n|%s|\n", read, end);
                                      char* newbuff = malloc(sizeof(char) * servermanifestlength);
                                      bzero(newbuff,servermanifestlength);
                                      strncpy(newbuff, servermanifestbuffer, index);
                                      strncat(newbuff, (start + read), (servermanifestlength - index - read));

                                      //printf("NEW MANIFEST WITHOUT %s : \n|%s|\n", filepath, newbuff);
                                      smbfd = open(".Manifest", O_CREAT | O_WRONLY | O_TRUNC, 0644);
                                          write(smbfd, newbuff, strlen(newbuff));
                                      close(smbfd);

                                      remove(filepath);
                                      free(newbuff);
                                      free(servermanifestbuffer);
                      }//DELETE

                }//WHILE

        close(fd);//.Commit File CLose
        char complete = 'C';
        write(clientfd, &complete, 1);//WHEN CLIENT GETS C, WE ARE COMPLETE

        //NOW MAKE THE .HISTORY FILE
        int hfd = open(".History", O_CREAT | O_WRONLY | O_APPEND, 0644);
        int chfd = open(".Commit", O_RDONLY);

                  int d = 0; char histchar;
                  for(d = 0; d < commitlen; d++){//APPEND COMMIT STUFF TO .HISTORY
                        read(chfd, &histchar, 1);
                        write(hfd, &histchar, 1);
                  }
        close(chfd);

        write(hfd, serverversion, strlen(serverversion));
        write(hfd, "\n\n", 2);
        close(hfd);

        //END OF CHANGES!!! ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
        remove(".Commit");
        printf("Completed Push.\n");
        chdir("..");
        sendManifest(projectName, clientfd);//UPDATE CLIENT MANIFEST

}//PUSH

//-----------------------------------------------------------------------------------------| HISTORY

void history(char* projectName, int clientfd){

    chdir(projectName);

    int response = 0;
    int hfd = open(".History", O_RDONLY);
    if(hfd == -1){
          printf("A Commit History File Does Not Exist Yet In This Project.\n");
          close(hfd);
          response = 1;//COULD NOT DO HISTORY
          write(clientfd, &response, sizeof(response));
          chdir("..");
          return;
    }

    else{
          response = 0;//COULD DO HISTORY
          write(clientfd, &response, sizeof(response));
          printf("Sending The Commit History...\n");
    }
    close(hfd);

                //SEND HISTORY FILE
                int filesbeingsent = 1;
                int fl = getfilelength(".History"); //File Length
                int fnl = strlen(".History"); //File Name Length
                    write(clientfd, &filesbeingsent, sizeof(filesbeingsent));//Sending # Files Being Sent
                    write(clientfd, &fnl, sizeof(fnl));//Sending # Bytes In File Name
                    write(clientfd, ".History", fnl);//Sending File Name
                    write(clientfd, &fl, sizeof(fl));//Sending File Length

                int i = 0;
                char toSend;
                hfd = open(".History", O_RDONLY);
                          if(hfd == -1){
                               printf(".History Could Not Be Found??? Error Writing To It!\n");
                          }
                for(i = 0; i < fl; i++){
                      read(hfd, &toSend, 1);
                      //printf("TS : %c\n", toSend);
                      write(clientfd, &toSend, 1);
                }//For
                close(hfd);

    chdir("..");
}//HISTORY

//----------------------------------------------------------------------------------------------: rollback

void rollback(char* projectName, char* backupname, char* rollbackversion, int clientfd){

    int status = chdir(backupname);
    if(status == 0){//THE ROLLBACK VERSION EXISTS
          printf("Rolling Back To Version %s Of %s\n", rollbackversion, projectName);
          write(clientfd, "0", 2);
    }
    else{//ROLLBACK VERSION DOES NOT Exist
          printf("Version %s Of %s Does Not Exist. Could Not Roll Back.\n", rollbackversion, projectName);
          write(clientfd, "1", 2);
          return;
    }

    chdir("..");//PROJECT NAME:VERNUM
    chdir("..");//PROJECT NAME
    chdir("..");//.Backups

    //NOW WE ARE IN SERVER DIR
    destroy(projectName);//DESTORY THE PROJECT

            //MOVE BACKUP TO SERVER FOLDER
            char command[1024] = "mv "; // mv ./.Backups/x/x:0 ./.
                strcat(command, backupname);
                strcat(command, " ./.");

            //RENAME THE BACKUP TO ORIGINAL PROJECT NAME
            char command2[1024] = "mv ./";// mv x:0 x
                  strcat(command2, projectName);
                  strcat(command2, ":");
                  strcat(command2, rollbackversion);
                  strcat(command2, "/ ./");
                  strcat(command2, projectName);
                  strcat(command2, "/");

                  //printf("command 1 = %s\n", command);
                  //printf("command 2 = %s\n", command2);

                  system(command);
                  system(command2);

    //NOW DESTROY ALL PROJECTS THAT HAVE GREATER FILE VERSION
        //chdir(".Backups");
        //chdir(projectName);

    //NOW IN 'X' ---> REMOVE ALL FILES THAT DONT HAVE GREATER VERSION
    //ALL FILES IN THIS DIR ARE BACKUPS
        //int versionint = atoi(rollbackversion);
        //printf("Destroying all backups greater than %d.\n", versionint);

        printf("Completed Rollback.\n");

}//ROLLBACK

//-----------------------------------------------------------------------------------------| ATEXIT FUNCTION TO CLOSE SOCKET AND manifestbuffer

void quitfunc(){
    close(clientfd);
    close(socketfd);
    printf("Server Shut Down Successfully.\n");
    printf("---------------------------------------------------------------------\n");
}
