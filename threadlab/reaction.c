//
// Jonah Tuckman
//

#include "thread.h"

// Forward declaration. This function is implemented in reaction-runner.c,
// but you needn't care what it does. Just be sure it's called when
// appropriate within reaction_o()/reaction_h().
void make_water();

struct reaction {
		int numH;
		int numO;
		struct lock makingWater;
		struct condition HSleep;
		struct condition OSleep;
		struct condition OneO;
	};

void
reaction_init(struct reaction *reaction)
{
	reaction->numH = 0;
	reaction->numO = 0;
	lock_init(&reaction->makingWater);
	cond_init(&reaction->HSleep);
	cond_init(&reaction->OSleep);
	cond_init(&reaction->OneO);
}

void
reaction_h(struct reaction *reaction)
{
	lock_acquire(&reaction->makingWater);
	reaction->numH++;

	if (reaction->numH == 1){ // If we only have 1 H we dont have enough and wait
		cond_wait(&reaction->HSleep, &reaction->makingWater);
		lock_release(&reaction->makingWater);
	}
	if (reaction-> numH >= 2 && reaction->numO == 0) { // We have two Hs but no Os
		cond_wait(&reaction->OSleep, &reaction->makingWater); // Unlocked when a water comes
		cond_signal(&reaction->HSleep, &reaction->makingWater); // This is locked from when we have 1 H
		make_water();
		reaction->numH-=2;
		reaction->numO--;
		lock_release(&reaction->makingWater);
		//return;
	}
	if((reaction->numH >= 2) && (reaction->numO >= 1)) { // If we have all we need up front
		make_water();
		reaction->numH-=2;
		reaction->numO--;
		cond_signal(&reaction->OSleep, &reaction->makingWater);
		cond_signal(&reaction->HSleep, &reaction->makingWater);
		lock_release(&reaction->makingWater);
		//return;
	}
}


void
reaction_o(struct reaction *reaction)
{
	reaction->numO++;
	if(reaction->numH < 2){ // Os are waiting until we have enough Hs to wake one
		lock_acquire(&reaction->makingWater);
		cond_wait(&reaction->OSleep, &reaction->makingWater);
		lock_release(&reaction->makingWater);
		}
	if(reaction->numH >= 2 && reaction -> numO == 1){ // We have been waiting on an O
		lock_acquire(&reaction->makingWater);
		cond_signal(&reaction->OSleep, &reaction->makingWater);
		lock_release(&reaction->makingWater);
	}

}
