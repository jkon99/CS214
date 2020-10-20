#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>

/*
IMPLEMENTATION NOTES
    Command Line Format : ./fileCompressor <flag> <path or file> |codebook|
        Potential Flags :
            'b' = Build Codebook (flag_b)
                        Index file indicated, output a "HuffmanCodebook" file in same directory fileCompressor is invoked in
                            ./fileCompressor <flag> <filename>
            'c' = Compress (flag_c)
                        Compress the file indicated using the codebook in to <filename>.hcz, alongside the original file
                            ./fileCompressor <flag> <filename> |codebook|
            'd' = Decompress (flag_d)
                        Decompresses the file indicated (<filename>.hcz) using the given codebook, into <filename>, alongside original file
                            ./fileCompressor <flag> <filename>.hcz |codebook|
            'R' = Recursive (flag_R)
                        Including this flag means that the file argument WILL BE A DIRECTORY INSTEAD. The program should descend into that
                        directory + all subdirectories, recursively applying the given operations to all files found.
                            ./fileCompressor <flag> <directory> |codebook optional (depending on flags)|
    Huffman Codebook Format :
        <escape character>
        <ASCII bytestring> <tab> <token> <newline>
        ...
        <blank line - newline terminated>
            EXAMPLE :
                \
                0    and
                100  cat
                101  button
                1100 a
                1101 dog
                111  ball
                <blank line - newline terminated>
*/

//|---------------------------------------------------------------| LINKED LIST ---> "linked" = Node For Linked List

struct linked{
    int freq; //frequency of token
    char *token;
    char* ascii;
    struct linked *next;
};

typedef struct linked linked;
    linked *head = NULL;

//|---------------------------------------------------------------| MIN-HEAP ---> "heapNode" = Node For Min-Heap

struct heapNode{
    int freq;
    char *token;
    struct heapNode *left;
    struct heapNode *right;
};

typedef struct heapNode heapNode;
    heapNode *root = NULL;

struct heap {
    int size;
    int capacity;
    struct heapNode** array; //Array of minheap node pointers
};

typedef struct heap heap;
    heap *hp = NULL;

//|---------------------------------------------------------------| METHOD LIST

void dir_contents(char* rootpath, int flag_b, int flag_c, int flag_d, char del);
void tokenize(char* filename);
void linkedinsert(char *str);
void listprint();
void freelist(linked* head);
int countlist(linked* head, int print);
//void visualizeHuffman(heapNode* huffmanRoot);
heapNode* newNode(char* token, int freq); //Make a new node for the minheap for the given token and its frequency
void swap(heapNode** a, heapNode** b); //Swap two min heap nodes
void minHeapify(heap* heap, int index); //Heapify function
heapNode* extractMin(heap* heap); //Get minimum value node from heap
void insertMinHeap(heap* heap, heapNode* heapNode); //Insert new node to minheap
void buildMinHeap(heap* heap); //Build min heap
heap* createAndBuildMinHeap(int size); //Make min heap of capacity size
heapNode* buildHuffmanTree(heap* heap, int size); //Builds the huffman tree
void writeCodebook();
void generateBytestrings(heapNode* node, char* ascii);
void compress(char* filename, char* codebookname);  //compress using -c flag, taking in the descriptors of the file and codebook
void compressed_tokenize(char* filename);
char decompress(char* codebookname); //decompress using -d flag, taking in the descriptors of the file comrpessed hcz and the codebook
void recreate(char* token, char* ascii);
void traverse(char* filename, char del);
void freeheap(heapNode* node);

//|-------------------------------------------------------------------------------------------------------------------| MAIN

