//
// Jonah Tuckman
//

#include "thread.h"

struct station {
	int waitingPassenger;
	int standing;
	int allSeated;
	int numSeats;
//	int trainHere;
	struct lock fillingTrain;
	struct condition waitForTrain;
	struct condition waitForPass;
	struct condition waitToSeat;
};



void
station_init(struct station *station)
{
	station->waitingPassenger = 0;
	station->standing=0;
	station->numSeats=0;
	station->allSeated=0;
//	station->trainHere=0;
	lock_init(&station->fillingTrain);
	cond_init(&station->waitForTrain);
	cond_init(&station->waitForPass);
	cond_init(&station->waitToSeat);

}

void
station_load_train(struct station *station, int count)
{
	// CONDITION: May not return until train is fully loaded or all passengers are loaded
	// Do not want to wait idly, want to continue to add passengers to train so lock, not wait

	lock_acquire(&station->fillingTrain); // may not return until out of the while loop (one of the conditions is met)
	station->numSeats = count;
//	station->trainHere=1;

	// Wake up a waiting passenger and wait for them
	while((station->waitingPassenger > 0) && (count > 0)){
		cond_signal(&station->waitForTrain, &station->fillingTrain); // passanger that has been waiting has been loaded / signaled
		count--;
		cond_wait(&station->waitToSeat, &station->fillingTrain);
	}
	// This is signaled in load_passenger
	while((station->waitingPassenger != 0 && station->numSeats != 0) && station->standing != 0) { //run until the trainSpace or waitingPassengers has become 0
		cond_wait(&station->waitForPass, &station->fillingTrain);
	}

//station->trainHere=0
	lock_release(&station->fillingTrain); // Lock is unlocked because train has been loaded completely / passangers are gone


}

void
station_wait_for_train(struct station *station)
{
	lock_acquire(&station->fillingTrain);
	station->waitingPassenger++;
	///// ADD CONDITION HERE< CANNOT RETURN UNTIL LOAD TRAIN IS CALLED AND THERE IS SPACE

//HERE IS THE ISSUE
	while(station->numSeats == 0){ // Train is not here / no where to sit
	// Otherwise the other function is called and the check there with train space is done
		cond_wait(&station->waitForTrain, &station->fillingTrain);
	}

	if(station->numSeats != 0){
		station->standing++;
		station->waitingPassenger--; // station now has one less person waiting
		station->numSeats--; // the train now has one less seat
	}

	cond_signal(&station->waitToSeat, &station->fillingTrain);
	// if the while is not entered then the train is here and there is no waiting
	// the thread will go to the load train function
	lock_release(&station->fillingTrain);
}

void
station_on_board(struct station *station)
{
	lock_acquire(&station->fillingTrain);


		station->standing--;



	if((station->numSeats == 0 || station->waitingPassenger == 0) && station->standing == 0){
		cond_signal(&station->waitForPass, &station->fillingTrain);
	}
	lock_release(&station->fillingTrain);
}
