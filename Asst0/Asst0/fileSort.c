#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

//|-----------------------------------------------------------------------------------------------------| PREREQUISITES
/*
    argv[1] = FLAG FOR SORTING METHOD
        0 = INSERTION SORT
        1 = QUICK SORT
    argv[2] = FILE NAME
    COMPARISON MODE = FLAG FOR DETERMINING IF WE ARE COMPARING DIGITS OR STRINGS
        0 = COMPARING DIGITS
        1 = COMPARING STRINGS
    LINKED LIST HAS TWO VALUES/MODES
        VALUE - When in "integer mode", value will hold the values of each integer
                "TOKEN" will be empty / NULL
        TOKEN - When in "string mode", token char* will hold each node's string
                Value will NOT be NULL in string mode, but rather, it will be used to hold the length of the string (excluding the null byte '\0')
                    (So for example, if token = "hello", value = 5, not 6. HOWEVER, the string will have six character, with '\0' being the final character)
*/
//comparator is for comparing/sorting based on int or string
//LINKED LIST STRUCT
// CHECK txt file for more
// NO MAKE FILE, ONLY gcc
//no sorting methods like strcmp allowed, alphanumeric stuff is ok though
// NO C99, no fsanitizer
/* 	Be sure that you read in your data carefully:
		- check for EOF (end of file) as you go
		- make sure you handle nonblocking-reads
		- make sure you find/drop spaces, tabs, newlines and other breaks in the token sequences
		- make sure you can read in your tokens with no presumption on token length
		- make sure you can read in your tokens with no presumption on total number of tokens
*/
    // can use atoi for int/string conversion
//start by reading a file (open + error states, read in char up to comma etc, read 1 char a time)
// sort -> code linked lists, do insertion sort first, write sorts on ll, duplicate code and change to work on LL
//function pointers -> write an integer and string comparator, generalize inputs to be void* but cast inside function, copy/past sort and modify to accept comparators

// WHAT TO DO
// FREE MEMORY
// GO OVER DISCORD, PIAZZA, FRANCISCO DOCS TO MAKE SURE EVERYTHING IS IMPLEMENTED PROPERLY
// TEST MANY DIFFERENT CASES SUCH AS BLANK TOKENS, NEGATIVE NUMBERS, ETC
// EMPTY TOKENS COUNT AND PRINT A NEW LINE
// REALLAOCATE/EXPAND MEMORY TO HANDLE VARIABLE SIZES FOR THE BUFFER AND TOKENS
/* best way to do realloc is this
malloc()

memcpy()

free()
*/
// GET RID OF TESTING PRINT STATEMENTS TO MAKE SURE FINAL PROGRAM OUTPUTS PROPERLY


struct node{
    int value;
    char *token; //make data a void* or void? must cast so better to do it for addresses
    struct node *next;
    struct node *prev;
};

typedef struct node node;
    node *head = NULL;
    node *tail = NULL;

//--------------------------| FUNCTIONS LIST
void printlist(int mode);
void insert_string(char *str, int stringlength);
void freelist(node* h);
void insert_digit(int input);
int intComp(void* thing1, void* thing2);
int strComp(void* thing1, void* thing2);
int insertionSort(void* toSort, int (*comparator)(void*, void*));
int quickSort(void* toSort, int (*comparator)(void*, void*));
node* partition(node* start, node* end, int (*comparator)(void*, void*));
void _quickSort(node* start, node* end, int (*comparator)(void*, void*));


int sortingmethod; //insertion or quick
int compmode; // digit or character

//|-----------------------------------------------------------------------------------------------------| MAIN