int main(int argc, char** argv){

    //Flag List
    //0 = INACTIVE ... 1 = ACTIVE
    int flag_b = 0;
    int flag_c = 0;
    int flag_d = 0;
    int flag_R = 0;

    //Directory
    DIR *dir;
    struct dirent *dent;
    heapNode* huffmanRoot;

    char* dirpath; //Directory Given As Argument (If Supplied)
    char* filename; //File Given As Argument (If Supplied)
    char* codebookname; //Codebook (If Supplied)

    int fd; //General File Descriptor
    int cb; //Codebook File Descriptor

    int listnum = 0; //# Elements In Linked List

//|---------------------------------------------------------------| Finding Flags In Argument

    int i;
    for(i = 0; i < argc; i++){
        //Flag Recognition
        if(argv[i][0] == '-'){
                if((argv[i][1] == 'b') && (argv[i][2] == '\0')){
                  flag_b = 1;
                }
                else if((argv[i][1] == 'c') && (argv[i][2] == '\0')){
                  flag_c = 1;
                }
                else if((argv[i][1] == 'd') && (argv[i][2] == '\0')){
                  flag_d = 1;
                }
                else if((argv[i][1] == 'R') && (argv[i][2] == '\0')){
                  flag_R = 1;
                }
      }//If '-'
    }

    if(flag_b + flag_c + flag_d > 1){
        printf("ERROR : Multiple flags present, when only one ('-b', '-c', or '-d') must be present!\n");
        return EXIT_FAILURE;
    }
    if(flag_b + flag_c + flag_d == 0){
        printf("ERROR : No flags were present! Include either '-b', '-c', or '-d'.\n");
        return EXIT_FAILURE;
    }

//|---------------------------------------------------------------| DIRECTORY PROCRESSING

    if(flag_R == 1){
          if(flag_b == 1){
                  dirpath = argv[argc - 1];
                  dir = opendir(dirpath);
                      if(dir == NULL) {
                          printf ("ERROR : Cannot Open Directory (Make sure argument formatting is correct)\n");
                          return EXIT_FAILURE;
                      }
                      else{
                          //printf("RECOGNIZED '%s' DIRECTORY\n", argv[argc - 1]);
                      }
          }//If
          else if((flag_c == 1) || (flag_d == 1)){
                  dirpath = argv[argc - 2];
                  dir = opendir(dirpath);
                  codebookname = argv[argc - 1];
                  cb = open(argv[argc - 1], O_RDONLY);
                      if(dir == NULL) {
                          printf ("ERROR : Cannot Open Directory (Make sure argument formatting is correct)\n");
                          return EXIT_FAILURE;
                      }
                      else{
                            //printf("RECOGNIZED '%s' DIRECTORY\n", argv[argc - 2]);
                            //printf("RECOGNIZED '%s' CODEBOOK\n", codebookname);

                            cb = open(codebookname, O_RDONLY);
                            if(cb == -1){
                              printf("ERROR : Opening Codebook (Make sure argument formatting is correct)\n");
                              return EXIT_FAILURE;
                            }
                            close(cb);
                      }
          }//Else
  }

//|---------------------------------------------------------------| FILE PROCRESSING

    else if(flag_R == 0){
          if(flag_b == 1){
              filename = argv[argc - 1];

              fd = open(filename, O_RDONLY);
                if(fd == -1){
                    printf("ERROR : Opening File '%s'\n\t(Perhaps it does not exist)!\n", filename);
                    return EXIT_FAILURE;
                }
                close(fd);
          }
          else if((flag_c == 1) || (flag_d == 1)){
                filename = argv[argc - 2];
                codebookname = argv[argc - 1];

                fd = open(filename, O_RDONLY);
                if(fd == -1){
                    printf("ERROR : File Supplied Does Not Exist!\n");
                    return EXIT_FAILURE;
                }
                close(fd);

                cb = open(codebookname, O_RDONLY);
                char test;
                if(cb == -1 || read(cb, &test, 1) == 0){
                    printf("ERROR : Opening Codebook '%s'\n\t(Perhaps it does not exist)!\n", codebookname);
                    return EXIT_FAILURE;
                }
                close(cb);

          }
  }

//|---------------------------------------------------------------| DIAGNOSTIC INFORMATION PRINTS
//printf("\nFlags Active :\n\t'b' | %d (Build Codebook)\n\t'c' | %d (Compress)\n\t'd' | %d (Decompress)\n\t'R' | %d (Recursive)\n", flag_b, flag_c, flag_d, flag_R);

//OPERATIONS FOR DIRECTORIES
  if(flag_R == 1){

  char del = '^';

     if(flag_d == 1) {
        del = decompress(codebookname);
     }

    if(dirpath[strlen(dirpath) - 1] == '/'){
        dirpath[strlen(dirpath) - 1] = '\0';
    }

    //printf("\nDirectory %s .:.\n", dirpath);

    dir_contents(dirpath, flag_b, flag_c, flag_d, del);

    if(flag_b == 1){
        tokenize(filename);
        listnum = countlist(head, 0);
        hp = createAndBuildMinHeap(listnum);
        huffmanRoot = buildHuffmanTree(hp, listnum);
        //listprint();
        char* gen = malloc(64 * sizeof(char));
        gen = "";
        generateBytestrings(huffmanRoot, gen);
        writeCodebook();
        //listprint();
        freeheap(huffmanRoot);
    }

     if(flag_d == 1) {
        freeheap(root);
     }

        //LOOK INSIDE DIR_CONTENTS -> PERFORM OPERATIONS FOR FLAGS WITHIN THERE?

  } //IF -R

//|---------------------------------------------------------------| HEAP CREATION

//OPERATIONS FOR FILES
  if(flag_R == 0){

    //printf("\nFILE %s .:.\n", filename);

    if(flag_b == 1){
        tokenize(filename);
        listnum = countlist(head, 0);
        hp = createAndBuildMinHeap(listnum);
        huffmanRoot = buildHuffmanTree(hp, listnum);
        //listprint();
        char* gen = malloc(64 * sizeof(char));
        gen = "";
        generateBytestrings(huffmanRoot, gen);
        writeCodebook();
        //listprint();
        freeheap(huffmanRoot);
    }

    if(flag_c == 1) {
        compress(filename, codebookname);
        //listprint();
    }
    if(flag_d == 1) {
        char del = decompress(codebookname);

            if(strstr(filename, ".hcz") != NULL){
                traverse(filename, del);
            }
            else{
                printf("ERROR : The File Supplied Does Not Have The .hcz Extention! Cannot Decompress!\n");
                return EXIT_FAILURE;
            }

        freeheap(root);
    }

  }//IF NOT -R

  freelist(head);

  return EXIT_SUCCESS;
}//MAIN

