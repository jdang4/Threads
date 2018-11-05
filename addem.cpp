#include <iostream>
#include <inttypes.h>
#include "Mailbox.h"
using namespace std;

// Jonathan Dang
// WPI ID: jdang

mailbox *mailboxes; // global variable to allow multiple threads to access the array of mailboxes 

// defining the function that each thread will be responsible in doing
void *myThread(void* arg) {
	int result = 0;
	int num = (long) arg; // the value of the current "i"
	struct msg Range;
	mailboxes->RecvMsg(num, &Range);  // recieving the message, then starts doing its job
	int begin = Range.value1;
	int end = Range.value2;

	// adding up the numbers within its range
	for (int i = begin; i < end; i++) {
		result += i;
	}

	struct msg *sendMessage = new struct msg;
	sendMessage->iSender = num;
	sendMessage->type = ALLDONE;
	sendMessage->value1 = result;

	// mailboxes[0] will be the position to store the new message with the added up result
	mailboxes->SendMsg(0, sendMessage);
}

int main(int argc, char *argv[]) {	

	int totalSum = 0; // the total sum

	/* ERROR CHECKING FOR INPUTS */
	if (argc < 3) {
		cerr << "Not enough arguments were inputed in" << endl;
		cerr << "Program Terminating" << endl;
		return 1;
	}

	if (argc > 3) {
		cerr << "Inputed in too many arguments" << endl;
		cerr << "Program Terminating" << endl;
		return 1;
	}

	int num_of_threads;
	int range;

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

	if (sscanf(argv[2], "%d", &range) != 1) {
		cerr << "There was an error trying to get the range value" << endl;
		cerr << "Program Terminating" << endl;
		return 1;
	}

	if ( (num_of_threads <= 0) || (range <= 0) ) {
		cerr << "The number of threads or range cannot be <= 0" << endl;
		cerr << "Program Terminating" << endl;
		return 1;
	}

	// if too many threads than the range then have the number of threads be equaled to the range
	if (num_of_threads > range) {
		num_of_threads = range;
	}

	mailboxes = new mailbox(num_of_threads + 1); // defining the array of mailboxes

	pthread_t thread[num_of_threads]; 

	int perThread = range / num_of_threads; // for each thread
	int remainingThreads = range % num_of_threads; // in case of not evenly divisible

	int temp = 1;
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

		//cout << "Printy " << i << endl;
		// storing info in a message to be sent to the mailbox at "i"
		struct msg *sendMessage = new struct msg;
		sendMessage->iSender = 0; // this part doesn't really matter 
		sendMessage->type = RANGE;
		sendMessage->value1 = beginRange; // the value that it would start adding from
		sendMessage->value2 = endRange; // the value that it would stop adding

		// sending the message to the mailbox
		int iTo = i + 1; 
		mailboxes->SendMsg(iTo, sendMessage); // sending the message to the created thread

		temp = endRange; // now defining the new range for the next threads
	}

	//cout << "testy1" << endl;

	// getting the added up results from each thread	
	for (int i = 0; i < num_of_threads; i++) {
		struct msg result;

		/* Note: it will always have first parameter 0 because that is where
		         the myThread function is sending the new msg to
		*/
		mailboxes->RecvMsg(0, &result); // stores the recieved message into result 
		//cout << "Printy2" << endl;
		totalSum += result.value1; // getting the total sum
	}

	cout << "The total for 1 to " << argv[2] << " using " << argv[1] << " threads is " << totalSum << endl;
	
	for (int i = 0; i < num_of_threads; i++) {
		(void)pthread_join(thread[i], NULL);
	}

	delete mailboxes;

	return 0;
}
