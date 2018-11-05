#include <iostream>
#include "Mailbox.h"
using namespace std;

// Jonathan Dang
// WPI ID: jdang

// Constructor of the mailbox object
mailbox::mailbox(int n) {
	// this is the array of mailboxes that has a size dependent upon how many threads 
	mailboxes = new struct msg * [n]; 
	
	sendSem = new sem_t [n];     // initializing n amount of send semaphores
	recieveSem = new sem_t [n];  // initializing n amount of recieve semaphores

	for(int i = 0; i < n; i++) {
		sem_init(&sendSem[i], 0, 1);
		sem_init(&recieveSem[i], 0, 0);
	}

	size = n; // sets the size of the mailbox to how many threads
}

// Destructor of the mailbox object
mailbox::~mailbox() {
	// something here
}


// sends a message to the targeted mailbox 
void mailbox::SendMsg(int iTo, struct msg *pMsg) {
	sem_wait(&sendSem[iTo]); // waits if another message is already in the mailbox that it's trying to send to 
	
	/*
	if (sem_wait(&sendSem[iTo]) == 0) {
		cout << "Waiting on Mailbox " << iTo << endl;
	}
	*/
	mailboxes[iTo] = pMsg; // stores the message that would be sent to the mailbox to be accessed for potential threads
	sem_post(&recieveSem[iTo]); // increments (unlocks) the semaphore pointed to by recieveSem[iTo]
}

// recieves the messsage from the targeted mailbox
void mailbox::RecvMsg(int iFrom, struct msg *pMsg) {
	sem_wait(&recieveSem[iFrom]); // waits if there's no messages in the mailbox that it's trying to access from
	*pMsg = *mailboxes[iFrom];  // fills in the msg struct with the received message 
	delete mailboxes[iFrom]; // deletes the message from the array of mailboxes
	sem_post(&sendSem[iFrom]); // increments (unlocks) the semaphore pointed to by sendSem[iFrom]
}