int main(int argc, char** argv){
  //select -i for insertion or -q for quick sort
//dont use fopen, use open
// return exit_success or have return 0?
  int fd; //(File Descriptor)
  //char buffer[1000]; //realloc or some equivalent

//SELECTING SORTING METHOD
    if((argv[1][0] == '-') && (argv[1][1] == 'i') && (argv[1][2] == '\0')){
        sortingmethod = 0;
    }
    else if((argv[1][0] == '-') && (argv[1][1] == 'q') && (argv[1][2] == '\0')){
        sortingmethod = 1;
    }
//ERROR HANDLING FOR INCORRECT ARG COUNT
    //TWO ARGS GIVEN
    if(argc == 3){
        if(argv[1][0] != '-'){
            printf("Fatal Error: \"%s\" is not a valid sort flag", argv[1]);
            return EXIT_FAILURE;
        }
    }
    //ONE ARG GIVEN
    else if(argc == 2){
         printf("Fatal Error: expected two arguments, had one");
        return EXIT_FAILURE;
    }

    fd = open(argv[2], O_RDONLY);
        if(fd == -1){
          printf("FATAL ERROR : Could not open file. Perhaps it does not exist? Closing Program.");
          close(fd);
          return EXIT_FAILURE;
        }
        //--------------------------| DETERMINING THE BUFFER SIZE
    int buffersize = 10;
    char* buffer = malloc(sizeof(char) * buffersize);
    if(buffer == NULL){
      printf("\nERROR : A malloc() failed!");
      }
    int rd = read(fd, buffer, buffersize); //reallocate more space to buffer if not big enough
    if(rd == 0){
        printf("WARNING : The file is empty! Cannot process any data.");
      //  return EXIT_FAILURE;
    }
    int loops = 0;

    //  printf("(   rd  |   loops   |   buffersize  )\n");
      while(rd != 0){
          rd = read(fd, buffer, buffersize);
          loops++;
          //printf("(   %d    |   %d    |   %d    )\n",rd, loops, (buffersize * loops));
      }
      close(fd);

      buffersize = buffersize * loops;
      buffer = malloc(sizeof(char) * buffersize);
      if(buffer == NULL){
        printf("\nERROR : A malloc() failed!");
      }
          fd = open(argv[2], O_RDONLY);
          rd = read(fd, buffer, buffersize);
          close(fd);
      //printf("FINAL BUFFER SIZE : %d \nRD : %d\n", buffersize, rd);
    //  free(buffer);

    //IF STATEMENT TO DETERMINE IF COMPARING DIGITS OR STRINGS
    int m = 0;
    compmode = 0;

    for(m = 0; m < rd; m++){
        if(buffer[m] == ',' || buffer[m] == '\t' || buffer[m] == '\n' || buffer[m] == ' '){}
        else if(isdigit(buffer[m]) > 0){ //If it is a digit enter if
            compmode = 0;
            break;
        }
        else if(isdigit(buffer[m]) == 0){ //isdigit is 0 if char isnt a digit
            compmode = 1;
            break;
        }
    }

  //  printf("\nSorting Method : %d ... (0 = INSERTION SORT | 1 = QUICKSORT)\n", sortingmethod);
  //  printf("Comparison Mode : %d ... (0 = COMPARING DIGITS | 1 = COMPARING STRINGS)\n", compmode);
  //  printf("File Name : %s\n", argv[2]);
//    printf("\n-------------------------------------------------------------\n \n");

//|-----------------------------------------------------------------------------------------------------| PUTTING THE TOKENS INTO THE LINKED LIST

    int i = 0;
    int j = 0;
    int k = 0;
    int in;
    char str[buffersize];
        str[0] = '\0';
        buffer[rd] = '\0';

//    printf("BUFFER ENDS AT buffer[%d]\n", rd); //is whole file reading NON BLOCKING?

    for(i = 0; i <= rd; i++){
        //CREATING STRINGS FOR EACH TOKEN
    //    printf("\nbuffer[%d] = \'%c\'", i, buffer[i]);
          if(buffer[i] != ',' && buffer[i] != '\0'){
            if(buffer[i] != '\t' && buffer[i] != '\n' && buffer[i] != ' '){
                str[j + 1] = '\0';
                str[j] = buffer[i];
                j++;
            }
          }
      //EXPORT TOKEN TO LINKED LIST
          else{
      //      printf("\n - EXPORTING STRING : %s", str);
            //IF STRING HAS CONTENT
            if(str[0] != '\0'){
              if(compmode == 0){
                  insert_digit(atoi(str));
              }
              else if(compmode == 1){
                  insert_string(str, j);
              }
            }//IF
              //ELSE, STRING IS NULL... BUT STILL NEED TO EXPORT
              else{
                  if(i != rd){
                    if(compmode == 0){
                        insert_digit(0);
                    }
                    else if(compmode == 1){
                        //IN FINAL VERSION THIS SHOULD PROBABLY BE REPLACED WITH ("",0)
                        //RIGHT NOW IT IS A STAR TO SIGNIFY A NULL VALUE
                        insert_string("", 0);
                    }
                  }
                }

    //FREE ALL CHARS IN STR ARRAY (Free wasn't working???)
            str[0] = '\0';
                for(k = 1; k < j; k++){
                    //free(str[k]);  //fix get assignment makes integer from pointer without cast
                    str[k] = '\0';
                }
            j = 0;
          }//ELSE
    }//FOR

//|-----------------------------------------------------------------------------------------------------| PRINTING LISTS

  //  printf("\n\n-----------------------\n");
  //  printf("\n - Buffered Text (FROM FILE) : %s", buffer);
  //  printf("\n\n-------------------------------------------------------------\n");

      /*  if(compmode == 0){
            printlist(0);
        }
        if(compmode == 1){
            printlist(1);
        } */
    node *toSort = head;
    //do insertion sort here
  //  printf("\n\n-----------------------\n");
  //  printf("\n - SORTED FILES PRINTING");
//    printf("\n\n-------------------------------------------------------------\n");
    if (sortingmethod==0) { //insertion sort
      if (compmode==0) { //digits
        insertionSort(toSort, intComp);
        printlist(0);
      } else if (compmode==1) { //letters
        insertionSort(toSort, strComp);
        printlist(1);
      }
    } else if (sortingmethod=1) { //quicksort
      if (compmode==0) { //digits
        quickSort(toSort, intComp);
        printlist(0);
      } else if (compmode==1) { //leters
        quickSort(toSort, strComp);
        printlist(1);
      }
    }
    //FREE THE LINKED LIST
    //freelist(head);
    //PRINTLIST FOR DEBUGGING -> CHECKS IF HEAD / LIST IS NULL (NON-EXISTANT)
    //printlist(0);
    freelist(head);
    free(buffer);

  return EXIT_SUCCESS; // EXIT_SUCCESS IS = TO 0 BTW
}

