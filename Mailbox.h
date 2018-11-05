#define MAXTHREAD 10
#define RANGE 1
#define ALLDONE 2
#define GO 3
#define GENDONE 4 // Generation Done
#define STOP 5

#include <semaphore.h>

// Jonathan Dang
// WPI ID: jdang

struct msg {
	int iSender; /* sender of the message (0 .. number-of-threads) */
	int type;    /* its type */
	int value1;  /* first value */
	int value2;  /* first value */
};

class mailbox {

public:
	mailbox(int n);
	~mailbox();

	// class functions
	void SendMsg(int iTo, struct msg *pMsg); // msg as ptr, C/C++
	void RecvMsg(int iFrom, struct msg *pMsg); // msg as ptr, C/C++

private:
	struct msg **mailboxes; // the array of mailboxes
	sem_t *sendSem;
	sem_t *recieveSem;
	int size;

};
