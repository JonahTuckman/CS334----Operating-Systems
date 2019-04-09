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
		struct condition OSleep1;
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
	cond_init(&reaction->OSleep1);
}

void
reaction_h(struct reaction *reaction)
{
	lock_acquire(&reaction->makingWater);
	reaction->numH++;

	cond_signal(&reaction->OSleep, &reaction->makingWater);
	cond_wait(&reaction->HSleep, &reaction->makingWater);



	lock_release(&reaction->makingWater);
}


void
reaction_o(struct reaction *reaction)
{
	lock_acquire(&reaction->makingWater);

	reaction->numO++;

	while(reaction->numH < 2){
		cond_wait(&reaction->OSleep, &reaction->makingWater);
	}
		make_water();
		reaction->numH-=2;
		reaction->numO--;
		//cond_signal(&reaction->OSleep, &reaction->makingWater);
		cond_signal(&reaction->HSleep, &reaction->makingWater);
		cond_signal(&reaction->HSleep, &reaction->makingWater);

	lock_release(&reaction->makingWater);

}