//|-------------------------------------------------------------------------------------------------------------------| DIR_CONTENTS

//Prints out the contents of all directories from our root directory

void dir_contents(char* rootpath, int flag_b, int flag_c, int flag_d, char del){
    char newpath[1024];
    memset(newpath, '\0', 1024);
    strcpy(newpath, rootpath);
    struct dirent *dent;
    DIR *dir = opendir(rootpath);
    int i = 0;

    if(dir == NULL){
        //printf("ERROR : Directory Is NULL!\n");
        return;
    }

    while((dent = readdir(dir)) != NULL){
        //Does not include self or parent directories
        if((strcmp(dent->d_name, "..") != 0) && (strcmp(dent->d_name, ".") != 0)){

            //UNCOMMENT THIS PRINTF AND COMMENT OUT TOKENIZE (BELOW) IF YOU WANT TO JUST SEE THE LIST OF ALL FILES IN THE SUPPLIED DIRECTORY
            //printf("\t%s\n", dent->d_name);

            //Concat Newpath To Traverse From Rootpath
            strcpy(newpath, rootpath);
            strcat(newpath, "/");
            strcat(newpath, dent->d_name);

            if(strstr(newpath, "HuffmanCodebook") == NULL && strstr(newpath, ".hcz") == NULL){ //Tokenize Everything BESIDES Codebook
                //printf("NEWPATH : %s\n", newpath);

                if(flag_b == 1){
                    //printf("TOKENIZING\n");
                    tokenize(newpath);
                }
                else if(flag_c == 1){
                    //printf("COMPRESSING \n");
                    compress(newpath, "HuffmanCodebook");
                }
            }
            else if(strstr(newpath, "HuffmanCodebook") == NULL && strstr(newpath, ".hcz") != NULL){

                if(flag_d == 1){
                    //printf("DECOMPRESSING \n");
                    traverse(newpath, del);
                    i = 1;
                }

            }

            dir_contents(newpath, flag_b, flag_c, flag_d, del);
        }//IF NOT .. OR .
    }//WHILE

    if(i == 0 && flag_d == 1){
        printf("ERROR : There were no .hcz files in the directory! Cannot Decompress!");
    }

    closedir(dir);
}//DIR_CONTENTS

//|-------------------------------------------------------------------------------------------------------------------| TOKENIZE

void tokenize(char* filename){

  int fd = open(filename, O_RDONLY);
      if(fd == -1){
          //If fd = -1, Directory Was Supplied / File DNE ---> EXIT FUNCTION
          //printf("\nError Opening File '%s'", filename);
          return;
      }
      else{
           //printf("\n'%s'\n", filename);
      }

  int buffersize = 10;
  char* buffer = malloc(sizeof(char) * buffersize);

  //------------------------------------------------------------| FINDING BUFFER SIZE

  int rd = read(fd, buffer, buffersize);
      if(rd == 0){
          printf("\nWARNING : File %s is empty! Cannot process any data on it.", filename);
          return;
      }

      int loops = 0;
      while(rd != 0){
          rd = read(fd, buffer, buffersize);
          loops++;
          //printf("\t%d\t|\t%d\t|\t%d\t\n", rd, loops, (buffersize * loops));
          }
      close(fd);

      buffersize = buffersize * loops;
      buffer = malloc(sizeof(char) * buffersize);
          fd = open(filename, O_RDONLY);
          rd = read(fd, buffer, buffersize);
          close(fd);

//------------------------------------------------------------| FINDING BUFFER SIZE

  int i = 0;
  int j = 0;
  int k = 0;
  char str[buffersize];
      str[0] = '\0';
      buffer[rd] = '\0';

  for(i = 0; i <= rd; i++){
            //printf("\nBuffer[%d] = \'%c\'", i, buffer[i]);
            if((buffer[i] != ' ') && (buffer[i] != '\n') && (buffer[i] != '\t') && (buffer[i] != '\0')){
                str[j + 1] = '\0';
                str[j] = buffer[i];
                j++;
            }

            else{ //EXPORT TOKEN TO LINKED LIST (TERMINATOR CHARACTER WAS REACHED)
                //printf("\t%s\n", str);
                    if(str[0] != '\0'){ //STRING HAS CONTENT
                        linkedinsert(str);
                    }
                    if((buffer[i] == ' ') || (buffer[i] == '\n') || (buffer[i] == '\t') || (buffer[i] != '\0')){
                            str[0] = '\0';
                                for(k = 1; k < j; k++){
                                    str[k] = '\0';}
                            j = 0;
                                    if(buffer[i] == ' '){
                                        str[0] = '^';
                                        str[1] = 's';
                                        str[2] = '\0';
                                    }
                                    else if(buffer[i] == '\n'){
                                        str[0] = '^';
                                        str[1] = 'n';
                                        str[2] = '\0';
                                    }
                                    else if(buffer[i] == '\t'){
                                        str[0] = '^';
                                        str[1] = 't';
                                        str[2] = '\0';
                                    }
                        linkedinsert(str);
                    }

                str[0] = '\0';
                    for(k = 1; k < j; k++){
                        str[k] = '\0';}
                j = 0;
            }
  }//For
  free(buffer);
}//TOKENIZE

