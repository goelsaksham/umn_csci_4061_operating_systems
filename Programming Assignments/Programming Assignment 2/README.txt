README FOR Programming Assignment 2

/*  login : goelx029, hartm433
*   date : 02/21/18
*   name : Saksham Goel, Franklin Hartman
*   id : goelx029(5138568), hartm433(5062484)
*/

The purpose of the program:-
 The purpose of the program is to count up the total amount of votes from a series of regions in order to declare a candidate a winner. The structure of the regions and their subregions may be thought of as a directory (tree) data structure, allowing the votes to be counted at each directory and aggregated when traversing up the directory struture. Consequentially, the count of votes in one region/directory is dependent on the count of votes in all of its subdirectories (children). This is similar in function to programming assignment one. 
The difference between this assignment and the previous assignment is therefore not in the program’s functionality, but its implementation. In this assignment, the graph structure is dictated by the directory structure rather than being given in the input. Furthermore, it was necessary to write our own read from file and write to file utilities (Leaf_Counter, Aggregate_Votes and Vote_Counter).

How to compile the program:-
    The program may be compiled using the makefile we have provided in the package. This will create an executable by the name of Vote_Counter, Aggregate_Votes and Leaf_Counter 

How to use the program from the shell (syntax):-
     To use the program from the shell, one must use the form of “./Vote_Counter %s”,
where %s represents the directory in which the subdirectories representing the graph are located. You can use the same command with other executables as follows:
    “./Aggregate_Votes %s”
    “./Leaf_Counter %s”


What exactly does your program do:
    Our program makes use of a fork and exec call structure to execute each node in the graph individually based off the dependency structure of the graph. This ensures that the parent nodes will wait until the child nodes have finished returning their results, which is necessary because the parent’s results depend on the child’s. Furthermore, the dependency structure of the graph for the regions and subregions is given by the structure of the file directories given, where the parent node is a folder and its direct children are folders directly within that folder (i.e. a folder named County_1 may contain multiple folder named Sub_County_1, Sub_County_2, etc.). A leaf node, or region with no subregions is simply a directory which contains no subdirectories and also contains a file named - votes.txt.
Once the result for a particular region is generated, a text file is generated with the count of votes for the region. Due to dependency structure, aggregation, and fork and exec calls, the count of votes for this region is also necessarily the total count of all the subregions votes as well.


Your x500 and the x500 of your partner:-
  goelx029 - Saksham Goel
  hartm433 - Franklin Hartman

Your and your partner's individual contributions:-
  We both worked on the project together, such that we first used to discuss what we will be doing and then splitting that part so that each of us can individually focus on one. used GIT as the VCS to merge the changes and developments. The contributions can be marked as follows:
  goelx029 - Making DAG, directory traversal, and exec and fork system call handling
  hartm433 - file parsing, memory allocation/freeing, error handling, documentation.
  Most of our work was distributed while making sure the of the strengths of the individual and what he/she would like to do.


Extra Credit: We did opt to do the extra credit for this project.

Note - We are including a file named - EdgeCases which includes a list of edgecases that we have considered

