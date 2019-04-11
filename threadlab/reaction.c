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
	};

void
reaction_init(struct reaction *reaction)
{
	reaction->numH = 0;// number of H threads currently present
	reaction->numO = 0; // number of O threads currently present
	lock_init(&reaction->makingWater); 
	cond_init(&reaction->HSleep); // Hydrogen is asleep
	cond_init(&reaction->OSleep); // Oxygen is asleep
}

void
reaction_h(struct reaction *reaction)
{
	lock_acquire(&reaction->makingWater);
	reaction->numH++;

	cond_signal(&reaction->OSleep, &reaction->makingWater); // Every time we get a new H we will wake up the 
								// Sleeping O to check if we have enough
	cond_wait(&reaction->HSleep, &reaction->makingWater); // H then sleeps until O has created an H2O and wakes it

	
	lock_release(&reaction->makingWater);
}


void
reaction_o(struct reaction *reaction)
{
	lock_acquire(&reaction->makingWater);
	
	reaction->numO++;

	while(reaction->numH < 2) {
		cond_wait(&reaction->OSleep, &reaction->makingWater); // Will sleep until H wakes and O rechecks condition 
	}	
	make_water();
	reaction->numH-=2;
	reaction->numO--;

	cond_signal(&reaction->HSleep, &reaction->makingWater); // Wake H up, H2O is built
	cond_signal(&reaction->HSleep, &reaction->makingWater); // Two H threads are asleep because we need two per H2O
	lock_release(&reaction->makingWater);

}