//|-------------------------------------------------------------------------------------------------------------------| INSERT (LINKED LIST)

void linkedinsert(char *token){

    linked* tempnode = malloc(sizeof(linked));
        tempnode->token = malloc(sizeof(char) * strlen(token));
        strcpy(tempnode->token, token);
        tempnode->freq = 1;
        tempnode->ascii = "";

    if(head == NULL){
        head = tempnode;
        (*head).next = NULL;
    }
    else{
        linked *ptr = head;

        while(ptr->next != NULL){
                //Token Being Inserted Exists In LL Already
                if(strcmp(ptr->token, token) == 0){
                    free(tempnode);
                    ptr->freq = ptr->freq + 1;
                    return;
                }
                ptr = ptr->next;
                        //Edge Case Where Last LL Node Is The Similar Token
                        if(ptr->next == NULL){
                            if(strcmp(ptr->token, token) == 0){
                                free(tempnode);
                                ptr->freq = ptr->freq + 1;
                                return;
                            }
                        }
        }//WHILE
        tempnode->next = NULL;
        ptr->next = tempnode;
    }//ELSE
}

//|-------------------------------------------------------------------------------------------------------------------| PRINT (LINKED LIST)

void listprint(){

linked *ptr = head;

    if(head == NULL){
         //printf(" - The head is empty! Cannot printlist()! (No Linked List Exists)\n");
         return;
    }
    else{
        printf("\nLinked List (Frequency | Token) :\n");
            while(ptr != NULL){
                printf("\t%d\t|\t'%s' (ASCII = %s)\n", ptr->freq, ptr->token, ptr->ascii);
                ptr = ptr->next;
            }
    }
}

//|-------------------------------------------------------------------------------------------------------------------| FREE (LINKED LIST)

void freelist(linked* head){
    struct linked* temp;
        while (head != NULL){
            temp = head;
            head = head->next;
            free(temp->ascii);
            free(temp->token);
            free(temp);
        }
    return;
}

//|-------------------------------------------------------------------------------------------------------------------| COUNT LL NODES

int countlist(linked* head, int print){

    int count = 0;
    linked *ptr = head;

    while (ptr != NULL){
        ptr = ptr->next;
        count++;
    }

    if(print > 0){ //Prints list size if specified
        printf("\nLIST SIZE : %d\n\n", count);
    }
    return count;
}

//|-------------------------------------------------------------------------------------------------------------------| CREATE AND BUILD MINHEAP

heap* createAndBuildMinHeap(int size) {

    heap* heap = malloc(sizeof(heap));
        heap->size = size;
        heap->capacity = size;
        heap->array = malloc(heap->capacity * sizeof(heapNode*));

    int i = 0;
    linked *ptr = head;

    while(ptr != NULL){
        heap->array[i] = newNode(ptr->token, ptr->freq);
        i++;
        ptr = ptr->next;
    }
    heap->size = size;
    buildMinHeap(heap);

    /*
    for(i = 0; i < size; i++){
        printf("\n(%s|%d)", heap->array[i]->token, heap->array[i]->freq);
    }
    */
    return heap;
}

//|-------------------------------------------------------------------------------------------------------------------| NEW NODE IN HEAP

heapNode* newNode(char* token, int freq) {

    heapNode* temp = malloc(sizeof(heapNode));

    temp->left = NULL;
    temp->right = NULL;

    temp->token = malloc(sizeof(char) * strlen(token));
    strcpy(temp->token, token);
    temp->freq = freq;

    return temp;
}

//|-------------------------------------------------------------------------------------------------------------------| SWAP FOR HEAPS

void swap(heapNode** a, heapNode** b) {

  heapNode* temp = *a;
  *a = *b;
  *b = temp;
}

//|-------------------------------------------------------------------------------------------------------------------| HEAPIFY

void minHeapify(heap* heap, int index) {

    int smallest = index;
    int left = 2 * index +1;  //2n+1
    int right = 2 * index + 2; //2n + 2

    if((left < heap->size) && (heap->array[left]->freq < heap->array[smallest]->freq)){
        smallest = left;
        //printf("SMALLEST IS LEFT\n");
    }

    if((right < heap->size) && (heap->array[right]->freq < heap->array[smallest]->freq)) {
        smallest = right;
        //printf("SMALLEST IS RIGHT\n");
    }

    if(smallest != index) {
        swap(&heap->array[smallest], &heap->array[index]);
        //printf("MINHEAPIFY AGAIN\n");
        minHeapify(heap, smallest);
  }
}

//|-------------------------------------------------------------------------------------------------------------------| EXTRACT MIN

heapNode* extractMin(heap* heap) {
  heapNode* temp = heap->array[0];
      heap->array[0] = heap->array[heap->size-1];
      heap->size = heap->size - 1;
      minHeapify(heap, 0);
  return temp;
}