//|-----------------------------------------------------------------------------------------------------| INSERTION SORT

int insertionSort(void* toSort, int (*comparator)(void*, void*)) {  //insertion sort but only ints work, no comparator
  //head = NULL;
  //int cmp;
  node* sorted = NULL;
  node* curr = (node*)toSort;
  node* next = NULL;
  node* ptr;

  while(curr != NULL){
    next = curr->next;
    curr->prev = curr->next = NULL;
    if(sorted == NULL){
        sorted = curr;


    //} else if((*item1).token >= (*item2).token){
    }
    else if(comparator(sorted, curr) == 1){
        curr->next = sorted;
        curr->next->prev = curr;
        sorted = curr;
    }
    else if(comparator(sorted, curr) == 0){
        ptr = sorted;

            //while(ptr->next != NULL && (*ptr->next).token < (*item2).token){
            while(ptr->next != NULL && comparator(ptr->next, curr) == 0){
                ptr = ptr->next;
            }
                curr->next = ptr->next;

            if(ptr->next != NULL){
                curr->next->prev = curr;
            }
                ptr->next = curr;
                curr->prev = ptr;
            }

    else{
      ptr = sorted;
        //while(ptr->next != NULL && (*ptr->next).token < (*item2).token){
        while(ptr->next != NULL && comparator(ptr->next, curr) == 0){
            ptr = ptr->next;
        }
            curr->next = ptr->next;
        if(ptr->next != NULL){
            curr->next->prev = curr;
        }
            ptr->next = curr;
            curr->prev = ptr;

    }
    /*sorted = head;
    head = NULL; */
    curr = next;
  }
  head = sorted;
  tail = head; //head->prev?

  return 0;
}

