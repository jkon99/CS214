CS 214: Systems Programming
Assignment 1: File Compressor
Authors:
	Jonathan Konopka
	NETID: jk1549
	RUID: 178005220
	Nicholas Rytelewski
	NETID: nr548
	RUID: 197000952

Project Description:
	The goal of this project was to create a program that would be able to create and use a “HuffmanCodebook” file, which could be used to perform compression and decompression operations on targeted files. Different flags may be used in the command line arguments to perform such operations (SEE BELOW). We created the huffman codes in our program with the min-heap data structure. A linked list was used to tokenize the contents of a called file and track the frequency of tokens, along with being used for miscellaneous purposes depending on the given flag.

Our program took either of the following command line flags :
1. “-b” : BUILD (./fileCompressor -b <filename>)
2. “-c” : COMPRESS (./fileCompressor -c <filename> HuffmanCodebook)
3. “-d” : DECOMPRESS (./fileCompressor -d <filename>.hcz HuffmanCodebook)
4. “-R” : RECURSIVE (./fileCompressor -R <flag> <filename>)

Having -R present before any previous flags makes the operation require a directory path rather than a file path, as specified above. This flag will collect all files within the directory specified and perform the operation on each. “-b” would make a single HuffmanCodebook to decipher all files within the directory. “-c” would create .hcz files for each non-hcz and non-codebook file found in the directory. “-d” would create the original decoded version of any .hcz files in the directory.

Time and Space Analysis:
We had two data structures in our project; a min-heap, and a linked list. In many cases throughout the program, elements within the linked list or min-heap needed to be searched for, for example, to retrieve the token or ascii character arrays. To search through our linked list, the time complexity would be O(n). Likewise, searching through our min-heap would have a time complexity of O(log(n)), comparable to that of a binary search tree since after our heap was set up, it was traversed as so.



Functions: listed in pdf