//|-------------------------------------------------------------------------------------------------------------------| INSERT MINHEAP

void insertMinHeap(heap* heap, heapNode* heapNode) {
    heap->size = heap->size + 1;
    int i = heap->size - 1;
        while (i && heapNode->freq < heap->array[(i-1) / 2]->freq) {
            heap->array[i] = heap->array[(i-1)/ 2];
            i = (i-1)/ 2;
        }
    heap->array[i] = heapNode;
}

//|-------------------------------------------------------------------------------------------------------------------| BUILD MINHEAP

void buildMinHeap(heap* heap) {

  int n = heap->size - 1;
  int i;

  for (i = (n-1)/2; i >= 0; i--) {
    minHeapify(heap, i);
  }
}

//|-------------------------------------------------------------------------------------------------------------------| BUILD HUFMAN TREE

heapNode* buildHuffmanTree(heap* heap, int size) {

    heapNode* left, *right, *top;

    while(heap->size != 1) {  //Iterate while size of heap isn't 1
        //Extract least frequent items from min heap
        left = extractMin(heap);
        right = extractMin(heap);

        //Add new node with sum of frequencies with the extracted nodes as left and right children, arbitrary value for parent
        top = newNode("", left->freq + right->freq);
        top->left = left;
        top->right = right;
        insertMinHeap(heap, top);
    }
    return extractMin(heap); //Return Root
}

//|-------------------------------------------------------------------------------------------------------------------| VISUALIZE HUFMAN TREE
/*
void visualizeHuffman(heapNode* huffmanRoot){
        HELPER FUNCTION
        NOT ACTUALLY USED IN CODE
            Call visualizeHuffman to go through the nodes of the min-heap, for testing purposes
            Hit either of w, a, or d (and hit enter)
                w = return to root node
                a = go left
                d = go right
                x = exit visualizer
            If there is no token displayed (Second pair of parenthesis is populated), the node viewed is a frequency connector node.
            If a token is displayed, hit w or x. Otherwise the program crashes. lol.


    char control;
    heapNode* current = huffmanRoot;

    printf("VISUALIZING HUFFMAN TREE\n");

    while(read(STDIN_FILENO, &control, 1) > 0){
        if(control == 'w'){ //BACK TO ROOT
            current = huffmanRoot;
            printf("RETURNED TO ROOT (FREQ = %d) (%s)\n", current->freq, current->token);
        }
        else if(control == 'd'){//RIGHT
            current = current->right;
            printf("RIGHT : (FREQ = %d) (%s)\n", current->freq, current->token);
        }
        else if(control == 'a'){//LEFT
            current = current->left;
            printf("LEFT : (FREQ = %d) (%s)\n", current->freq, current->token);
        }
        else if(control == 'x'){//EXIT
            return;
        }
    }
    return;
}
*/
//|-------------------------------------------------------------------------------------------------------------------| WRITE CODEBOOK (-b flag)

void writeCodebook(){

  //RIGHT NOW IT MAKES A TXT FILE BUT IT SHOULD BE JUST "HuffmanCodebook" -> FIX LATER
  int wr = open("./HuffmanCodebook", O_CREAT | O_WRONLY | O_TRUNC, 0644);
  //printf("\nFD : %d\n", fd);

    //TERMINATOR CHARACTER + NEWLINE
    write(wr, "^", 1);
    write(wr, "\n", 1);

        linked *ptr = head;
        while(ptr != NULL){
            write(wr, ptr->ascii, strlen(ptr->ascii));
            write(wr, "\t", 1);
            write(wr, ptr->token, strlen(ptr->token));
            write(wr, "\n", 1);
                ptr = ptr->next;
        }
    //NEWLINE TERMINATED
    //write(wr, "\n", 1);
    close(wr);
}

//|-------------------------------------------------------------------------------------------------------------------| GENERATE BYTE STRINGS FOR CODEBOOK (-b)

void generateBytestrings(heapNode* node, char* ascii){

    if(node == NULL){
        return;
    }
    char *left = malloc(64 * sizeof(char));
        strcpy(left, ascii);
    char *right = malloc(64 * sizeof(char));
        strcpy(right, ascii);

    //printf("\n(%d | %s | %s)", node->freq, ascii, node->token);

    if(strlen(node->token) > 0){ //Token is populated
        //printf("*"); //Signify Token Is Printed
        //Find ptr in linked list to token in heap
        linked *ptr = head;
            while(ptr != NULL){
                    if(strcmp(node->token, ptr->token) == 0){
                            ptr->ascii = malloc(sizeof(char) * strlen(ascii));
                            strcpy(ptr->ascii, ascii);
                            break;
                    }
                ptr = ptr->next;
            }//WHILE

    }//TOKEN POPULATED
    strcat(left, "0");
    generateBytestrings(node->left, left);
    free(left);

    strcat(right, "1");
    generateBytestrings(node->right, right);
    free(right);
}

//|-------------------------------------------------------------------------------------------------------------------| COMPRESS (-c)

