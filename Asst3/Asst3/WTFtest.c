#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>

//gcc WTF.c -lssl -lcrypto -o WTF

int main(int argc, char** argv){

      // THIS PROGRAM ONLY RUNS IF THE WTF, WTFserver, AND WTFtest ARE ALL IN THE SAME DIRECTORY!!!!
      // OTHERWISE IT WILL NOT WORK ---> IT RELIES ON THE CWD TO CREATE NEW FILES FOR TESTING
      //USER MUST ENTER A PORT NUM TO START THE SERVER ON

      char* port;

      if(argc != 2){
            printf("WTFtest Arguments Not Given Properly.\nPlease Include A Port Number To Start The Server On.");
            return;
      }
      else{
            port = argv[1];
      }

      //COMMAND STRING ---> USING STRCPY() TO PASS COMMANDS TO IT, THEN USING SYSTEM TO EXECUTE THEM.
      char command [1024] = "";

      //START THE SERVER ---> PORT ARGV[1]
      strcpy(command, "./WTFserver ");
      strcat(command, port);
      strcat(command, " &");
      printf("*TEST IS EXECUTING \"%s\"*\n", command);
            system(command);

      //START CLIENT Commands ---> LOCALHOST WITH PORT GIVEN
      strcpy(command, "./WTF configure 127.0.0.1 ");
      strcat(command, port);
      printf("*TEST IS EXECUTING \"%s\"*\n", command);
            system(command);

      strcpy(command, "./WTF create x");
      printf("*TEST IS EXECUTING \"%s\"*\n", command);
            system(command);

                        //CREATE NEW FILES TO ADD TO PROJECT
                        chdir("Client");
                        chdir("x");
                              system("touch ONE.txt TWO.txt");
                                    mkdir("DIR", 0777);
                                    chdir("DIR");
                                          system("touch THREE.txt");
                                          chdir("..");//BACK TO PROJECT FOLDER
                                          chdir("..");//BACK TO CLIENT FOLDER
                                          chdir("..");//BACK TO DIRECTORY WHERE EXECUTABLE IS


      strcpy(command, "./WTF add x ONE.txt");
      printf("*TEST IS EXECUTING \"%s\"*\n", command);
            system(command);

      strcpy(command, "./WTF add x DIR/THREE.txt");
      printf("*TEST IS EXECUTING \"%s\"*\n", command);
            system(command);

      strcpy(command, "./WTF commit x");
      printf("*TEST IS EXECUTING \"%s\"*\n", command);
            system(command);

      strcpy(command, "./WTF push x");
      printf("*TEST IS EXECUTING \"%s\"*\n", command);
            system(command);

      //NOW THAT COMMIT AND PUSH HAVE GONE THROUGH, DELETE THE PROJECT ON SERVER SIDE TO CHECK OUT / DOWNLOAD
      chdir("Client");
          system("rm -rf x");
      chdir("..");

      strcpy(command, "./WTF checkout x");
      printf("*TEST IS EXECUTING \"%s\"*\n", command);
            system(command);

                      //CHANGING FILES FOR ANOTHER PUSH
                      strcpy(command, "./WTF add x TWO.txt");
                      printf("*TEST IS EXECUTING \"%s\"*\n", command);
                            system(command);

                      strcpy(command, "./WTF remove x ONE.txt");
                      printf("*TEST IS EXECUTING \"%s\"*\n", command);
                            system(command);

                      //MODIFY THREE.TXT
                      int fd = open("./Client/x/DIR/THREE.txt", O_WRONLY);
                              write(fd, "This Text Has Been Added!", 25);
                      close(fd);

      strcpy(command, "./WTF commit x");
      printf("*TEST IS EXECUTING \"%s\"*\n", command);
            system(command);

      strcpy(command, "./WTF push x");
      printf("*TEST IS EXECUTING \"%s\"*\n", command);
            system(command);

      strcpy(command, "./WTF history x");
      printf("*TEST IS EXECUTING \"%s\"*\n", command);
            system(command);

      strcpy(command, "./WTF currentversion x");
      printf("*TEST IS EXECUTING \"%s\"*\n", command);
            system(command);

            strcpy(command, "./WTF commit x");
            printf("*TEST IS EXECUTING \"%s\"*\n", command);
                  system(command);

                        strcpy(command, "./WTF push x");
                        printf("*TEST IS EXECUTING \"%s\"*\n", command);
                              system(command);

            strcpy(command, "./WTF commit x");
            printf("*TEST IS EXECUTING \"%s\"*\n", command);
                  system(command);

                        strcpy(command, "./WTF push x");
                        printf("*TEST IS EXECUTING \"%s\"*\n", command);
                              system(command);

          //REMOVE CLIENT FILE SO WE CAN CHECKOUT
          system("rm -rf ./Client/x");

      strcpy(command, "./WTF checkout x");//CLIENT IS NOW MOST RECENT VERSION
      printf("*TEST IS EXECUTING \"%s\"*\n", command);
            system(command);

      strcpy(command, "./WTF update x");
      printf("*TEST IS EXECUTING \"%s\"*\n", command);
            system(command);

      strcpy(command, "./WTF upgrade x");
      printf("*TEST IS EXECUTING \"%s\"*\n", command);
            system(command);

      strcpy(command, "./WTF destroy x");
      printf("*TEST IS EXECUTING \"%s\"*\n", command);
            system(command);

      printf("\n*TESTING IS COMPLETED.*\n*IF SERVER IS STILL RUNNING FOR SOME REASON, CALL SIGINT (CTRL+C) TO SHUT SERVER DOWN!*\n");
      printf("*IF SERVER SHUT DOWN ON ITS OWN (LIKELY DUE TO IT BEING RUN AS A BACKGROUND PROCESS),\nTHE PORT MAY OR MAY NOT BE ABLE TO BE BOUND TO THAT PORT AGAIN...*\n");
      printf("\n*TESTER MAY NEED TO DO 'kill <PID> IN TERMINAL TO BE ABLE TO REBIND TO THAT PORT!*\n");

}//MAIN
