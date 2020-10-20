#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#include <openssl/md5.h>

//WTF CLIENT
/*
  COMMANDS TO IMPLEMENT ('-' next to func = completed         '>' next to func = being worked on)
      - Configure
      - Create
      - Destroy
      - ADD
      - Remove
      - Currentversion
      - Checkout
      - Update
      - Upgrade
      - Commit
      > Push
      > History
      Rollback

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
      i = Upgrade
      j = Rollback
      o = Commit
      p = Push

  TERMS...
      Project: directory where collection of code resides
      Repository: directory where all projects are located, contains all copies of all projects being managed as well as backups
      .Manifest: metadata file with all paths to all files in project, their state, a hash of all contents of each file and version
      history: a list of all updates made to a given project
      roll back: change version of project to previous
      commit/push: upload changes and updating project to new version
      check out: download current version of a project in repo
      update: download from repo only files in a project newer than client's

      Multi-threading?
      WTFtest - use system calls to execute test cases for each command, makefile will build this application


*/




//-----------------------------------------------------------------------------------------| FUNCTION DELCARATIONS

void configure(char* ip, char* port);
void checkout(char* projectName, int socketfd);
void create(char* projectName);
void destroy(char* projectName); //only affects server side
void add(char* projectName, char* fileName);
void removefl(char* projectName, char* fileName);
void readcurrentversion(int socketfd);
void history(char* projectName, int socketfd);
void rollback(char* projectName, char* version);
void download_files(int socketfd);
void push(char* projectName, int socketfd);
void commit(char* projectName, int socketfd);
char* makehash(char* filepath);
void update(char* projectName, int socketfd);
int getfilelength(char* filepath);
void upgrade(char* projectName, int socketfd);

//------------------------------------------------------------------------------------| GLOBALS

int checkConfig = 0;
char operation = 'x';

//-----------------------------------------------------------------------------------------| MAIN