void compress(char* filename, char* codebookname){

    int cb = open(codebookname, O_RDONLY);
    int rd = 0;

//----------| ALLOCATE CODEBOOK BUFFER

    int buffersize = 10;
    char* cb_buffer = malloc(sizeof(char) * buffersize);

    rd = read(cb, cb_buffer, buffersize);
    int loops = 0;
        while(rd != 0){
          rd = read(cb, cb_buffer, buffersize);
          loops++;
        }
      close(cb);
              buffersize = buffersize * loops;
              cb_buffer = malloc(sizeof(char) * buffersize);
                  cb = open(codebookname, O_RDONLY);
                  rd = read(cb, cb_buffer, buffersize);
                  cb_buffer[rd] = '\0';
                  close(cb);
//----------| GRAB ASCII + TOKENS FROM CB

    char minibuffer[128];
    char asciibuffer[128];
    int i = 2;
    int cmb = 0;
    int begptr = 0;

    for(cmb = 0; cmb < 128; cmb++){
        minibuffer[cmb] = '\0';
    }

    while(i != (rd)){

        //-----------------------------------------------------| GRAB WHAT'S BETWEEN NEWLINE AND TAB (THE TOKEN)
        if(cb_buffer[i] == '\n'){
            //printf("NEWLINE | TOKEN : %s\n", minibuffer);
            //printf("(T : %s | A : %s)\n", minibuffer, asciibuffer);
            linkedinsert(minibuffer);

            linked *ptr = head;
                while(ptr->next != NULL){
                    ptr = ptr->next;
                }
                ptr->ascii = malloc(sizeof(char) * strlen(asciibuffer));

            //Send ASCII
                for(cmb = 0; cmb < strlen(asciibuffer); cmb++){
                        ptr->ascii[cmb] = asciibuffer[cmb];
                }
                ptr->ascii[strlen(asciibuffer)] = '\0';

            //CLEAR MINIBUFFER
                for(cmb = 0; cmb < 128; cmb++){
                    minibuffer[cmb] = '\0';
                    asciibuffer[cmb] = '\0';
                }
                begptr = 0;
        }//IF \n

        //-----------------------------------------------------| GRAB WHAT'S BETWEEN START OF LINE AND TAB (THE BYTESTRING)
        else if(cb_buffer[i] == '\t'){
            //printf("TAB | ASCII : %s\n", minibuffer);
                strcpy(asciibuffer, minibuffer);

                //printf("ASCIIBUFFER : %s\n", asciibuffer);


            //CLEAR MINIBUFFER
                for(cmb = 0; cmb < 128; cmb++){
                    minibuffer[cmb] = '\0';
                }
                begptr = 0;
        }//IF \t

         //-----------------------------------------------------| CONTINUE ADDING ON TO TOKEN
        else{ //CHAR IS A PART OF TOKEN
            minibuffer[begptr] = cb_buffer[i];
            begptr++;
            //printf("BUFFER : %s\n", minibuffer);
        }

        i++;
    }//While

  free(cb_buffer);

  compressed_tokenize(filename);

}

//|-------------------------------------------------------------------------------------------------------------------| COMPRESS TOKENIZE (-d)

