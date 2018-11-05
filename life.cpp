#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdio>
#include "Mailbox.h"
using namespace std;

#define MAXGRID 40

// Jonathan Dang
// WPI ID: jdang

mailbox *mailboxes; 
int columns;
int rows;
int **evenGrid;
int **initialGrid;
int **oddGrid;
int totalGens;

// allocates memory to create the grid
int** createGrid(int r, int c) {
	int **a;
	a = new int * [r];

	for (int i = 0; i < r; i++) {
		a[i] = new int[c];
	}

	return a;
}

// prints out the grid
void printGrid(int maxR, int maxC, int type) {
	//cout << "hi" << endl;
	for (int r = 0; r < maxR; r++) {
		// loop invariant: at this point it is about to print all of the values within row r
		for (int c = 0; c < maxC; c++) {

			// if odd generation
			if ((type % 2) == 1) {
				cout << oddGrid[r][c] << " ";
			}

			// if even generation
			else if ((type % 2) == 0) {
				cout << evenGrid[r][c] << " ";
			}
			
		}
		// loop invariant: at this point all the columns within r (row) have been printed out
		cout << endl;
		cout.flush();
	}

}

// gets the number of neighbors
int getNeighbors(int maxR, int maxC, int r, int c, int type) {
	int value = 0;

	for (int i = r - 1; i <= r + 1; i++) {
		for (int j = c - 1; j <= c + 1; j++) {
			if ((i >= 0) && (j >= 0) && (i < maxR) && (j < maxC) &&
				(!(i == r && c == j))) {

				// if even generation
				if ((type % 2) == 0) {
					value += evenGrid[i][j];
				}

				// if odd generation
				else if ((type % 2) == 1) {
					value += oddGrid[i][j];
				}
			}
		}
	}

	//cout << "Neighbors: " << value << endl;
	return value;
}

void *myThread(void* arg) {
	int num = (long) arg; // the thread id
	struct msg Range;
	mailboxes->RecvMsg(num, &Range);  // recieving the message, then starts doing its job
	int begin = Range.value1;
	int end = Range.value2;

	for (int gen = 1; gen <= totalGens; gen++) {
		// by having it recieve a message again, it must wait for a message to be sent
		// in this case must waits for a message that has a type GO
		mailboxes->RecvMsg(num, &Range);

		// if recieved a message of type STOP, breaks out
		if (Range.type == STOP) {
			break;
		}

		int sameGrid = 1;
		int allDeadCells = 1;
		//int repeatedPattern = 1;

		for (int i = begin; i < end; i++) {
			for (int j = 0; j < columns; j++) {
				int neighbors = getNeighbors(rows, columns, i, j, (gen - 1));
				int val = -1;

				// if there are 3 live neighbors, new one is created
				if (neighbors == 3) {
						val = 1;
				}

				// if less than 2 neighbors and more than 3 neighbors, it dies
				else if (neighbors < 2 || neighbors > 3) {
					val = 0;
				}

				// even generation
				if (gen % 2 == 0) {

					// no change
					if (val == -1) {
						evenGrid[i][j] = oddGrid[i][j];
					}

					// the new value would be dependent upon the number of neighbors
					else {
						evenGrid[i][j] = val;
					}

					// checks to see if all dead cells
					if (evenGrid[i][j] != 0) {
						allDeadCells = 0;
					}

				}

				// odd generation
				else if (gen % 2 == 1) {
					
					// no change
					if (val == -1) {
						oddGrid[i][j] = evenGrid[i][j];
					}

					// the new value would be dependent upon the number of neighbors
					else {
						oddGrid[i][j] = val;
					}

					// checks to see if all dead cells
					if (oddGrid[i][j] != 0) {
						allDeadCells = 0;
					}

				}

				// checks to see if the grids are the same	
				if (evenGrid[i][j] != oddGrid[i][j]) {
					sameGrid = 0;
				}
				
			}			
		}
		
		// by this point, it has finished a generation
		struct msg *sendMessage = new struct msg;
		sendMessage->iSender = num;
		sendMessage->type = GENDONE;
		sendMessage->value1 = allDeadCells;
		sendMessage->value2 = sameGrid;
		mailboxes->SendMsg(0, sendMessage);
	}

	// function is all done
	// either completed all threads or met exceptions
	struct msg *send = new struct msg;
	send->iSender = num;
	send->type = ALLDONE;
	mailboxes->SendMsg(0, send);
}