int main(int argc, char** argv){
  //printf("WTF Client Started\n\n");

//------------------------------------------------| CLIENT SETUP / CREATION

int socketfd = 0;
int fd;
char* ip;
char* port;
int wr;
char* projectName;
char* fileName;
char* rollbackversion;
struct sockaddr_in serv_addr;
struct hostent *server;
operation = 'x';

//------------------------------------------------| MAKEING SENSE OF ARGUMENTS

  if(argc == 1){
        printf("No Commands Were Given To The Client!\n");
        return EXIT_FAILURE;
  }

  if(strcmp(argv[1], "configure") == 0){

                if(argc <= 3){
                    printf("Configure Arguments Incorrectly Supplied. (./WTF configure <IP/HOST> <PORT NUMBER>)\n");
                    return;
                }

                if(argc == 4){
                    ip = argv[2];
                    port = argv[3];
                    configure(ip, port);
                    printf("Client Was Succesfully Configured.\n");
                    return;
                }
                else{
                    printf("Arguments For Configure Are Not Properly Given. Terminating Program.\n");
                    return EXIT_FAILURE;
                }
      }//IF
              else{
                  fd = open(".configure", O_RDONLY);
                      if(fd == -1){
                          printf("No '.configure' File Is Present! Perhaps You Should Create It With './WTF configure <IP/HOST> <PORT NUMBER>'!\n");
                          return EXIT_FAILURE;
                      }
                      else{
                          //------------------------------------------------| GET IP FROM .configure FILE
                          //Potential IP Adresses Can BE "XXX.XXX.XXX.XXX" So I Allocated 16 Spots For The Characters To File
                          //Same Goes For Potential Port - "XXXXX"
                          int i = 0;
                          char* k = malloc(sizeof(char) * 2);
                          char tempIP[16] = "";
                              for(i = 0; i < 16; i++){
                                  read(fd, k, 1);
                                  k[1] = '\0';
                                  //printf("K : %s\n", k);
                                  if(k[0] == '\n'){break;}
                                  strcat(tempIP, k);
                              }
                              tempIP[i + 1] = '\0';
                          ip = tempIP;
                          //printf("IP ADDRESS : %s\n", ip);

                          char tempPort[6] = "";
                              for(i = 0; i < 6; i++){
                                  read(fd, k, 1);
                                  if(k[0] == '\n'){break;}
                                  strcat(tempPort, k);
                              }
                              tempPort[i + 1] = '\0';
                          port = tempPort;
                          //printf("PORT NUMBER : %s\n", port);
                          close(fd);
                          free(k);
                      }//ELSE
  }//CONFIGURE

  if(strcmp(argv[1], "checkout") == 0) {
        operation = 'g';
        if(argc <= 2){
            printf("Checkout Arguments Incorrectly Supplied. (./WTF checkout <PROJECT NAME>)\n");
            return EXIT_FAILURE;
        }
        int i = 2;
        char fullprojname[256] = "";
        for(i = 2; i < argc; i++){
            strcat(fullprojname, argv[i]);
            if(i != (argc - 1)){
                strcat(fullprojname, " ");
            }
        }
      projectName = fullprojname;

  } //CHECKOUT

  else if (strcmp(argv[1], "create") == 0) {
      operation = 'a';
      if(argc <= 2){
          printf("Create Arguments Incorrectly Supplied. (./WTF create <PROJECT NAME>)\n");
          return EXIT_FAILURE;
      }
      int i = 2;
      char fullprojname[256] = "";
      for(i = 2; i < argc; i++){
          strcat(fullprojname, argv[i]);
          if(i != (argc - 1)){
              strcat(fullprojname, " ");
          }
      }
    projectName = fullprojname;
  } //CREATE

  else if (strcmp(argv[1], "destroy") == 0) {
    operation = 'b';
    if(argc <= 2){
        printf("Destroy Arguments Incorrectly Supplied. (./WTF destroy <PROJECT NAME>)\n");
        return EXIT_FAILURE;
    }
    char fullprojname[256] = "";
    int i = 2;
    for(i = 2; i < argc; i++){
        strcat(fullprojname, argv[i]);
        if(i != (argc - 1)){
            strcat(fullprojname, " ");
        }
    }
  projectName = fullprojname;
  } //DESTROY

  else if (strcmp(argv[1], "commit") == 0) {
    operation = 'o';
    if(argc <= 2){
        printf("Commit Arguments Incorrectly Supplied.(./WTF commit <PROJECT NAME>)\n");
        return EXIT_FAILURE;
    }
    char fullprojname[256] = "";
    int i = 2;
    for (i = 2; i< argc; i++) {
      strcat(fullprojname, argv[i]);
      if (i!=(argc - 1)) {
        strcat(fullprojname, " ");
      }
    }
    projectName = fullprojname;
  }

  else if (strcmp(argv[1], "currentversion") == 0) {
      operation = 'e';
      if(argc <= 2){
          printf("Current Version Arguments Incorrectly Supplied.(./WTF currentversion <PROJECT NAME>)\n");
          return EXIT_FAILURE;
      }
      char fullprojname[256] = "";
      int i = 2;
      for(i = 2; i < argc; i++){
          strcat(fullprojname, argv[i]);
          if(i != (argc - 1)){
              strcat(fullprojname, " ");
          }
      }
    projectName = fullprojname;
  } //CURRENTVERSION

  else if (strcmp(argv[1], "add") == 0) {
      operation = 'c';
      if(argc <= 3){
          printf("Add Checkout Arguments Incorrectly Supplied. (./WTF add <PROJECT NAME> <FILE NAME>)\n");
          return EXIT_FAILURE;
      }
          char fullprojname[256] = "";
          int i = 2;
          for(i = 2; i < argc-1; i++){
              strcat(fullprojname, argv[i]);
              if(i != (argc - 2)){
                  strcat(fullprojname, " ");
              }
          }
    projectName = fullprojname;
    fileName = argv[argc-1];
    if(fileName[0] == '.' && fileName[1] == '/'){ //IF THE FILE SUPPLIED BEGINS WITH A DIR, LIKE "./xxxxxxxx", REMOVE THE "./"
          char* substr = fileName + 2;
          strcpy(fileName, substr);
    }
  } //ADD

  else if(strcmp(argv[1], "remove") == 0) {
      operation = 'd';
      if(argc <= 3){
          printf("Remove Arguments Incorrectly Supplied. (./WTF remove <PROJECT NAME> <FILE NAME>)\n");
          return EXIT_FAILURE;
      }
          char fullprojname[256] = "";
          int i = 2;
          for(i = 2; i < argc-1; i++){
              strcat(fullprojname, argv[i]);
              if(i != (argc - 2)){
                  strcat(fullprojname, " ");
              }
          }
    projectName = fullprojname;
    fileName = argv[argc-1];
    if(fileName[0] == '.' && fileName[1] == '/'){ //IF THE FILE SUPPLIED BEGINS WITH A DIR, LIKE "./xxxxxxxx", REMOVE THE "./"
          char* substr = fileName + 2;
          strcpy(fileName, substr);
    }

    //printf("FILE NAME : %s\n", fileName);
  } //REMOVE

  else if (strcmp(argv[1], "history") == 0) {
    operation = 'h';
    if (argc <= 2) {
      printf("History Arguments Incorrectly Supplied. (./WTF history <PROJECT NAME>)\n");
      return EXIT_FAILURE;
    }
    int i = 2;
    char fullprojname[256] = "";
    for(i = 2; i < argc; i++){
        strcat(fullprojname, argv[i]);
        if(i != (argc - 1)){
            strcat(fullprojname, " ");
        }
    }
  projectName = fullprojname;
  }//HISTORY

  else if (strcmp(argv[1], "update") == 0) {//UPDATE
    operation = 'f';
        if(argc <= 2) {
          printf("Update Arguments Incorrectly Supplied. (./WTF update <PROJECT NAME>)\n");
          return EXIT_FAILURE;
        }
        int i = 2;
        char fullprojname[256] = "";
        for(i = 2; i < argc; i++){
            strcat(fullprojname, argv[i]);
            if(i != (argc - 1)){
                strcat(fullprojname, " ");
            }
        }
  //printf("Project Name = %s\n", fullprojname);
  projectName = fullprojname;
  }//UPDATE

  else if (strcmp(argv[1], "upgrade") == 0) {//UPGRADE
    operation = 'i';
        if(argc <= 2) {
          printf("Upgrade Arguments Incorrectly Supplied. (./WTF upgrade <PROJECT NAME>)\n");
          return EXIT_FAILURE;
        }
        int i = 2;
        char fullprojname[256] = "";
        for(i = 2; i < argc; i++){
            strcat(fullprojname, argv[i]);
            if(i != (argc - 1)){
                strcat(fullprojname, " ");
            }
        }
  //printf("Project Name = %s\n", fullprojname);
  projectName = fullprojname;
  }//UPGRADE

  else if(strcmp(argv[1], "rollback") == 0){//ROLLBACK
      operation = 'j';
      if(argc <= 3) {
        printf("Rollback Arguments Incorrectly Supplied. (./WTF rollback <PROJECT NAME> <VERSION NUMBER>)\n");
        return EXIT_FAILURE;
      }

      projectName = argv[2];
      rollbackversion = argv[3];
      int i = 0;
      for(i = 0; i < strlen(rollbackversion); i++){
          //printf("Rollback[%d] = %c\n", i, rollbackversion[i]);
          if((rollbackversion[i] != '1') &&
              (rollbackversion[i] != '2') &&
              (rollbackversion[i] != '3') &&
              (rollbackversion[i] != '4') &&
              (rollbackversion[i] != '5') &&
              (rollbackversion[i] != '6') &&
              (rollbackversion[i] != '7') &&
              (rollbackversion[i] != '8') &&
              (rollbackversion[i] != '9') &&
              (rollbackversion[i] != '0')){//IF
                  printf("The Version Number For Rollback Contains Illegal Characters (Not 0-9). Enter A Real Version Number.\n");
                  return EXIT_FAILURE;
          }//IF
      }//FOR
      //printf("PROJECT NAME : %s\n", projectName);
      //printf("ROLLBACK VERSION NUM : %s\n", rollbackversion);
  }//ROLLBACK

  else if (strcmp(argv[1], "push") == 0) {//PUSH
    operation = 'p';
        if(argc <= 2) {
          printf("Upgrade Arguments Incorrectly Supplied. (./WTF push <PROJECT NAME>)\n");
          return EXIT_FAILURE;
        }
        int i = 2;
        char fullprojname[256] = "";
        for(i = 2; i < argc; i++){
            strcat(fullprojname, argv[i]);
            if(i != (argc - 1)){
                strcat(fullprojname, " ");
            }
        }
  //printf("Project Name = %s\n", fullprojname);
  projectName = fullprojname;
}//PUSH

  else{
        printf("No Proper Operation Was Supplied!\n");
        return EXIT_FAILURE;
  }

//------------------------------------------------| SERVER SETUP AND CONNECTION

  socketfd = socket(AF_INET, SOCK_STREAM, 0);
     if(socketfd < 0){  //Error checking if no socket
        printf("Error Opening Socket");
        return EXIT_FAILURE;
     }

  server = gethostbyname(ip);
     if(server == NULL){ //Error chcking if IP is invalid
        printf("IP Error (Host N/A)\n");
        return EXIT_FAILURE;
     }

  bzero((char *) &serv_addr, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(atoi(port));

     //Now we can connect to server
  int connected = 1; //Connect is positive if it connects

     while(connected != 0){
       if(connected = connect(socketfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
          printf("Error Connecting To Server\n");
          printf("Attemping To Reconnect In 3 Seconds...\n");

          sleep(3);
       }
       else{
                printf("\n| CONNECTED TO SERVER |\n");
       }

     }//While connected < 0

     mkdir("./Client/", 0777); //Will fail (= -1) in most cases as server folder will probably be present. Mainly to be succeessful on the first occasion of server being ran... just in case
     chdir("./Client/");

//------------------------------------------------| After operation character is sent, server will request next info

  //printf("Operation : %c\n", operation);
  char servermessage[2] = "";
  char serverprint[256];

     //SEND MESSAGE
     wr = write(socketfd, &operation, 1);
     if(wr < 0){ printf("ERROR Writing To Socket"); return EXIT_FAILURE;}

     //------------------------------------------------| AFTER TELLING SERVER THE OPERATION...

     if(operation == 'a'){ //CREATE
       wr = write(socketfd, projectName, strlen(projectName));
       read(socketfd, servermessage, 2);
           if(strcmp(servermessage, "0") == 0){ //SERVER RESPONDED SAYING NEW PROJECT CREATION WAS SUCCESSFUL
                mkdir(projectName, 0777);
                chdir(projectName);
                download_files(socketfd);
                chdir("..");
                printf("A New Project Was Created With The Name '%s'\n", projectName);
           }
           else{
                printf("The Server Already Contains A Project With That Name.\n");
           }
     }//CREATE

                                                                                     if(operation == 'b'){ //DESTROY
                                                                                       wr = write(socketfd, projectName, strlen(projectName));
                                                                                       read(socketfd, servermessage, 2);
                                                                                       if(strcmp(servermessage, "0") == 0) { //SERVER SAYING PROJECT SUCCESSFULLY DESTROYED
                                                                                            read(socketfd, serverprint, 37);
                                                                                            printf("%s", serverprint);
                                                                                       }
                                                                                       else {
                                                                                         printf("Server Does Not Contain Given Project\n");
                                                                                       }
                                                                                     }//DESTROY

     if(operation == 'c'){ //ADD
           read(socketfd, servermessage, 2);
           if(strcmp(servermessage, "0") == 0) { //SERVER SAYING PROJECT SUCCESSFULLY DESTROYED
             add(projectName, fileName);
           }

     }

                                                                                     if(operation == 'd'){ //REMOVE
                                                                                             read(socketfd, servermessage, 2);
                                                                                             if(strcmp(servermessage, "0") == 0) { //SERVER SAYING PROJECT SUCCESSFULLY DESTROYED
                                                                                                   removefl(projectName, fileName);
                                                                                             }
                                                                                     }//REMOVE

     if(operation == 'e'){ //CURRENTVERSION
       wr = write(socketfd, projectName, strlen(projectName));
       wr = read(socketfd, servermessage, 2);
       if(strcmp(servermessage, "0") == 0) { //SERVER SAYING PROJECT SUCCESSFULLY SENT OVER CURRENT VER
            readcurrentversion(socketfd);
            printf("|   END OF CURRENT VERSION REPORT   |\n");
       }
       else{
            printf("Project '%s' Does Not Exist On The Server. Terminating Program.\n", projectName);
       }
     }//CURRENTVERSION

                                                                                if(operation == 'f'){ //UPDATE
                                                                                  int canupgrade = checkIfProjectDirExists(projectName);
                                                                                  wr = write(socketfd, &canupgrade, sizeof(canupgrade));//Write -1 if client DOESNT have copy of proj - Write 0 if it does
                                                                                  //printf("Value Of Cancheck = '%d'\n", cancheckout);

                                                                                  if(canupgrade == 0){//Proj Exists On Client
                                                                                           wr = write(socketfd, projectName, strlen(projectName));
                                                                                           wr = read(socketfd, servermessage, 2);

                                                                                           if(strcmp(servermessage, "0") == 0) { //SERVER SAYING PROJECT EXISTS
                                                                                                  update(projectName, socketfd);
                                                                                           }
                                                                                           else {
                                                                                                   printf("The Server Does Not Contain A Project Named '%s'.\n", projectName);
                                                                                           }

                                                                                  }//IF
                                                                                  else{//Proj Exists On Client
                                                                                           printf("Project '%s' Does Not Exist On The Client! (Could Not Complete An Upgrade)\n", projectName);
                                                                                  }
                                                                                }//UPDATE

      if(operation == 'i'){ //UPGRADE
        int canupgrade = checkIfProjectDirExists(projectName);
        wr = write(socketfd, &canupgrade, sizeof(canupgrade));//Write -1 if client DOESNT have copy of proj - Write 0 if it does
        //printf("Value Of Cancheck = '%d'\n", cancheckout);

        if(canupgrade == 0){//Proj Exists On Client
                 wr = write(socketfd, projectName, strlen(projectName));
                 wr = read(socketfd, servermessage, 2);

                 if(strcmp(servermessage, "0") == 0) { //SERVER SAYING PROJECT EXISTS
                        upgrade(projectName, socketfd);
                 }
                 else {
                         printf("The Server Does Not Contain A Project Named '%s'.\n", projectName);
                 }

        }//IF
        else{//Proj Exists On Client
                 printf("Project '%s' Does Not Exist On The Client! (Could Not Complete An Upgrade)\n", projectName);
        }
      }//UPGRADE

                                                                 if (operation == 'h') { // History
                                                                         wr = write(socketfd, projectName, strlen(projectName));//WRITE PROJ NAME

                                                                         wr = read(socketfd, servermessage, 2);
                                                                         if(strcmp(servermessage, "0") == 0) { //SERVER SAYING PROJECT Exists On Server Side
                                                                                  printf("Requesting Commit History From Server...\n");
                                                                                  history(projectName, socketfd);
                                                                         }
                                                                         else{
                                                                              printf("Project '%s' Does Not Exist On The Server.\n", projectName);
                                                                          }
                                                                 }//HISTOPRY

     if(operation == 'g'){//CHECKOUT
           int cancheckout = checkIfProjectDirExists(projectName);
           wr = write(socketfd, &cancheckout, sizeof(cancheckout));//Write -1 if client DOESNT have copy of proj - Write 0 if it does
           //printf("Value Of Cancheck = '%d'\n", cancheckout);

           if(cancheckout == -1){//Proj Doesnt Exist On Client
                    wr = write(socketfd, projectName, strlen(projectName));
                    wr = read(socketfd, servermessage, 2);
                    if(strcmp(servermessage, "0") == 0) { //SERVER SAYING PROJECT EXISTS
                           checkout(projectName, socketfd);
                           printf("Completed A Checkout Of %s.\n", projectName);
                    }
                    else {
                            printf("The Server Does Not Contain A Project Named '%s'.\n", projectName);
                    }

           }
           else{//Proj Exists On Client
                    printf("Project Already Exist's On Client! (Could Not Complete A Checkout)\n");
           }
     }//CHECKOUT

                                                     if(operation == 'j'){ //ROLLBACK
                                                             write(socketfd, projectName, strlen(projectName));//WRITE PROJ NAME


                                                             read(socketfd, servermessage, 2);
                                                       if(strcmp(servermessage, "0") == 0) { //SERVER SAYING PROJECT Exists On Server Side
                                                            write(socketfd, rollbackversion, strlen(rollbackversion)); //WRITE VERSION

                                                                  //READ IF ROLLBACK CAN HAPPEN OR Not
                                                                  read(socketfd, servermessage, 2);
                                                                  if(strcmp(servermessage, "0") == 0){//SERVER DID ROLLBACK
                                                                          printf("Server Completed Rollback To Version #%s\n", rollbackversion);
                                                                  }
                                                                  else{
                                                                          printf("Server Could Not Complete Rollback To Version #%s, As That Version Does Not Exist!\n", rollbackversion);
                                                                  }


                                                       }
                                                       else{
                                                            printf("Project '%s' Does Not Exist On The Server.\n", projectName);
                                                       }
                                                     }//ROLLBACK

     if(operation == 'o'){ //COMMIT
         int canupgrade = checkIfProjectDirExists(projectName);
         wr = write(socketfd, &canupgrade, sizeof(canupgrade));//Write -1 if client DOESNT have copy of proj - Write 0 if it does
         //printf("Value Of Cancheck = '%d'\n", cancheckout);

         if(canupgrade == 0){//Proj Exists On Client
                  wr = write(socketfd, projectName, strlen(projectName));
                  wr = read(socketfd, servermessage, 2);

                  if(strcmp(servermessage, "0") == 0) { //SERVER SAYING PROJECT EXISTS
                          printf("Starting Commit...\n");
                          commit(projectName, socketfd);
                  }
                  else {
                          printf("The Server Does Not Contain A Project Named '%s'.\n", projectName);
                  }

         }//IF
         else{//Proj Exists On Client
                  printf("Project '%s' Does Not Exist On The Client! (Could Not Complete A Commit)\n", projectName);
         }
       }//UPDATE

                                                         if(operation == 'p'){ //PUSH
                                                             int canpush = checkIfProjectDirExists(projectName);
                                                             wr = write(socketfd, &canpush, sizeof(canpush));//Write -1 if client DOESNT have copy of proj - Write 0 if it does
                                                             //printf("Value Of canpush = '%d'\n", canpush);

                                                             if(canpush == 0){//Proj Exists On Client
                                                                      wr = write(socketfd, projectName, strlen(projectName));
                                                                      wr = read(socketfd, servermessage, 2);

                                                                            if(strcmp(servermessage, "0") == 0) { //SERVER SAYING PROJECT EXISTS
                                                                                    printf("Starting Push...\n");
                                                                                    push(projectName, socketfd);
                                                                            }
                                                                            else {
                                                                                    printf("The Server Does Not Contain A Project Named '%s'.\n", projectName);
                                                                            }

                                                             }//IF
                                                             else{//Proj DNE On Client
                                                                      printf("Project '%s' Does Not Exist On The Client! (Could Not Complete A Push)\n", projectName);
                                                             }
                                                           }//PUSH

         close(socketfd);
         printf("| DISCONNECTED FROM SERVER |\n");
         //printf("-------------------------------\n\n"); //COMMENT THIS OUT

  return EXIT_SUCCESS;
}//END OF MAIN

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|

//-----------------------------------------------------------------------------------------| configure

void configure(char* ip, char* port){ // ./WTF configure <IP/hostname> <port>
    int fd = open(".configure", O_CREAT | O_WRONLY | O_TRUNC, 0644);
        write(fd, ip, strlen(ip));
        write(fd, "\n", 1);
        write(fd, port, strlen(port));
        write(fd, "\n", 1);
        checkConfig = 1;
    close(fd);
}//configure

//-----------------------------------------------------------------------------------------| ADD

void add(char* projectName, char* fileName) { //  ./WTF add <project name> <filename>
    if(checkIfProjectDirExists(projectName) == -1){
            printf("Project Does Not Exist On Client! Could Not Add File To It.\n");
            return;
    }

    /*
    char projdir[1024] = "./";
    strcat(projdir, projectName);
    strcat(projdir, "/");
    //printf("PROJ DIR : %s\n", projdir);
    chdir(projdir);
    //strcat(projdir, fileName);
    */

    chdir(projectName);

    int afd = open(fileName, O_RDONLY);
          if(afd == -1){
              //File Doesn't Exist
              printf("Error, %s Does Not Exist In The Project Folder!\n", fileName);
              return;
          }

    int fd = open(".Manifest", O_RDONLY);
          if(fd == -1){
              //Manifest Doesn't Exist
              printf("Error - .Manifest File Does Not Exist!\n");
              return;
          }

    int manlen = getfilelength(".Manifest");
    char* manifestbuffer = malloc(sizeof(char) * manlen);
    read(fd, manifestbuffer, manlen);
    manifestbuffer[manlen] = '\0';
    if(strstr(manifestbuffer, fileName) != NULL){//File already in manifest
        printf("The File Trying To Be Added To The Project Already Exists (In The Project)! Could Not Add.\n");
        return;
    }
    free(manifestbuffer);
    close(fd);
    fd = open(".Manifest", O_RDONLY);

    //This reads the top of the .Manifest to increment proj number
    char buffer[256];
    bzero(buffer, 256);
    char onebyone = 'x';
        while(onebyone != '\n'){
            read(fd, &onebyone, 1);
            strncat(buffer, &onebyone, 1);
        }
    close(fd);
    int projnum = atoi(buffer);
    //printf("Project Num = '%d'\n", projnum);
    //projnum++;

    char* hash = makehash(fileName);

    //Appends new file being added to bottom of .Manifest
    fd = open(".Manifest", O_WRONLY | O_APPEND);
        write(fd, "\nX\n", 3);//USED TO BE "A" FOR ADD ---> REALIZED I DONT NEED FLAGS LATER INTO PROJECT SO I REPLACED IT WITH AN X TO SIGNIFY A NEW ENTRY
        write(fd, "./", 2);
        write(fd, projectName, strlen(projectName));
        write(fd, "/", 1);
        write(fd, fileName, strlen(fileName));
        write(fd, "\n", 1);
        write(fd, hash, strlen(hash)); //Create hash function
        write(fd, "\n0\n", 3);
    close(fd);


    //Increment Project Number, First clear current proj num, then write in space that was cleared
    fd = open(".Manifest", O_WRONLY);
        write(fd, "          ", 10);
    close(fd);
    fd = open(".Manifest", O_WRONLY);
        char numbuff[100];
        sprintf(numbuff, "%d", projnum);
        //printf("NUM BUFF = %s\n", numbuff);
        write(fd, numbuff, strlen(numbuff));
    close(fd);

    printf("Successfully Added '%s' To '%s'.\n", fileName, projectName);
}//ADD

//-------------------------------------------------------------------------------------------| File Downloading Protocol

void download_files(int socketfd){

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
  read(socketfd, &numfiles, sizeof(int));
  //printf("Number Of Files Downloading : %d\n", numfiles);

  for(i = 0; i < numfiles; i++){//LOOP FOR EACH FILE BEING DOWNLOADED
      int filenamebytes;
      char* filename;
      int filebytes;
      int fd;

      read(socketfd, &filenamebytes, sizeof(int));
      filename = malloc((sizeof(char) * filenamebytes) + 1);
      read(socketfd, filename, filenamebytes);
      filename[filenamebytes] = '\0';
      read(socketfd, &filebytes, sizeof(int));

                      if((operation == 'g') || (operation == 'i' && strcmp(filename, ".ManifestServer") != 0)){//CHECKOUT ---> NEED TO MAKE DIRECTORIES WHEN DOWNLOADING FILES!!
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
                          if(operation == 'g'){
                                max = 2;//INCLUDE ./x/
                          }
                          else if(operation == 'i'){
                                max = 0;//WITHOUT ./x/
                          }

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
                      }//CHECKOUT

      fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
      if(fd == -1){
        //printf("\tCould Not Open %s!\n", filename);
      }
      char recieved;

              for(j = 0; j < filebytes; j++){//LOOP FOR EACH BYTE IN FILE CURRENTLY BEING PROCESSED
                  read(socketfd, &recieved, 1);
                  //printf("Recieved Byte : |%c|\n", recieved);
                  write(fd, &recieved, 1);
              }//LOOP FOR EACH BYTE IN FILE CURRENTLY BEING PROCESSED

      //printf("\tDownloaded '%s'\n", filename);
      close(fd);
      free(filename);

  }//LOOP FOR EACH FILE BEING DOWNLOADED

}//download_files

//-------------------------------------------------------------------------------------------| Check If Project Exists On Client

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

//-------------------------------------------------------------------------------------------| Get Hash Of File

char* makehash(char* filepath){ //instructor mentioned Openssl for hashing library?

  char* newfilepath = malloc(sizeof(char) * strlen(filepath) + 2);
      if(strstr(filepath, "./") == NULL){
          newfilepath[0] = '.'; newfilepath[1] = '/'; newfilepath[2] = '\0';
          strcat(newfilepath, filepath);
      }
      else{
          strcpy(newfilepath, filepath);
      }

      //printf("FILEPATH FROM INPUT : '%s'\n", filepath);
      //printf("FILEPATH FOR HASH : '%s'\n", newfilepath);

  int i = 0;
  unsigned char digest[16];
  char onebyte;
  MD5_CTX context;
  MD5_Init(&context);

  int fd = open(newfilepath, O_RDONLY);
  if(fd == -1){
      printf("ERROR - COULD NOT GET HASH OF FILE %s! (Perhaps It Does Not Exist / An Incorrect Directory Was Accessed)\n", filepath);
      return;
  }

  int filelength = getfilelength(filepath);
  char* wholefile = malloc(sizeof(char) * filelength + 1);
  read(fd, wholefile, filelength);
  wholefile[filelength] = '\0';
  close(fd);

  MD5_Update(&context, wholefile, filelength);
  MD5_Final(digest, &context);
  free(wholefile);

  char* md5string = malloc(sizeof(char) * 33);
  for(i = 0; i < 16; ++i){
        sprintf(&md5string[i*2], "%02x", digest[i]);
  }
  return md5string;
}//makehash

//-------------------------------------------------------------------------------------------| Get File Length

int getfilelength(char* filepath){
  char temp;
  int bytecount = 0;
  int fd = open(filepath, O_RDONLY);
    if(fd == -1){
      printf("ERROR OPENING FILE %s IN getfilelength()", filepath);
      return 0;
    }
            while(read(fd, &temp, 1) != 0){
              //printf("%c\n", temp);
              bytecount++;
            }
            close(fd);
  //printf("Bytes In %s : %d\n", filepath, bytecount);
  return bytecount;
}

//-------------------------------------------------------------------------------------------| Read Current Proj Version And Dir Contents From Server

void readcurrentversion(int socketfd){
  char buffer[256];
  int i = 0, j = 0;
  int projnum;

  read(socketfd, &projnum, sizeof(int));
  printf("Project Version #%d :\n", projnum);


  int numpaths = 0;
  read(socketfd, &numpaths, sizeof(int));
  //printf("Number Of File Paths Being Recieved : %d\n", numpaths);

  for(i = 0; i < numpaths; i++){//LOOP FOR EACH FILE BEING DOWNLOADED
      int filenamebytes;
      char* filename;
      int vernumbytes;
      char* vernum;

      read(socketfd, &filenamebytes, sizeof(int));
      filename = malloc((sizeof(char) * filenamebytes));

      read(socketfd, filename, filenamebytes);
      filename[filenamebytes] = '\0';

      read(socketfd, &vernumbytes, sizeof(int));
      vernum = malloc((sizeof(char) * vernumbytes));

      read(socketfd, vernum, vernumbytes);
      vernum[vernumbytes] = '\0';

      printf("\t%s (Version #%s)\n", filename, vernum);
      free(filename);
      free(vernum);
    }//LOOP FOR EACH PATH BEING SENT
}

//-------------------------------------------------------------------------------------------| REMOVE

void removefl(char* projectName, char* fileName){ //  ./WTF add <project name> <filename>
    if(checkIfProjectDirExists(projectName) == -1){
            printf("Project Does Not Exist On Client! Could Not Add File To It.\n");
            return;
    }

    chdir("./Client/");
    char projdir[1024] = "./";
    strcat(projdir, projectName);
    chdir(projdir);
    strcat(projdir, "/");
    strcat(projdir, fileName);//Proj dir contains filename ---> .(/Client)/projname/filename.txt

    int fd = open(".Manifest", O_RDONLY);
          if(fd == -1){
              //Manifest Doesn't Exist
              printf("Error, .Manifest File Does Not Exist!\n");
          }

    int manlen = getfilelength(".Manifest");
    //printf("MANIFEST FILE LENGTH = %d\n", manlen);
    char* manifestbuffer = malloc(sizeof(char) * manlen);
    read(fd, manifestbuffer, manlen);
    close(fd);

    char* found = strstr(manifestbuffer, projdir);
          if(found != NULL){//File already in manifest
                found = found - 3;

                int i = 0, foundlen = strlen(found), newlines = 0;
                      for(i = 0; i < foundlen; i++){
                            if(newlines >= 5){
                                found[i] = '\0';
                            }
                            else if(found[i] == '\n'){
                                newlines++;
                                //printf("NEWLINE %d\n", newlines);
                            }
                      }//FOR
                    //printf("FOUND :\n%s\n[END OF FOUND - %d]-----------------------------------------------\n", found, strlen(found));

                    if(manlen > 0){

                        char *start = strstr(manifestbuffer, found);
                        //printf("FOUND LENGTH = %d\n", strlen(found));
                        char *end = strstr(manifestbuffer, found) + strlen(found);
                        int index = start ? start - manifestbuffer : -1;

                        bzero(manifestbuffer, manlen);
                        fd = open(".Manifest", O_RDONLY);
                        read(fd, manifestbuffer, manlen);
                        close(fd);
                        //printf("OLD MANIFEST : \n%s\n[OLD MAN]]-----------------------------------------------\n", manifestbuffer);

                        char* new_manifestbuffer = malloc((sizeof(char) * manlen));
                        strncpy(new_manifestbuffer, manifestbuffer, index);
                        strcat(new_manifestbuffer, end);
                        //printf("\n\n\nSTART : \n%s\n[START]-----------------------------------------------\n", new_manifestbuffer);
                        //printf("\n\n\nEND : \n%s\n[END]---------------------------------------------------\n", end);
                        //printf("NEW MANIFEST : \n%s\n[NEW MAN]]-----------------------------------------------\n", new_manifestbuffer);

                            fd = open(".Manifest", O_CREAT | O_WRONLY | O_TRUNC, 0644);
                            write(fd, new_manifestbuffer, strlen(new_manifestbuffer));
                            close(fd);
                        free(new_manifestbuffer);
                    }

          }//IF FOUND != NULL
          else{//FILE NOT IN Manifest
              printf("Error, File Was Not Found On The Client-Side Manifest!\n");
              free(manifestbuffer);

              return;
          }
           printf("Removed '%s' From '%s'\n", fileName, projectName);
    free(manifestbuffer);
    close(fd);
    chdir("..");
}//REMOVE

//-------------------------------------------------------------------------------------------| UPDATE

void update(char* projectName, int socketfd){

    //Get Server Manifest
    //Compare Every Entry To Client Manifest
    //See If There Are Changes On Server Side For Client

    /*
    IN ALL CASES LISTED HERE, VERSION NUMBERS ARE NOT THE SAME!!!
        PSC1 : MODIFY CODE
              Client entry files have a version and stored hash that is different from the servers
              Live hash of those files match the hash in the client's .MANIFEST
                    APPEND 'M <FILE/PATH> <SERVER HASH>' TO .Update
                    OUTPUT 'M <FILE/PATH>' TO STDOUT
        PSC2 : ADD CODE ---
              Client manifest does not have a file or more than one file that is in the servers
                    APPEND 'A <FILE/PATH> <SERVER HASH>' TO .Update
                    OUTPUT 'A <FILE/PATH>' TO STDOUT
        PSC3 : DELETE CODE ---
              Client manifest has a file that is not in the servers manifest
                    APPEND 'D <FILE/PATH> <SERVER HASH>' TO .Update
                    OUTPUT 'D <FILE/PATH>' TO STDOUT
        FC1 : NEED TO DOWNLOAD THINGS BUT CAN'T BECAUSE USER HAS MADE CHANGES TO THE SAME FILES ---
              Server has updated data for the client, but the user has changed that file locally.
              Client manifest has a file who's hash is diff than both the server's manifest AND the live hash of the files
                    APPEND 'C <FILE/PATH> <LIVE HASH>' TO .Conflict
                    OUTPUT 'C <FILE/PATH>' TO STDOUT
              After getting a conflict don't stop... find all updates and conflicts.
              After scanning all of server manifest, output to STDOUT "Conflicts Were Found And Must Be Resolved Before The Project Can Be Updated"

    */
        //IF THERE ARE, IT ADDS A LINE TO A .UPDATE FILE TO REFLECT THE CHANGE -> OUTPUTS INFO TO STDOUT TO LET USER KNOW WHAT WILL BE CHANGED
        //IF THERE IS AN UPDATE BUT THE USER CHANGED THE FILE THAT NEEDS TO BE UPDATED, UPDATE SHOULD WRITE A .CONFLICT FILE AND DELETE ANY .UPDATE FILE IF PRESENT
        //IF SERVER HAS NO CHANGES FOR CLIENT, UPDATE CAN STOP AND DOES NOT HAVE TO DO LINE BY LINE COMPARISON -> BLANK THE .UPDATE FILE AND DELETE ANY .CONFLICT FILE B/C THERE ARE NO SERVER UPDATES

    chdir(projectName);
    download_files(socketfd); //Download The .ManifestServer File

    int readbytes = 0; int readbytes2 = 0;
    int clientfdlength = getfilelength(".Manifest");
    int serverfdlength = getfilelength(".ManifestServer");

    int clientfd = open(".Manifest", O_RDONLY);
    int serverfd = open(".ManifestServer", O_RDONLY);
    int ufd;
    int cfd;

    //PUT CLIENT MANIFEST AND SERVER MANIFEST INTO A BUFFER FOR COMPARISON WITH ENTRYBUFFER
                char* clientmanifest = malloc(sizeof(char) * clientfdlength);
                read(clientfd, clientmanifest, clientfdlength);
                close(clientfd);
                clientfd = open(".Manifest", O_RDONLY);

                char* servermanifest = malloc(sizeof(char) * serverfdlength);
                read(serverfd, servermanifest, serverfdlength);
                close(serverfd);
                serverfd = open(".ManifestServer", O_RDONLY);

    //READ .MANIFESTSERVER AND GO THROUGH FOUND ENTRIES ONE BY ONE
    char vnumserverbuff[32] = ""; int vnumserver = -1;
    char vnumclientbuff[32] = ""; int vnumclient = -1;

    char fullentrybuffer[1024] = "";
    char filepathbuffer[1024] = "";
    char hashbuffer[1024] = "";
    char versionbuffer[16] = "";

    char minientrybuffer = '\0';
    int newlines = 0;

                    while(minientrybuffer != '\n'){//LOOP TO GET SERVER PROJECT NUMBER
                        read(serverfd, &minientrybuffer, 1);
                        //printf("|%c|\n", minientrybuffer);
                        if(minientrybuffer != ' ' && minientrybuffer != '\n'){
                            strncat(vnumserverbuff, &minientrybuffer, 1);
                        }
                        readbytes++;
                    }
                    vnumserver = atoi(vnumserverbuff);
                    //printf("SERVER PROJECT NUMBER : %d | %s\n", vnumserver, vnumserverbuff);
                    read(serverfd, &minientrybuffer, 1); readbytes++; //Read the second newline (line #2 in the .ManifestServer)
                    minientrybuffer = '\0';

                                      while(minientrybuffer != '\n'){//LOOP TO GET CLIENT PROJECT NUMBER
                                          read(clientfd, &minientrybuffer, 1);
                                          if(minientrybuffer != ' ' && minientrybuffer != '\n'){
                                              strncat(vnumclientbuff, &minientrybuffer, 1);
                                          }
                                          readbytes2++;
                                      }
                                      vnumclient = atoi(vnumclientbuff);
                                      //printf("CLIENT PROJECT NUMBER : %d | %s\n", vnumclient, vnumclientbuff);
                                      read(clientfd, &minientrybuffer, 1); //Read the second newline (line #2 in the .Manifest)

    if(vnumserver == vnumclient){ //BOTH MANIFEST VERSIONS ARE THE SAME
            ufd = open(".Update", O_CREAT | O_WRONLY | O_TRUNC, 0644);
                  write(ufd, "V ", 2);
                  write(ufd, vnumserverbuff, strlen(vnumserverbuff));
                  write(ufd, " ", 1);
            close(ufd);
            remove(".Conflict");
            printf("Up To Date.\n");
                remove(".ManifestServer");
                close(cfd);
                close(clientfd);
                close(serverfd);
                chdir("..");
                return;
    }

    else{
              ufd = open(".Update", O_CREAT | O_WRONLY | O_TRUNC, 0644);
              close(ufd);
              cfd = open(".Conflict", O_CREAT | O_WRONLY | O_TRUNC, 0644);
              close(cfd);

              while(readbytes < serverfdlength){ //FIRST LOOK AT SERVER ENTRIES
                      bzero(fullentrybuffer, 1024);
                      bzero(filepathbuffer, 1024);
                      bzero(hashbuffer, 1024);
                      bzero(versionbuffer, 16);
                      newlines = 0;

                      while(newlines != 5){//LOOP TO GET EACH ENTRY IN THE SERVER MANIFEST
                          read(serverfd, &minientrybuffer, 1);
                              if((newlines == 1) && (minientrybuffer != '\n')){
                                  strncat(filepathbuffer, &minientrybuffer, 1);
                              }
                                    if((newlines == 2) && (minientrybuffer != '\n')){
                                        strncat(hashbuffer, &minientrybuffer, 1);
                                    }
                                          if((newlines == 3) && (minientrybuffer != '\n')){
                                              strncat(versionbuffer, &minientrybuffer, 1);
                                          }
                                                if(minientrybuffer == '\n'){
                                                  newlines++;
                                                }
                          strncat(fullentrybuffer, &minientrybuffer, 1);
                          readbytes++;
                      }
                        //printf("ENTRY SERVER :\n|%s|\n", fullentrybuffer);
                        //printf("\nPATH : %s\n", filepathbuffer);
                        //printf("HASH : %s\n", hashbuffer);
                        //printf("VERSION : %s\n", versionbuffer);

                        //COMPARING SERVER ENTRIES TO CLIENT
                        char* ptr = NULL;
                        if((ptr = strstr(clientmanifest, filepathbuffer)) != NULL){ //SEARCH FOR A PATH FROM SERVER BEING CONTAINED IN CLIENT - IF FOUND
                            //printf("Found %s (A Server Entry) In Client Manifest\n", filepathbuffer);

                            //PSC1 MODIFY CASE
                            //FIND IF VERSION AND STORED HASH IS DIFF FROM SERVER
                            ptr = ptr + strlen(filepathbuffer) + 1; //+1 Is The Newline
                            char clienthash[33]; char clientversion[16];
                            strncpy(clienthash, ptr, 32);
                            clienthash[33] = '\0';

                                ptr = ptr + 33;
                                while(ptr[0] != '\n'){
                                      strncpy(clientversion, ptr, 1);
                                      ptr = ptr + 1;
                                }

                                      chdir("..");
                                      char* livehash = makehash(filepathbuffer);
                                      chdir(projectName);
/*
                                printf("Client Hash = |%s|\n", clienthash);
                                printf("Client Version = |%s|\n", clientversion);
                                printf("Live Client Hash = |%s|\n", livehash);
                                printf("Server Hash = |%s|\n", hashbuffer);
                                printf("Server Version = |%s|\n", versionbuffer);
*/
                                      if(strcmp(clienthash, livehash) == 0){//IDENTICAL HASH VS LIVE HASH
                                          //printf("Hashes Are Identical On Client\n\n");
                                                if((strcmp(clienthash, hashbuffer) == 0) && (strcmp(clientversion, versionbuffer) == 0)){//COMPARE CLIENT AND SERVER
                                                        ///CLIENT HASH == SERVER HASH... NO UPDATE NEEDED!
                                                        //printf("Hash And Version ARE The Same Across Client And Server - Don't Update %s\n", filepathbuffer);
                                                }
                                                else{
                                                        //printf("Hash And Version Are The NOT The Same Across Client And Server\n");
                                                        ufd = open(".Update", O_CREAT | O_WRONLY | O_APPEND, 0644);
                                                            write(ufd, "M ", 2);
                                                            write(ufd, filepathbuffer, strlen(filepathbuffer));
                                                            write(ufd, " ", 1);
                                                            write(ufd, hashbuffer, strlen(hashbuffer));
                                                            write(ufd, "\n", 1);
                                                        close(ufd);
                                                        printf("\tM %s\n", filepathbuffer);
                                                }
                                      }
                                      else{//NOT IDENTICAL HASH VS LIVE HASH
                                            //printf("Live Hash And Hash On Client Are NOT IDENTICAL\n");
                                            if(strcmp(hashbuffer, livehash) != 0){
                                                //printf("Hashes ALSO NOT IDENTICAL On Server\n\n");
                                                cfd = open(".Conflict", O_CREAT | O_WRONLY | O_APPEND, 0644);
                                                    write(ufd, "C ", 2);
                                                    write(ufd, filepathbuffer, strlen(filepathbuffer));
                                                    write(ufd, " ", 1);
                                                    write(ufd, livehash, strlen(livehash));
                                                    write(ufd, "\n", 1);
                                                close(cfd);
                                                printf("\tC %s\n", filepathbuffer);
                                            }
                                      }
                        }
                        else{ //IF NOT FOUND ---> PSC2 ADD CASE
                            //printf("Did Not Find %s (A Server Entry) In Client Manifest\n", filepathbuffer);
                            ufd = open(".Update", O_CREAT | O_WRONLY | O_APPEND, 0644);
                                write(ufd, "A ", 2);
                                write(ufd, filepathbuffer, strlen(filepathbuffer));
                                write(ufd, " ", 1);
                                write(ufd, hashbuffer, strlen(hashbuffer));
                                write(ufd, "\n", 1);
                            close(ufd);
                            printf("\tA %s\n", filepathbuffer);
                        }
            }//WHILE READBYTES
                                while(readbytes2 < clientfdlength){ //NEXT LOOK AT CLIENT ENTRIES
                                        bzero(fullentrybuffer, 1024);
                                        bzero(filepathbuffer, 1024);
                                        bzero(hashbuffer, 1024);
                                        newlines = 0;

                                        while(newlines != 5){//LOOP TO GET EACH ENTRY IN THE SERVER MANIFEST
                                            read(clientfd, &minientrybuffer, 1);
                                                if((newlines == 1) && (minientrybuffer != '\n')){
                                                    strncat(filepathbuffer, &minientrybuffer, 1);
                                                }
                                                      if((newlines == 2) && (minientrybuffer != '\n')){
                                                          strncat(hashbuffer, &minientrybuffer, 1);
                                                      }
                                                            if(minientrybuffer == '\n'){
                                                              newlines++;
                                                            }
                                            strncat(fullentrybuffer, &minientrybuffer, 1);
                                            readbytes2++;
                                        }
                                          //printf("ENTRY CLIENT :\n|%s|\n", fullentrybuffer);
                                          //printf("PATH : %s\n", filepathbuffer);
                                          //printf("HASH : %s\n", hashbuffer);

                                          //COMPARING CLIENT ENTRIES TO SERVER
                                          if(strstr(servermanifest, filepathbuffer) != NULL){ //SEARCH FOR A PATH FROM SERVER BEING CONTAINED IN SERVER - IF FOUND
                                              //printf("Found %s (A Client Entry) In Server Manifest\n", filepathbuffer);
                                          }
                                          else{ //IF NOT FOUND ---> PS3 DELETE CODE
                                              //printf("Did Not Find %s (A Client Entry) In Server Manifest\n", filepathbuffer);
                                              ufd = open(".Update", O_CREAT | O_WRONLY | O_APPEND, 0644);
                                                  write(ufd, "D ", 2);
                                                  write(ufd, filepathbuffer, strlen(filepathbuffer));
                                                  write(ufd, " ", 1);
                                                  write(ufd, hashbuffer, strlen(hashbuffer));
                                                  write(ufd, "\n", 1);
                                              close(ufd);
                                              printf("\tD %s\n", filepathbuffer);
                                          }
                              }//WHILE READBYTES2

                              //WRITE THE SERVER MANIFEST VERSION NUM TO BOTTOM OF UPDATE
                              ufd = open(".Update", O_CREAT | O_WRONLY | O_APPEND, 0644);
                                  write(ufd, "V ", 2);
                                  write(ufd, vnumserverbuff, strlen(vnumserverbuff));
                                  write(ufd, " ", 1);
                              close(ufd);

  }//ELSE

    cfd = open(".Conflict", O_RDONLY); readbytes = getfilelength(".Conflict");
    ufd = open(".Update", O_RDONLY);

    if(readbytes == 0){//NO CONFLICTS FOUND
          remove(".Conflict");
    }
    else{
          printf("Conflicts Were Found And Must Be Resolved Before The Project Can Be Updated!\n");
    }
                char empty = 'x';
                read(ufd, &empty, 1);
                if(empty == 'V'){//ONLY THING IN MANIFEST IS VERSION NUM OF SERVER!! SO ITS EMPTY!
                      printf("Up To Date.\n");
                }
                else{
                      printf("Project Updated.\n");
                }

    remove(".ManifestServer");
    close(cfd);
    close(ufd);
    close(clientfd);
    close(serverfd);
    chdir("..");
}//UPDATE

//-------------------------------------------------------------------------------------------| Checkout

void checkout(char* projectName, int socketfd){    // ./WTF checkout <project name>
  mkdir(projectName, 0777);
  chdir(projectName);
        download_files(socketfd);//FIRST DOWNLOAD MANIFEST
        rename(".ManifestServer", ".Manifest");
              chdir("..");
              download_files(socketfd);//THEN DOWNLOAD ALL OTHER FILES
}//CHECKOUT

//-------------------------------------------------------------------------------------------| UPGRADE

void upgrade(char* projectName, int socketfd){

  chdir(projectName);

  int updatelength = 0;
  int readbytes = 0;
  int ufd = open(".Conflict", O_RDONLY);
  if(ufd != -1){//CONFLICT EXISTS!
        printf("Conflicts Exist, Therefore An Upgrade Cannot Be Performed!\n(Resolve All Conflicts And Update!)\n");
        close(ufd);
        return;
  }
  close(ufd);

        char check;
        ufd = open(".Update", O_RDONLY);
        if(ufd == -1){//UPDATE DOES NOT EXIST!
              printf("No Update File Exists, Therefore An Upgrade Cannot Be Performed!\n(Call The 'update' Command!)\n");
              close(ufd);
              write(socketfd, "V", 1);
              return;
        }
        else{
              read(ufd, &check, 1);
              close(ufd);
        }

  updatelength = -1;
  updatelength = getfilelength(".Update"); //printf(".UPDATE LENGTH : %d\n", ufd);

  if(check == 'V'){//UPDATE FILE ONLY CONTAINS THE SERVER VERSION
        printf("The Project Is Up To Date!\n");
        write(socketfd, "V", 1);
              ufd = open(".Update", O_RDONLY);
                  char versionbuff[16] = "";
                  read(ufd, &check, 1); //SKIP V
                  read(ufd, &check, 1); //SKIP space
                  check = 'x';

                  while(check != ' '){
                      read(ufd, &check, 1);
                            if(check != ' '){
                              //printf("CHECK : %c\n", check);
                              strncat(versionbuff, &check, 1);
                            }
                  }
                  close(ufd);
                  //printf("VERSION NUM : %s\n", versionbuff);
              remove(".Update");
              //MAKES THE CLIENT MANIFEST THE VERSION OF THE SERVER MANIFEST, BECAUSE THE PROJECT IS UP TO DATE WITH THE SERVER
              ufd = open(".Manifest", O_WRONLY);
                  write(ufd, "           ", 11);
              close(ufd);
              ufd = open(".Manifest", O_WRONLY);
                  write(ufd, versionbuff, strlen(versionbuff));
              close(ufd);
        return;
  }//NO ACTUAL UPGRADES NEEDED!!

  ufd = open(".Update", O_RDONLY);

                  while(readbytes < updatelength){
                        char operation = 'x';
                        char obs = 'x';
                        char filepath[256] = "";
                        char hash[33] = "";
                        char versionbuff[16] = "";
                            bzero(filepath, 256);
                            bzero(hash, 33);

                                    //GETS THE OPERATION, PATH, AND HASH FROM .UPDATE
                                    read(ufd, &operation, 1);//GET OPERATION
                                    readbytes++;

                                    if(operation == 'V'){//LAST REACHED / VERSION NUM ----------------------------------------------------------------------------------------
                                      read(ufd, &obs, 1); readbytes++;//SKIP SPACE + PROJECT NAME
                                      obs = 'x';

                                      while(obs != ' '){
                                          read(ufd, &obs, 1);
                                          readbytes++;
                                          if(obs != ' '){
                                            strncat(versionbuff, &obs, 1);
                                          }
                                      }//WHILE
                                      //printf("PROJECT VERSION : %s\n", versionbuff);
                                      int fd = open(".Manifest", O_WRONLY);
                                          write(fd, "           ", 11);
                                      close(fd);
                                      fd = open(".Manifest", O_WRONLY);
                                          write(fd, versionbuff, strlen(versionbuff));
                                      close(fd);

                                      char op = 'V';
                                      write(socketfd, &op, 1);

                                    }//VERSION

                                    else{//ACTUAL UPDATE REACHED --------------------------------------------------------------------------------------------------------------
                                          int i = 0;
                                          for(i = 0; i < (4 + strlen(projectName)); i++){
                                              read(ufd, &obs, 1);//SKIP SPACE + PROJECT NAME
                                              readbytes++;
                                          }

                                          obs = 'x';//RESET OBSERVED
                                          while(obs != ' '){//FILE PATH
                                                read(ufd, &obs, 1);
                                                readbytes++;
                                                if(obs != ' '){
                                                    strncat(filepath, &obs, 1);
                                                }
                                          }
                                          obs = 'x';//RESET OBSERVED
                                          while(obs != '\n'){//HASH
                                                read(ufd, &obs, 1);
                                                readbytes++;
                                                if(obs != '\n'){
                                                    strncat(hash, &obs, 1);
                                                }
                                          }//Hash

                                          //printf("OPERATION : |%c|\n", operation);
                                          //printf("\nFILE PATH/NAME : |%s|\n", filepath);
                                          //printf("HASH : |%s|\n", hash);
                                    }

                  char op;
                  if(operation == 'A'){//ADD --------------------------------------------------------------------------------------------------------------
                        printf("\tAdding %s.\n", filepath);
                        op = 'A';
                        write(socketfd, &op, 1);
                        download_files(socketfd);//DOWNLOADS THE SERVER .MANIFEST ---> NEED TO EXTRACT THE FILE VERSION OF THE FILE WE ARE ADDING

                        int servermanifestlength = getfilelength(".ManifestServer");
                        char* servermanifestbuffer = malloc(sizeof(char) * servermanifestlength);
                        int fdmans = open(".ManifestServer", O_RDONLY);
                        read(fdmans, servermanifestbuffer, servermanifestlength);
                        close(fdmans);
                        servermanifestbuffer[servermanifestlength] = '\0';
                        char fileversion[16] = "";

                        char* ptr = strstr(servermanifestbuffer, filepath) + strlen(filepath) + 2 + 32;//SHOULD LEAD UP TO THE FILE VERSION NUM
                        //printf("PTR : \n%s\n", ptr);

                        while(ptr[0] != '\n'){
                          strncat(fileversion, &ptr[0], 1);
                          ptr = ptr + 1;
                        }
                        //printf("FILE VERSION : |%s|\n", fileversion);//FINALLY HAVE FILE Version
                        free(servermanifestbuffer);
                        remove(".ManifestServer");

                        //NOW CAN APPEND TO .MANIFEST
                        fdmans = open(".Manifest", O_WRONLY | O_APPEND, 0644);
                              write(fdmans, "\nX\n", 3);//USED TO BE "A" FOR ADD ---> REALIZED I DONT NEED FLAGS LATER INTO PROJECT SO I REPLACED IT WITH AN X TO SIGNIFY A NEW ENTRY
                              write(fdmans, "./", 2);
                              write(fdmans, projectName, strlen(projectName));
                              write(fdmans, "/", 1);
                              write(fdmans, filepath, strlen(filepath));
                              write(fdmans, "\n", 1);
                              write(fdmans, hash, strlen(hash)); //Create hash function
                              write(fdmans, "\n", 1);
                              write(fdmans, fileversion, strlen(fileversion));
                              write(fdmans, "\n", 1);
                        close(fdmans);

                        //NOW CAN ADD FILE FROM THE SERVER;
                        write(socketfd, filepath, strlen(filepath));
                        download_files(socketfd);//DOWNLOADS A FILE WITH THE FILEPATH GIVEN
                  }//ADD

                  else if(operation == 'M'){//MODIFY --------------------------------------------------------------------------------------------------------------
                        printf("\tModified %s.\n", filepath);
                        op = 'M';
                        write(socketfd, &op, 1);

                        download_files(socketfd);//DOWNLOADS THE SERVER .MANIFEST ---> NEED TO EXTRACT THE FILE VERSION OF THE FILE WE ARE ADDING

                        int servermanifestlength = getfilelength(".ManifestServer");
                        char* servermanifestbuffer = malloc(sizeof(char) * servermanifestlength);
                        int fdmans = open(".ManifestServer", O_RDONLY);
                        read(fdmans, servermanifestbuffer, servermanifestlength);
                        close(fdmans);
                        servermanifestbuffer[servermanifestlength] = '\0';
                        char fileversion[16] = "";

                        char* ptr = strstr(servermanifestbuffer, filepath) + strlen(filepath) + 2 + 32;//SHOULD LEAD UP TO THE FILE VERSION NUM
                        //printf("PTR : \n%s\n", ptr);

                        while(ptr[0] != '\n'){//GET SERVER FILE VER
                          strncat(fileversion, &ptr[0], 1);
                          ptr = ptr + 1;
                        }
                        //printf("FILE VERSION : |%s|\n", fileversion);//FINALLY HAVE FILE Version
                        free(servermanifestbuffer);
                        remove(".ManifestServer");

                        int clientmanifestlength = getfilelength(".Manifest");
                        char* clientmanifestbuffer = malloc(sizeof(char) * clientmanifestlength);
                              fdmans = open(".Manifest", O_RDONLY);
                                    read(fdmans, clientmanifestbuffer, clientmanifestlength);
                                    clientmanifestbuffer[clientmanifestlength] = '\0';
                              close(fdmans);

                              ptr = strstr(clientmanifestbuffer, filepath) + strlen(filepath) + 1;//PASS /N AND NAME
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
                                        //printf("CLIENT FILE VER = %s\n", clifv);
                                        //ptr = ptr - strlen(clifv);
                                        //printf("PTR LOCATION = |%s|\n", ptr);

                              //WRITE THE HASH IN .UPDATE TO THE .MANIFEST
                              fdmans = open(".Manifest", O_CREAT | O_WRONLY | O_TRUNC, 0644);
                                    write(fdmans, clientmanifestbuffer, strlen(clientmanifestbuffer));
                              close(fdmans);

                              //GRAB INDEX OF THE START OF THE FILE VERSION SO NO OVERWRITE HAPPENS
                              int index = ptr ? ptr - clientmanifestbuffer : -1; //THIS CLEARS MANIFEST BUFFER SO WE NEED TO READ IT AGAIN
                              index = index - strlen(clifv);
                                    bzero(clientmanifestbuffer, clientmanifestlength);
                                    fdmans = open(".Manifest", O_RDONLY);
                                    read(fdmans, clientmanifestbuffer, clientmanifestlength);
                                    close(fdmans);

                              fdmans = open(".Manifest", O_CREAT | O_WRONLY | O_TRUNC, 0644);
                                    write(fdmans, clientmanifestbuffer, index);
                                    //printf("WROTE THE FIRST %d bytes of man buff\n", index);
                              close(fdmans);

                              fdmans = open(".Manifest", O_CREAT | O_WRONLY | O_APPEND, 0644);
                                    write(fdmans, fileversion, strlen(fileversion));
                                    write(fdmans, ptr, (strlen(clientmanifestbuffer) - index - strlen(clifv) - 1));
                                    //printf("WROTE THE FILE VERSION\n");
                              close(fdmans);

                        free(clientmanifestbuffer);

                        //NOW CAN ADD FILE FROM THE SERVER;
                        write(socketfd, filepath, strlen(filepath));
                        download_files(socketfd);//DOWNLOADS A FILE WITH THE FILEPATH GIVEN
                  }//MODIFY

                  else if(operation == 'D'){//DELETE --------------------------------------------------------------------------------------------------------------
                        printf("\tDeleted %s.\n", filepath);
                        op = 'D';
                        write(socketfd, &op, 1);

                        //PUT CLIENT MANIFEST INTO A BUFFER ---> GOING TO SEW PARTS AROUND THE ENTRY INTO A NEW MANIFEST TO SAVE
                        int clientmanifestlength = getfilelength(".Manifest");
                        char* clientmanifestbuffer = malloc(sizeof(char) * clientmanifestlength);
                        int fdmans = open(".Manifest", O_RDONLY);
                        read(fdmans, clientmanifestbuffer, clientmanifestlength);
                        close(fdmans);
                        //clientmanifestbuffer[clientmanifestlength] = '\0';

                        char* start = strstr(clientmanifestbuffer, filepath) - 3 - strlen(projectName) - 3;// PTR TO GIVES START OF ENTRY
                        char* end = start;
                        int index = start ? start - clientmanifestbuffer : -1; //THIS CLEARS MANIFEST BUFFER SO WE NEED TO READ IT AGAIN
                              bzero(clientmanifestbuffer, clientmanifestlength);
                              fdmans = open(".Manifest", O_RDONLY);
                              read(fdmans, clientmanifestbuffer, clientmanifestlength);
                              //clientmanifestbuffer[clientmanifestlength] = '\0';
                              close(fdmans);

                        start = strstr(clientmanifestbuffer, filepath) - 3 - strlen(projectName) - 3;
                        end = start;

                        //printf("START : \n|%s|\n", start);
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
                        char* newbuff = malloc(sizeof(char) * clientmanifestlength);
                        bzero(newbuff,clientmanifestlength);
                        strncpy(newbuff, clientmanifestbuffer, index);
                        strncat(newbuff, (start + read), (clientmanifestlength - index - read));

                        //printf("NEW MANIFEST WITHOUT %s : \n|%s|\n", filepath, newbuff);
                        fdmans = open(".Manifest", O_CREAT | O_WRONLY | O_TRUNC, 0644);
                            write(fdmans, newbuff, strlen(newbuff));
                        close(fdmans);

                        remove(filepath);
                        free(newbuff);
                        free(clientmanifestbuffer);
                  }//DELETE

                        //printf("\tREADBYTES / UPDATE LENGTH = %d / %d\n\n", readbytes, updatelength);

                }//WHILE
  close(ufd);
  remove(".Update");

}//UPGRADE

//-------------------------------------------------------------------------------------| Commit

void commit(char* projectName, int socketfd){

    /*
                  FAILS IF :
                      Project name doesnt exist on servers
                      If client cannot fetch servers manifest for the projects
                      Client has a .Update file that isn't empty
                      Client has a .Conflict file

        SHOULD :
            Client should get server manifest
            Check if server version + client version matches
                IF THEY DONT MATCH :
                        Client Stops : Tell user to update the local project first
                IF THEY DO MATCH :
                        Client runs through its own manifest and gets A LIVE HASH for each file listed in it.
                        IF A LIVE HASH IS DIFFERENT than the client manifests hash should have the entry written to a .Commit File
                            WITH FILE VERSION NUMBER INCREMENTED;

                    Commit should succeed if...
                    MODIFY :
                          Server and client manifest have the same file
                          HASH STORED IN BOTH SERVER AND CLIENT MANIFEST ARE THE SAME
                          Clients live hash of the file is different from the stored hash
                              APPEND M <FILE/PATH> <SERVER HASH> <INCREMENTED FILE VERSION> to .Commit
                              Output M <FILE/PATH> to STDOUT
                    ADD :
                          The servers manifest does not have the file, but the client manifest does
                              APPEND A <FILE/PATH> <SERVER HASH> <INCREMENTED FILE VERSION> to .Commit
                              Output A <FILE/PATH> to STDOUT
                    DELETE :
                          The CLIENT manifest does not have the file, but the server manifest does
                              APPEND D <FILE/PATH> <SERVER HASH> <INCREMENTED FILE VERSION> to .Commit
                              Output D <FILE/PATH> to STDOUT

        After Creating .Commit, send it to the server to save as an active commit.

        IF ANY FILES IN THE SERVERS MANIFEST HAVE A DIFFERENT HASH THAN THE CLIENTS, WHOSE VERSION NUM IS NOT LOWER THAN THE CLIENTS...
            COMMIT FAILS WITH A MESSAGE THAT THE CLIENT MUST SYNC WITH THE REPOSITORY BEFORE COMMITTING CHANGES
            IF CLIENT COMMIT FAILS, IT SHOULD DELETE ITS OWN .COMMIT FILE

    */

    chdir(projectName);
    download_files(socketfd); //Download The .ManifestServer File
    char minientrybuffer = '\0';

                    //CHECK TO SEE IF UPDATE OR CONFLICTS EXIST
                    int checkingfd = open(".Conflict", O_RDONLY);
                    if(checkingfd != -1){//CONFLICT EXISTS!
                          printf("A .Conflict File Exists, Thus A Commit Could Not Be Performed\n");
                          remove(".ManifestServer");
                          close(checkingfd);
                          chdir("..");
                          return;
                    }
                    close(checkingfd);

                    checkingfd = open(".Update", O_RDONLY);
                    if(checkingfd != -1){//UPDATE EXISTS, CHECK IF EMPTY FOR REAL
                          read(checkingfd, &minientrybuffer, 1);
                          if(minientrybuffer != 'V'){//FILE IS NOT EMPTY
                                printf("Update File Is Not Empty! Commit Cannot Occur.\n");
                                remove(".ManifestServer");
                                close(checkingfd);
                                chdir("..");
                                return;
                          }
                          else{
                              //printf("Update File Is Empty.\n");
                          }
                    }
                    else{//UPDATE DOESNT EXIST, BUT COMMIT SHOULD NOT FAIL
                              //printf("A .Update File Does Not Exist\n");
                    }
                    close(checkingfd);

    int readbytes = 0; int readbytes2 = 0;
    int clientfdlength = getfilelength(".Manifest");
    int serverfdlength = getfilelength(".ManifestServer");

    int clientfd = open(".Manifest", O_RDONLY);
    int serverfd = open(".ManifestServer", O_RDONLY);
    int cfd;

    //PUT CLIENT MANIFEST AND SERVER MANIFEST INTO A BUFFER FOR COMPARISON WITH ENTRYBUFFER
                char* clientmanifest = malloc(sizeof(char) * clientfdlength);
                read(clientfd, clientmanifest, clientfdlength);
                close(clientfd);
                clientfd = open(".Manifest", O_RDONLY);

                char* servermanifest = malloc(sizeof(char) * serverfdlength);
                read(serverfd, servermanifest, serverfdlength);
                close(serverfd);
                serverfd = open(".ManifestServer", O_RDONLY);

    //READ .MANIFESTSERVER AND GO THROUGH FOUND ENTRIES ONE BY ONE
    char vnumserverbuff[32] = ""; int vnumserver = -1;
    char vnumclientbuff[32] = ""; int vnumclient = -1;

    char fullentrybuffer[1024] = "";
    char filepathbuffer[1024] = "";
    char hashbuffer[1024] = "";
    char versionbuffer[16] = "";

    minientrybuffer = '\0';
    int newlines = 0;

                    while(minientrybuffer != '\n'){//LOOP TO GET SERVER PROJECT NUMBER
                        read(serverfd, &minientrybuffer, 1);
                        //printf("|%c|\n", minientrybuffer);
                        if(minientrybuffer != ' ' && minientrybuffer != '\n'){
                            strncat(vnumserverbuff, &minientrybuffer, 1);
                        }
                        readbytes++;
                    }
                    vnumserver = atoi(vnumserverbuff);
                    //printf("SERVER PROJECT NUMBER : %d\n", vnumserver);
                    read(serverfd, &minientrybuffer, 1); readbytes++; //Read the second newline (line #2 in the .ManifestServer)
                    minientrybuffer = '\0';

                                      while(minientrybuffer != '\n'){//LOOP TO GET CLIENT PROJECT NUMBER
                                          read(clientfd, &minientrybuffer, 1);
                                          if(minientrybuffer != ' ' && minientrybuffer != '\n'){
                                              strncat(vnumclientbuff, &minientrybuffer, 1);
                                          }
                                          readbytes2++;
                                      }
                                      vnumclient = atoi(vnumclientbuff);
                                      //printf("CLIENT PROJECT NUMBER : %d\n", vnumclient);
                                      read(clientfd, &minientrybuffer, 1); //Read the second newline (line #2 in the .Manifest)

    if(vnumserver != vnumclient){ //MANIFEST VERSIONS ARE DIFFERENT

            printf("Client And Server Manifest Versions Do Not Match! Update + Upgrade The Local Project First!\n");
                close(clientfd);
                close(serverfd);
                remove(".ManifestServer");
                chdir("..");
                return;
    }

    else{
              cfd = open(".Commit", O_CREAT | O_WRONLY | O_TRUNC, 0644);
              close(cfd);

              //printf("\n - STARTING TO SERVER TO CLIENT LOOKUPS - \n" );

              while(readbytes < serverfdlength){ //FIRST LOOK AT SERVER ENTRIES
                      bzero(fullentrybuffer, 1024);
                      bzero(filepathbuffer, 1024);
                      bzero(hashbuffer, 1024);
                      bzero(versionbuffer, 16);
                      newlines = 0;

                      while(newlines != 5){//LOOP TO GET EACH ENTRY IN THE SERVER MANIFEST
                          read(serverfd, &minientrybuffer, 1);
                              if((newlines == 1) && (minientrybuffer != '\n')){
                                  strncat(filepathbuffer, &minientrybuffer, 1);
                              }
                                    if((newlines == 2) && (minientrybuffer != '\n')){
                                        strncat(hashbuffer, &minientrybuffer, 1);
                                    }
                                          if((newlines == 3) && (minientrybuffer != '\n')){
                                              strncat(versionbuffer, &minientrybuffer, 1);
                                          }
                                                if(minientrybuffer == '\n'){
                                                  newlines++;
                                                }
                          strncat(fullentrybuffer, &minientrybuffer, 1);
                          readbytes++;
                      }

                        hashbuffer[33] = '\0';
                        //printf("SERVER ENTRY :\n|%s|\n", fullentrybuffer);
                        //printf("\n-E-N-T-R-Y---4---C-O-M-M-I-T-\n");
                        //printf("\tPATH : |%s|\n", filepathbuffer);
                        //printf("\tHASH : |%s|\n", hashbuffer);
                        //printf("\tVERSION : |%s|\n\n", versionbuffer);

                        //COMPARING SERVER ENTRIES TO CLIENT
                        char* ptr = NULL;
                        if((ptr = strstr(clientmanifest, filepathbuffer)) != NULL){ //SEARCH FOR A PATH FROM SERVER BEING CONTAINED IN CLIENT - IF FOUND
                            //printf("Found %s (A Server Entry) In Client Manifest\n", filepathbuffer);

                            //PSC1 MODIFY CASE
                            //FIND IF VERSION AND STORED HASH IS DIFF FROM SERVER
                            ptr = ptr + strlen(filepathbuffer) + 1; //+1 Is The Newline
                            char clienthash[33]; char clientversion[16];
                            strncpy(clienthash, ptr, 32);
                            clienthash[32] = '\0';

                                ptr = ptr + 33;
                                while(ptr[0] != '\n'){
                                      strncpy(clientversion, ptr, 1);
                                      ptr = ptr + 1;
                                }

                                      chdir("..");
                                      char* livehash = makehash(filepathbuffer);
                                      chdir(projectName);
/*
                                printf("S Hash = |%s|\n", hashbuffer);//server
                                printf("C Hash = |%s|\n", clienthash);
                                printf("L Hash = |%s|\n", livehash);
                                //printf("Client Version = |%s|\n", clientversion);
                                //printf("Server Version = |%s|\n", versionbuffer);
*/
                                    if(strcmp(hashbuffer, clienthash) == 0){//IDENTICAL CLIENT HASH VS SERVER HASH
                                            //printf("Hashes Are Identical Between Client And Server\n");

                                              if(strcmp(clienthash, livehash) != 0){//COMPARE CLIENT AND LIVE HASH
                                                      //printf("Hash In Manifests ARE NOT Identical To The Live Hash For %s.\n", filepathbuffer);

                                                      //NEED TO INCREMENT VERSION
                                                      int incremented = atoi(clientversion);
                                                      incremented++;
                                                      char strincrem[16];
                                                      snprintf(strincrem, 16, "%d", incremented);
                                                      //printf("INCREMENTED FILE VERSION TO WRITE : %d | STRING : %s\n", incremented, strincrem);

                                                      cfd = open(".Commit", O_CREAT | O_WRONLY | O_APPEND, 0644);
                                                          write(cfd, "M ", 2);
                                                          write(cfd, filepathbuffer, strlen(filepathbuffer));
                                                          write(cfd, " ", 1);
                                                          write(cfd, livehash, strlen(livehash));
                                                          write(cfd, " ", 1);
                                                          write(cfd, strincrem, strlen(strincrem));
                                                          write(cfd, "\n", 1);
                                                      close(cfd);
                                                      printf("\tM %s\n", filepathbuffer);
                                              }//COMPARE CLIENT AND LIVE HASH
                                    }//IDENTICAL CLIENT HASH VS SERVER HASH

                                    else{//DIFFERENT CLIENT AND SERVER HASH
                                            int servernum = atoi(versionbuffer);
                                            int clientnum = atoi(clientversion);

                                            if(servernum > clientnum){
                                                  printf("Commit Failed. The Client Must Sync With The Repository Before Committing Changes.\n");
                                                  remove(".Commit");
                                                  remove(".ManifestServer");
                                                      int status = 1;
                                                      write(socketfd, &status, sizeof(status));
                                                  return;
                                            }

                                    }//ELSE

                        }//SEARCH FOR A PATH FROM SERVER BEING CONTAINED IN CLIENT - IF FOUND

                        else{ //IF NOT FOUND
                              ptr = strstr(servermanifest, filepathbuffer);
                              ptr = ptr + strlen(filepathbuffer) + 1; //+1 Is The Newline
                              char clienthash[33]; char clientversion[16];
                              strncpy(clienthash, ptr, 32);
                              clienthash[33] = '\0';

                                  ptr = ptr + 33;
                                  while(ptr[0] != '\n'){
                                        strncpy(clientversion, ptr, 1);
                                        ptr = ptr + 1;
                                  }

                                        int incremented = atoi(clientversion);
                                        incremented++;
                                        char strincrem[16];
                                        snprintf(strincrem, 16, "%d", incremented);
                                        //printf("INCREMENTED FILE VERSION TO WRITE : %d | STRING : %s\n", incremented, strincrem);

                                //printf("Did Not Find %s (A Server Entry) In Client Manifest\n", filepathbuffer);
                                cfd = open(".Commit", O_CREAT | O_WRONLY | O_APPEND, 0644);
                                    write(cfd, "D ", 2);
                                    write(cfd, filepathbuffer, strlen(filepathbuffer));
                                    write(cfd, " ", 1);
                                    write(cfd, hashbuffer, strlen(hashbuffer));
                                    write(cfd, " ", 1);
                                    write(cfd, strincrem, strlen(strincrem));
                                    write(cfd, "\n", 1);
                                close(cfd);
                                printf("\tD %s\n", filepathbuffer);
                        }
            }//WHILE READBYTES

            //printf("\n - SWITCHING TO CLIENT TO SERVER LOOKUPS - \n" );

            while(readbytes2 < clientfdlength){ //NEXT LOOK AT CLIENT ENTRIES
                    bzero(fullentrybuffer, 1024);
                    bzero(filepathbuffer, 1024);
                    bzero(hashbuffer, 1024);
                    bzero(versionbuffer, 16);
                    newlines = 0;

                    while(newlines != 5){//LOOP TO GET EACH ENTRY IN THE SERVER MANIFEST
                        read(clientfd, &minientrybuffer, 1);
                            if((newlines == 1) && (minientrybuffer != '\n')){
                                strncat(filepathbuffer, &minientrybuffer, 1);
                            }
                                  if((newlines == 2) && (minientrybuffer != '\n')){
                                      strncat(hashbuffer, &minientrybuffer, 1);
                                  }
                                        if((newlines == 3) && (minientrybuffer != '\n')){
                                            strncat(versionbuffer, &minientrybuffer, 1);
                                        }
                                              if(minientrybuffer == '\n'){
                                                newlines++;
                                              }
                        strncat(fullentrybuffer, &minientrybuffer, 1);
                        readbytes2++;
                    }
                      //printf("SERVER ENTRY :\n|%s|\n", fullentrybuffer);
                      //printf("\n-E-N-T-R-Y---4---C-O-M-M-I-T-\n");
                      //printf("\tC PATH : |%s|\n", filepathbuffer);
                      //printf("\tC HASH : |%s|\n", hashbuffer);
                      //printf("\tC VERSION : |%s|\n\n", versionbuffer);

                      //COMPARING SERVER ENTRIES TO CLIENT
                      char* ptr = NULL;
                      if((ptr = strstr(servermanifest, filepathbuffer)) != NULL){ //SEARCH FOR A PATH FROM SERVER BEING CONTAINED IN CLIENT - IF FOUND
                          //printf("Found %s (A Client Entry) In Server Manifest\n", filepathbuffer);


                      }//SEARCH FOR A PATH FROM SERVER BEING CONTAINED IN CLIENT - IF FOUND

                      else{ //IF NOT FOUND ---> PSC2 ADD CASE
                            ptr = strstr(clientmanifest, filepathbuffer);
                            ptr = ptr + strlen(filepathbuffer) + 1; //+1 Is The Newline
                            char clienthash[33]; char clientversion[16];
                            strncpy(clienthash, ptr, 32);
                            clienthash[33] = '\0';

                                ptr = ptr + 33;
                                while(ptr[0] != '\n'){
                                      strncpy(clientversion, ptr, 1);
                                      ptr = ptr + 1;
                                }

                                      int incremented = atoi(clientversion);
                                      incremented++;
                                      char strincrem[16];
                                      snprintf(strincrem, 16, "%d", incremented);

                                      //printf("INCREMENTED FILE VERSION TO WRITE : %d | STRING : %s\n", incremented, strincrem);
                                      cfd = open(".Commit", O_CREAT | O_WRONLY | O_APPEND, 0644);
                                          write(cfd, "A ", 2);
                                          write(cfd, filepathbuffer, strlen(filepathbuffer));
                                          write(cfd, " ", 1);
                                          write(cfd, hashbuffer, strlen(hashbuffer));
                                          write(cfd, " ", 1);
                                          write(cfd, strincrem, strlen(strincrem));
                                          write(cfd, "\n", 1);
                                      close(cfd);
                                      printf("\tA %s\n", filepathbuffer);

                              //printf("Did Not Find %s (A Client Entry) In Server Manifest\n", filepathbuffer);
                      }
          }//WHILE READBYTES2

          int status = 0;
          write(socketfd, &status, sizeof(status));
          printf("Sending The Server The .Commit File.\n");

          //NOW WRITE COMMIT TO SERVER TO DOWNLOAD
          int filesbeingsent = 1;
          int fl = getfilelength(".Commit"); //File Length
          int fnl = strlen(".Commit"); //File Name Length
              write(socketfd, &filesbeingsent, sizeof(filesbeingsent));//Sending # Files Being Sent
              write(socketfd, &fnl, sizeof(fnl));//Sending # Bytes In File Name
              write(socketfd, ".Commit", fnl);//Sending File Name
              write(socketfd, &fl, sizeof(fl));//Sending File Length

          int i = 0;
          char toSend;
          cfd = open(".Commit", O_RDONLY);
          for(i = 0; i < fl; i++){
                read(cfd, &toSend, 1);
                write(socketfd, &toSend, 1);
          }//For
          close(cfd);
          printf("Commit Completed Succesfully.\n");

    }//ELSE

    remove(".ManifestServer");
    close(clientfd);
    close(serverfd);
    chdir("..");

}//COMMIT


//-------------------------------------------------------------------------------------| PUSH

void push(char* projectName, int socketfd){

  /*
        Client should send .CommitCompare to the server - commit should exist, if not, ERROR
        Server compares .Commit and .CommitCompare ---> IF THEY ARE THE SAME, server requests files that need to be changed
        After files are recieved, server should update its own project with those files and the manifest file

        On success, client and server manifest should increment their project and file versions, update hashes, and remove status codes.
        After update, expire/delete .Commit. Client should erase .Commit Regardless of failure or succes.
  */

    chdir(projectName);

    int status = 1;
    read(socketfd, &status, sizeof(status));
    if(status == 1){//NO .COMMIT ON SERVER
          printf("Could Not Complete A Push (The Server Does Not Have A .Commit File... Call 'commit' On The Client).\n");
          chdir("..");
          return;
    }
    else{
          //printf("Server Had A .Commit File\n");
    }

    status = 1;
    int fd = open(".Commit", O_RDONLY);
        if(fd == -1){
              printf("Could Not Complete A Push (The Client Does Not Have A .Commit File).\n");
              write(socketfd, &status, sizeof(status)); //WRITE 1
              close(fd);
              chdir("..");
              return;
        }
        //ELSE FD IS OPEN
                    close(fd);
                    status = 0;
                    write(socketfd, &status, sizeof(status));//WRITE 0
                    printf("Sending The Server The .Commit File.\n");

                    //NOW WRITE COMMIT TO SERVER TO DOWNLOAD
                    int filesbeingsent = 1;
                    int fl = getfilelength(".Commit"); //File Length
                    int fnl = strlen(".CommitCompare"); //File Name Length
                        write(socketfd, &filesbeingsent, sizeof(filesbeingsent));//Sending # Files Being Sent
                        write(socketfd, &fnl, sizeof(fnl));//Sending # Bytes In File Name
                        write(socketfd, ".CommitCompare", fnl);//Sending File Name
                        write(socketfd, &fl, sizeof(fl));//Sending File Length
                    int i = 0;
                    char toSend;
                    fd = open(".Commit", O_RDONLY);
                    for(i = 0; i < fl; i++){
                          read(fd, &toSend, 1);
                          write(socketfd, &toSend, 1);
                    }//For
                    close(fd);
                    printf(".Commit Succesfully Sent To Server.\n");

      //AWAIT TO SEE IF FILES ON SERVER ARE THE Same
      read(socketfd, &status, sizeof(status));
      if(status == 1){
            printf("The Commit That The Server Had Was Not The Same As The One The Client Sent! Could Not Push.\n");
            chdir("..");
            return;
      }

      //NOW THE CLIENT CAN UPDATE MANIFEST VERSION
      int ver_int = 0;
      read(socketfd, &ver_int, sizeof(ver_int));//GET THE INCREMENT NUMBER FROM THE SERVER
          char strincrem[16] = "";
          snprintf(strincrem, 16, "%d", ver_int);

      //printf("FILE INCREMENT VER = %s\n", strincrem);

            fd = open(".Manifest", O_WRONLY);
                write(fd, "           ", 11);
            close(fd);
            fd = open(".Manifest", O_WRONLY);
                write(fd, strincrem, strlen(strincrem));
            close(fd);

    //-------------------------------------------------------------- AT THIS POINT, SERVER IS PARSING .COMMIT, AND MUST REQUEST CERTAIN THINGS

       char upop;
       while(1){
            char filesend[256] = "";
            bzero(filesend, 256);
            read(socketfd, &upop, 1);//READS THE OPERATION EACH TIME
            //printf("Operation = %c\n", upop);

            if(upop == 'A' || upop == 'M'){
                    //printf("%c\n", upop);
                    read(socketfd, filesend, 256);
                    printf("\tSending The Server '%s'\n", filesend);

                    int filesbeingsent = 1;
                    int filelength = getfilelength(filesend); //File Length
                    int namelength = strlen(filesend); //File Name Length
                        write(socketfd, &filesbeingsent, sizeof(filesbeingsent));//Sending # Files Being Sent
                        write(socketfd, &namelength, sizeof(namelength));//Sending # Bytes In File Name
                        write(socketfd, filesend, namelength);//Sending File Name
                        write(socketfd, &filelength, sizeof(filelength));//Sending File Length

                        int i = 0;
                        char toSend;
                        int sendfd = open(filesend, O_RDONLY);
                                  if(sendfd == -1){
                                       printf("Error Pushing - Could Not Find File %s To Add/Modify!\n", filesend);
                                  }
                                  for(i = 0; i < filelength; i++){
                                        read(sendfd, &toSend, 1);
                                        //printf("Sending Byte : %c\n", toSend);
                                        write(socketfd, &toSend, 1);
                                  }//For
                        close(sendfd);
            }//ADD

            else if(upop == 'D'){

            }//DELETE

            else if(upop == 'C'){
                    printf("Server Completed Push\n");
                    break;

            }//DELETE

    }//WHILE PARSING

    //CLIENT NEEDS TO UPDATE FILE VERSIONS IN MANIFEST
    remove(".Manifest");
    download_files(socketfd);//DOWNLOAD ManifestServer
    rename(".ManifestServer", ".Manifest");
    remove(".Commit");

    chdir("..");

}//PUSH

//------------------------------------------------------------------------------------------------------------| History

void history(char* projectName, int socketfd){

    int response = 0;
    read(socketfd, &response, sizeof(response));

    if(response == 1){//NO HISTOPRY
          printf("The Project Requested Does Not Have A Commit History.\n");
          return;
    }
    else{
          printf("Downloading Commit History...\n");
    }

          download_files(socketfd);//DOWNLOAD .HISTORY

          printf("[START OF COMMIT HISTORY]\n");

          int fl = getfilelength(".History"); //File Length
          int hfd = open(".History", O_RDONLY);

                char readby; int i = 0;
                for(i = 0; i < fl; i++){
                      read(hfd, &readby, 1);
                      printf("%c", readby);
                }//For
                close(hfd);

          printf("[END OF COMMIT HISTORY]\n");

    remove(".History");

}