void compressed_tokenize(char* filename){

    int namelen = strlen(filename);
    char* hczfile = malloc((sizeof(char) * namelen) + 4);
    strcpy(hczfile, filename);
    strcat(hczfile, ".hcz");
    //printf("FILE NAME : %s", hczfile);
    int fd = open(filename, O_RDONLY);
        if((fd == -1)){
            //If fd = -1, Directory Was Supplied / File DNE ---> EXIT FUNCTION
            //printf("Error Opening File '%s'\n\t(Perhaps it does not exist, it is a directory, or permissions are not granted)\n", filename);
            return;
        }
        else{
            //printf("\n'%s'\n", filename);
        }
  int hcz = open(hczfile, O_CREAT | O_WRONLY | O_TRUNC, 0644);

  int buffersize = 10;
  char* buffer = malloc(sizeof(char) * buffersize);

  //------------------------------------------------------------| FINDING BUFFER SIZE

  int rd = read(fd, buffer, buffersize);
      if(rd == 0){
          printf("\tWARNING : File %s is empty! Cannot process any data.\n", filename);
          return;
      }

      int loops = 0;
      while(rd != 0){
          rd = read(fd, buffer, buffersize);
          loops++;
          //printf("\t%d\t|\t%d\t|\t%d\t\n", rd, loops, (buffersize * loops));
          }
      close(fd);

      buffersize = buffersize * loops;
      buffer = malloc(sizeof(char) * buffersize);
          fd = open(filename, O_RDONLY);
          rd = read(fd, buffer, buffersize);
          close(fd);
//------------------------------------------------------------| TOKENIZE
  int i = 0;
  int j = 0;
  int k = 0;
  char str[buffersize];
      str[0] = '\0';
      buffer[rd] = '\0';

  for(i = 0; i <= rd; i++){
            //printf("\nBuffer[%d] = \'%c\'", i, buffer[i]);
            if((buffer[i] != ' ') && (buffer[i] != '\n') && (buffer[i] != '\t') && (buffer[i] != '\0')){
                str[j + 1] = '\0';
                str[j] = buffer[i];
                j++;
            }

            else{ //EXPORT TOKEN TO LINKED LIST (TERMINATOR CHARACTER WAS REACHED)
                //printf("\t%s\n", str);
                    if(str[0] != '\0'){ //STRING HAS CONTENT
                            //printf("%s\n", str);
                                                linked *ptr = head;
                                                while(ptr != NULL){
                                                    if(strcmp(ptr->token, str) == 0){
                                                        //printf("WRITING : %s\n", ptr->ascii);
                                                        write(hcz, ptr->ascii, (strlen(ptr->ascii)));
                                                    }
                                                    else{
                                                        //printf(". (%s)\n", ptr->ascii);
                                                    }
                                                    ptr = ptr->next;
                                                }
                    }
                    if((buffer[i] == ' ') || (buffer[i] == '\n') || (buffer[i] == '\t') || (buffer[i] != '\0')){
                            str[0] = '\0';
                                for(k = 1; k < j; k++){
                                    str[k] = '\0';}
                            j = 0;
                                    if(buffer[i] == ' '){
                                        str[0] = '^';
                                        str[1] = 's';
                                        str[2] = '\0';
                                    }
                                    else if(buffer[i] == '\n'){
                                        str[0] = '^';
                                        str[1] = 'n';
                                        str[2] = '\0';
                                    }
                                    else if(buffer[i] == '\t'){
                                        str[0] = '^';
                                        str[1] = 't';
                                        str[2] = '\0';
                                    }
                                            linked *ptr = head;
                                                while(ptr != NULL){
                                                    if(strcmp(ptr->token, str) == 0){
                                                        write(hcz, ptr->ascii, (strlen(ptr->ascii)));
                                                    }
                                                    else{
                                                        //printf(". (%s)\n", ptr->ascii);
                                                    }
                                                    ptr = ptr->next;
                                                }//While
                    }
                str[0] = '\0';
                    for(k = 1; k < j; k++){
                        str[k] = '\0';}
                j = 0;
            }
  }//For

  close(hcz);
  close(fd);
  free(buffer);
  free(hczfile);

}//TOKENIZE

//|-------------------------------------------------------------------------------------------------------------------| DECOMPRESS (-d)

char decompress(char* codebookname){

//----------| ALLOCATE CODEBOOK BUFFER

    int cb = open(codebookname, O_RDONLY);
        root = newNode("", 0);

    int rd = 0;
    int buffersize = 10;
    char* cb_buffer = malloc(sizeof(char) * buffersize);

    rd = read(cb, cb_buffer, buffersize);
    int loops = 0;
        while(rd != 0){
          rd = read(cb, cb_buffer, buffersize);
          loops++;
        }
      close(cb);
              buffersize = buffersize * loops;
              cb_buffer = malloc(sizeof(char) * buffersize);
                  cb = open(codebookname, O_RDONLY);
                  rd = read(cb, cb_buffer, buffersize);
                  cb_buffer[rd] = '\0';
                  close(cb);

//----------| READ

    char delimeter = cb_buffer[0];
    //printf("DELIMETER : '%c'\n\n", delimeter);

    char minibuffer[128];
    char asciibuffer[128];
    int i = 2;
    int cmb = 0;
    int begptr = 0;

    for(cmb = 0; cmb < 128; cmb++){
        minibuffer[cmb] = '\0';
    }

    while(i != (rd)){

//-----------------------------------------------------| GRAB WHAT'S BETWEEN NEWLINE AND TAB (THE TOKEN)
        if(cb_buffer[i] == '\n'){
            //printf("NEWLINE | TOKEN : %s\n", minibuffer);
            //printf("\t(T : %s | A : %s)\n", minibuffer, asciibuffer);
            recreate(minibuffer, asciibuffer);

            //CLEAR MINIBUFFER
                for(cmb = 0; cmb < 128; cmb++){
                    minibuffer[cmb] = '\0';
                    asciibuffer[cmb] = '\0';
                }
                begptr = 0;
        }//IF \n

        //-----------------------------------------------------| GRAB WHAT'S BETWEEN START OF LINE AND TAB (THE BYTESTRING)
        else if(cb_buffer[i] == '\t'){
            //printf("TAB | ASCII : %s\n", minibuffer);
                strcpy(asciibuffer, minibuffer);

                //printf("ASCIIBUFFER : %s\n", asciibuffer);

            //CLEAR MINIBUFFER
                for(cmb = 0; cmb < 128; cmb++){
                    minibuffer[cmb] = '\0';
                }
                begptr = 0;
        }//IF \t

         //-----------------------------------------------------| CONTINUE ADDING ON TO TOKEN
        else{ //CHAR IS A PART OF TOKEN
            minibuffer[begptr] = cb_buffer[i];
            begptr++;
            //printf("BUFFER : %s\n", minibuffer);
        }

        i++;
    }//While

  free(cb_buffer);
  close(cb);

  return delimeter;
}