//|-----------------------------------------------------------------------------------------------------| INT COMP

int intComp(void* thing1, void* thing2) {
    // CHECK IF WORKS WITH NEGATIVES
    node* item1node = (node*) thing1; //cast void/generic argument into node or whatever you need
    node* item2node = (node*) thing2;
    int item1 = (*item1node).value;
    int item2 = (*item2node).value;

    if (item1 > item2) { //or item1 >= item2
      return 1;
    } else if ( item1 < item2) { // or item1 < item2
      return 0;
    }
}

//|-----------------------------------------------------------------------------------------------------| STR COMP

int strComp(void* thing1, void* thing2) { //use atoi for ascii sorting???
  // CHECK if works if both strings are the same
  node* item1node = (node*) thing1;
  node* item2node = (node*) thing2;
  char* item1 = (*item1node).token;
  char* item2 = (*item2node).token;

  int i = 0;
  int ge = 0; //item 1 is greater than or equal

  while (item1[i] != '\0' && item2[i] != '\0') {
    if (item1[i] == item2[i]) {
      i++;
    } else if (item1[i] > item2[i]) {
      ge = 1;
      return ge;
    } else if (item1[i] < item2[i]) { // or just <
      ge = 0;
      return ge;
    }
  }
    if(item1[0] == '\0'){
        ge = 0;
        return ge;
    }

     if(item2[0] == '\0'){
        ge = 1;
        return ge;
    }
  return ge;


  //0 = left less than right
  //1 = left greater than right
}


//|-----------------------------------------------------------------------------------------------------| QUICK SORT
int quickSort(void* toSort, int (*comparator)(void*, void*)) { //main quicksort with prototype
  node* start = (node*)toSort;
  node* end = (node*)toSort;
  while (end && end->next) {
    end = end->next;
  }

  _quickSort(start, end, comparator);
  head = start; //return sorted listed
  tail = head;
  return 0;
}

void _quickSort(node* start, node* end, int (*comparator)(void*, void*)) { //recursive quicksort
  if (end!=NULL && end!=start && start!=end->next) { //check if start and end have crossed over
//  if (start!=end && start->prev!=end) {
    node* pivot = partition(start,end, comparator);
    _quickSort(start, pivot->prev, comparator); //everything less than pivot
    _quickSort(pivot->next, end, comparator); //everythign greater than pivot
  }

}

node* partition(node* start, node* end, int (*comparator)(void*, void*)) { //partition and split the list
//  node* left = start;
//  node* ptr = end->prev;
  node* pivot = end; //should be first??
  node* right = start->prev;
  node* left;

  for(left=start; left!=end; left=left->next) {
    if(comparator(left,pivot)==0 || left==pivot) {

      right = (right ==NULL)? start : right->next;
      if (compmode==0) { //swap left and right
        int temp=right->value;
        right->value = left->value;
        left->value = temp;
      } else {
        char* temp = right->token;
        right->token = left->token;
        left->token = temp;
      }

    }
  }
  right  = (right ==NULL)? start : right->next;
  if (compmode==0) { //put pivot into proper position
    int temp=right->value;
    right->value = pivot->value;
    pivot->value = temp;
  } else {
    char* temp = right->token;
    right->token = pivot->token;
    pivot->token = temp;
  }
  return right;
}

//|-----------------------------------------------------------------------------------------------------| LINKED LIST PRINT
    /*
        MODES :
            0 = PRINTING OUT LIST OF DIGITS
            1 = PRINTING OUT LIST OF STRINGS
    */