int main(int argc, char *argv[]) {

	if (argc < 4) {
		cerr << "Not enough arguments were inputed in" << endl;
		cerr << "Program Terminating" << endl;
		return 1;
	}

	if (argc > 6) {
		cerr << "Inputed in too many arguments" << endl;
		cerr << "Program Terminating" << endl;
		return 1;
	}

	int num_of_threads;
	if (sscanf(argv[1], "%d", &num_of_threads) != 1) {
		cerr << "There was an error trying to get the number of threads" << endl;
		cerr << "Program Terminating" << endl;
		return 1;
	}

	if (num_of_threads > 10) {
		cerr << "The number of threads cannot be > 10" << endl;
		cerr << "Program Terminating" << endl;
		return 1;
	}

	//cout << num_of_threads << endl;
	if (sscanf(argv[3], "%d", &totalGens) != 1) {
		cerr << "There was an error trying to get the number of generations" << endl;
		cerr << "Program Terminating" << endl;
		return 1;
	}

	if ( (num_of_threads <= 0) || (totalGens <= 0) ) {
		cerr << "The number of threads or generations cannot be <= 0" << endl;
		cerr << "Program Terminating" << endl;
		return 1;
	}

	//cout << totalGens << endl;
	int doPrint = 0;
	if (argc > 4) {
		if (*argv[4] == 'y') {
			doPrint = 1; // user wants to print out the grids
		}
	}

	int doPause = 0;
	if (argc > 5) {
		if (*argv[5] == 'y') {
			doPause = 1; // the user wants program to pause after each generation
		}
	}

	const char* filename = argv[2];

	ifstream file;

	string lineInput;
	file.open(filename);

	rows = 0;
	columns = 0;

	if (!file) {
		cerr << "ERROR: Can't open the specified file" << endl;
		cerr << "Program Terminating" << endl;
		return 1;
	}

	// going through each line in the file
	while (getline(file, lineInput)) {
		
		istringstream stream (lineInput);
		//cout << lineInput << endl;
		int input = 0;
		int count = 0;

		// finding the number of characters in each line
		//count = lineInput.length();
		while (stream >> input) {
			count++;
		}

		// getting the number of columns
		if (columns == 0) {

			columns = count;
		}

		// getting the number of rows
		if (count != 0) {
			rows++;
		}

		//cout << columns << endl;
		//cout << endl;
	}

	file.close();

	if (rows == 0 || columns == 0) { 
		cerr << "ERROR: Grid is Invalid" << endl;
		cerr << "Program Terminating" << endl;
		return 1;
	}

	if (rows > MAXGRID || columns > MAXGRID) {
		cerr << "ERROR: Number of rows/columns exceeds the MAXGRID value" << endl;
		cerr << "Program Terminating" << endl;
		return 1;
	}

	// allocating the 2D int array
	evenGrid = createGrid(rows, columns);
	oddGrid = createGrid(rows, columns);


	file.open(filename);

	// filling the grid from the text file to evenGrid
	for (int i = 0; i < rows; i++){
		for (int j = 0; j < columns; j++){
			int input;
			file >> input;
			evenGrid[i][j] = input;
		}
	}

	file.close();

	//printGrid(rows, columns, evenGrid);
	
	// if too many threads than the range then have the number of threads be equaled to the range
	if (num_of_threads > rows) {
		num_of_threads = rows;
	}

	mailboxes = new mailbox(num_of_threads + 1); // defining the array of mailboxes

	pthread_t thread[num_of_threads]; 

	int perThread = rows / num_of_threads; // for each thread
	int remainingThreads = rows % num_of_threads; // in case of not evenly divisible

	int temp = 0;
	int beginRange, endRange; // will hold the values where the thread will begin with and end with

	for (long i = 0; i < num_of_threads; i++) {
		beginRange = temp; 
		endRange = beginRange + perThread; // 

		// obtaining all the remaining threads that haven't been included yet
		if (remainingThreads > 0) {
			remainingThreads--; // decrementing the remainder count
			endRange++; // for each iteration it will continue to add on the excluded remainding threads
		}

		//cout << "Begin " << beginRange << endl;
		//cout << "End " << endRange << endl;

		// creating the threads
		/* GUIDE TO UNDERSTANDING PARAMETERS OF "pthread_create" */
		// &thread[i] - the pointer that pthread_create will fill out with info on the thread it creates
		// NULL - read online to use as a standard for the pointer to the pthread_attr_t parameter
		// myThread - the function that the created thread will run on
		// (void *)(i + 1) - what I wanted to start up the thread with
		if (pthread_create(&thread[i], NULL, myThread, (void *)(i + 1)) != 0) { // WARNING
			cerr << "ERROR: Unable to create a thread" << endl;
			return 1; 
		}

		// storing info in a message to be sent to the mailbox at "i"
		struct msg *sendMessage = new struct msg;
		sendMessage->iSender = 0; 
		sendMessage->type = RANGE;
		sendMessage->value1 = beginRange; // the value that it would start adding from
		sendMessage->value2 = endRange; // the value that it would stop adding

		// sending the message to the mailbox
		int iTo = i + 1; 
		mailboxes->SendMsg(iTo, sendMessage); // sending the message to the created thread

		temp = endRange; // now defining the new range for the next threads
	}

	int numFinished = 0;
	int latestGen = 0;

	// playing the game
	for (int gen = 1; gen <= totalGens; gen++) {
		if (doPrint) {

			cout << "Generation " << gen - 1 << ":" << endl;
			printGrid(rows, columns, (gen - 1));
		}

		if (doPause) {
			getchar();
		}

		// since myThread function is waiting on another message
		// I must send messages to each of the threads in the mailbox
		for (int i = 0; i < num_of_threads; i++) {
			struct msg *sendMessage = new struct msg;
			sendMessage->iSender = 0;
			sendMessage->type = GO;

			mailboxes->SendMsg(i + 1, sendMessage);
		}

		int sameGrid = 1;
		int allDeadCells = 1;

		// now recieving back the messages from the parent thread
		for (int i = 0; i < num_of_threads; i++) {
			struct msg recievedMessage;
			mailboxes->RecvMsg(0, &recievedMessage);

			// if it got back an ALLDONE message then increment
			// the number of finished threads
			if (recievedMessage.type == ALLDONE) {
				numFinished++;
			}

			// checking to see if there were any allDeadCells
			if (recievedMessage.value1 == 0) {
				allDeadCells = 0;
			}

			// checking to see if the grids were the same
			if (recievedMessage.value2 == 0) {
				sameGrid = 0;
			}
		}

		latestGen = gen; // this is the latest generation

		// if meets these criterias then send STOP message to
		// each thread to have the myThread function stop doing its job
		if (sameGrid == 1 || allDeadCells == 1) {
			for (int i = 0; i < num_of_threads; i++) {
				struct msg *send = new struct msg;
				send->iSender = 0;
				send->type = STOP;

				mailboxes->SendMsg(i + 1, send);
			}

			if (doPrint) {
				cout << "Generation " << gen << ":" << endl;
				printGrid(rows, columns, (gen));
				cout << endl;
				cout << "... " << endl;
				cout << endl;
			}

			// no point in continuing the generations
			break;
		}
	}
	
	// emptying the parent thread
	for (int i = 0; i < (num_of_threads - numFinished); i++) {
		struct msg result;

		// must at least get one message back since has to go 
		// through at least one generation so use do while loop 
		do {
			mailboxes->RecvMsg(0, &result);
		} while (result.type != ALLDONE);
	}
	
	for (int i = 0; i < num_of_threads; i++) {
		(void)pthread_join(thread[i], NULL);
	}

	cout << "The game ends after " << totalGens << " generations with:" << endl;
	//cout << latestGen << endl;
	printGrid(rows, columns, latestGen);

	// freeing up the allocated memory
	delete evenGrid;
	delete oddGrid;
	delete mailboxes;

	return 0;
}