void recreate(char* token, char* ascii){ //BUILDS HUFFMAN TREE AGAIN

  int ascii_len = strlen(ascii);
  int i = 0;
  heapNode* temp = root;

    for(i = 0; i < ascii_len; i++){
        //printf("%c - ", ascii[i]);

        if(i == (ascii_len - 1)){//IF LAST
                if(ascii[i] == '0'){
                        if(temp->left == NULL){
                            temp->left = newNode(token, 1); //printf("Created A New Left Node *|* ");
                        }
                    //printf("L (LAST)\n");
                }//0

                if(ascii[i] == '1'){
                        if(temp->right == NULL){
                            temp->right = newNode(token, 1); //printf("Created A New Right Node *|* ");
                        }
                    //printf("R (LAST)\n");

                }//1
        }

        else if(ascii[i] == '0'){
           if(temp->left == NULL){
                temp->left = newNode("", 0);
            }
            temp = temp->left;
            //printf("L\n");
        }
        else if(ascii[i] == '1'){
            if(temp->right == NULL){
                temp->right = newNode("", 0);
            }
            temp = temp->right;
            //printf("R\n");
        }
    }
}

void traverse(char* filename, char delimeter){

  int fd = open(filename, O_RDONLY);

      if((fd == -1)){
          //If fd = -1, Directory Was Supplied / File DNE ---> EXIT FUNCTION
          //printf("Error Opening File '%s'\n\t(Perhaps it does not exist, it is a directory, or permissions are not granted)\n", filename);
          return;
      }

  int namelen = strlen(filename);
  char* originalfile = malloc(sizeof(char) * strlen(filename));
      strcpy(originalfile, filename);
      originalfile[namelen - 1] = '\0';
      originalfile[namelen - 2] = '\0';
      originalfile[namelen - 3] = '\0';
      originalfile[namelen - 4] = '\0';
          //printf("Original File : %s\n", originalfile);

  int originalfilefd = open(originalfile, O_CREAT | O_WRONLY | O_TRUNC, 0644);

  int buffersize = 10;
  char* buffer = malloc(sizeof(char) * buffersize);

//------------------------------------------------------------| FINDING BUFFER SIZE

  int rd = read(fd, buffer, buffersize);
      if(rd == 0){
          printf("\nERROR : File %s is empty! Cannot process any data.", filename);
          return;
      }

      int loops = 0;
      while(rd != 0){
          rd = read(fd, buffer, buffersize);
          loops++;
          //printf("\t%d\t|\t%d\t|\t%d\t\n", rd, loops, (buffersize * loops));
          }
      close(fd);

    buffersize = buffersize * loops;
    buffer = malloc(sizeof(char) * buffersize);
      fd = open(filename, O_RDONLY);
      rd = read(fd, buffer, buffersize);
      close(fd);
    buffer[rd] = '\0';

  heapNode* temp = root;
  char observed;
  int i = 0;
//printf("BUFFER : %s\n\n\n", buffer);

  while(buffer[i] != '\0'){

  observed = buffer[i];
  i++;

            if(observed == '0'){
                //printf("LEFT | ");
                temp = temp->left;

                    if(temp->freq == 1){
                            if(temp->token[0] == delimeter && temp->token[1] == 'n' && temp->token[2] == '\0'){
                                //printf("\n");
                                write(originalfilefd, "\n", 1);
                            }
                            else if(temp->token[0] == delimeter && temp->token[1] == 's' && temp->token[2] == '\0'){
                                //printf(" ");
                                write(originalfilefd, " ", 1);
                            }
                            else if(temp->token[0] == delimeter && temp->token[1] == 't' && temp->token[2] == '\0'){
                                //printf("\t");
                                write(originalfilefd, "\t", 1);
                            }
                            else{
                                //printf("%s", temp->token);
                                write(originalfilefd, temp->token, strlen(temp->token));
                            }
                        temp = root;
                    }//FREQ

            }//0
            else if(observed == '1'){
                //printf("RIGHT | ");
                temp = temp->right;

                    if(temp->freq == 1){
                           if(temp->token[0] == delimeter && temp->token[1] == 'n' && temp->token[2] == '\0'){
                                //printf("\n");
                                write(originalfilefd, "\n", 1);
                            }
                            else if(temp->token[0] == delimeter && temp->token[1] == 's' && temp->token[2] == '\0'){
                                //printf(" ");
                                write(originalfilefd, " ", 1);
                            }
                            else if(temp->token[0] == delimeter && temp->token[1] == 't' && temp->token[2] == '\0'){
                                //printf("\t");
                                write(originalfilefd, "\t", 1);
                            }
                            else{
                                //printf("%s", temp->token);
                                write(originalfilefd, temp->token, strlen(temp->token));
                            }
                        temp = root;
                    }//FREQ

            }//1

            else{
                printf("\nERROR : Illegal Character (Not 0 Or 1) In .hzc File!");
                return;
            }
  }

  close(originalfilefd);
  free(originalfile);
  free(buffer);

}

void freeheap(heapNode* node){

  if(node == NULL){
      return;
  }

  freeheap(node->left);
  freeheap(node->right);

  free(node->token);
  free(node);
}