void printlist(int mode){

    node *ptr = head;

    if(head == NULL){
         printf("\nError : The head is empty! Cannot printlist()! (No Linked List Exists)\n");
         return;
    }

    //DIGITS
    if(mode == 0){
      //  printf("\n - DIGIT Linked List : \n");
          while(ptr != NULL){
              printf("%d\n", ptr->value);
              ptr = ptr->next;
          }
        //  printf("-----------------------\n");
    /*  UNCOMMENT TO TEST REVERSING LINKED LIST
        ptr = tail;
        printf("\n - DIGIT Linked List (BACK TO FRONT) : \n");
            while(ptr != NULL){
                printf("%d ", ptr->value);
                ptr = ptr->prev;
            }
            printf("\n");
    */
    }
    //STRINGS
    else if(mode == 1){
    //    printf("\n - STRING Linked List : \n");
          while(ptr != NULL){
              printf("%s\n", ptr->token);
              //printf("(%s | %d) ", ptr->token, ptr->value);
              ptr = ptr->next;
          }
        //  printf("-----------------------\n");
    /*  UNCOMMENT TO TEST REVERSING LINKED LIST
        ptr = tail;
        printf("\n - STRING Linked List (BACK TO FRONT) : \n");
            while(ptr != NULL){
                printf("%s ", ptr->token);
                //printf("(%s | %d) ", ptr->token, ptr->value);
                ptr = ptr->prev;
            }
            printf("\n");
    */
    }
}
//|-----------------------------------------------------------------------------------------------------| INSERT DIGITS INTO LINKED LIST

void insert_digit(int input){

    node* tempNode = malloc(sizeof(node));
    if(tempNode == NULL){
      printf("\nERROR : A malloc() failed!");
  }
    (*tempNode).value = input;

      if(head == NULL){
          head = tempNode;
          (*head).next = NULL;
          (*head).prev = NULL;
          tail = head;
      }

      else{
        node *ptr = head;
        node *ptrprev = NULL;

          while(ptr->next != NULL){
              ptrprev = ptr;
                  //printf("PtrPrev Val = %d\n", ptrprev->value);
              ptr = ptr->next;
                  //printf("Ptr Val = %d\n", ptr->value);
          }
              tempNode->next = NULL;
              tempNode->prev = ptr;
                  ptr->next = tempNode;
                  tail = tempNode;
                /*
                printf(" -   Tail Prev = %d\n", tail->prev->value);
                printf(" -   Tail = %d\n", tail->value);
                printf("----------------\n");
                */
      }
    //  free(tempNode);
}

//|-----------------------------------------------------------------------------------------------------| INSERT STRINGS INTO LINKED LIST

void insert_string(char *str, int stringlength){

    node* tempNode = malloc(sizeof(node));
    if(tempNode == NULL){
        printf("\nERROR : A malloc() failed!");
    }

    tempNode->token = malloc(sizeof(char) * stringlength + 1);
    (*tempNode).value = stringlength;

    int i = 0;
    for(i = 0; i <= stringlength + 1; i++){
        tempNode->token[i] = str[i];
            if(i == stringlength + 1){
                tempNode->token[stringlength + 1] = '\0';
            }
    }

    //PRINT TO SEE IF VALUES MADE IT TO THE FUNCTION
  //  printf("\n\t - NEW NODE \'%s\' CREATED (Length Of %d)\n", tempNode->token, stringlength);

        if(head == NULL){
            head = tempNode;
            (*head).next = NULL;
            (*head).prev = NULL;
            tail = head;
            //printf("\t\tHead = %s \n", head->token);
        }

        else{
            node *ptr = head;
            node *ptrprev = NULL;

                  //printf("\t\tHead = (%s | %d)\n", head->token, head->value);
              while((ptr)->next != NULL){
                  ptrprev = ptr;
                  ptr = ptr->next;
                  //printf("\t\tPtr = (%s | %d)\n", ptr->token, ptr->value);
              }
                  tempNode->next = NULL;
                  tempNode->prev = tail;
                      ptr->next = tempNode;
                      tail = tempNode;
                  //printf("\t\tTail = (%s | %d)\n", tail->token, tail->value);
        }
      //  free(tempNode);
}

//|-----------------------------------------------------------------------------------------------------| FREE LINKED LIST

void freelist(node* h){
 node* next=h;

    while (next){
        node* n = next;
        next = n->next;
        //free((int)n->value);
      /*  if(compmode==0) {
          free(n->value);
        } else {
          free(n->token);
        } */
        free(n->token);
        free(n);
    }

  //  free(h);
}
