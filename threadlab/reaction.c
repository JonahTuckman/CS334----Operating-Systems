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
		struct condition HnotReady;
		struct condition OnotReady;
	};

void
reaction_init(struct reaction *reaction)
{
	reaction->numH = 0;
	reaction->numO = 0;
	lock_init(&reaction->makingWater);
	cond_init(&reaction->HnotReady);
	cond_init(&reaction->OnotReady);

}

void
reaction_h(struct reaction *reaction)
{
	lock_acquire(&reaction->makingWater);
	reaction->numH++;
	if((reaction->numH >= 2) && (reaction->numO >= 1)) {
		make_water();
		reaction->numH-=2;
		reaction->numO--;
		cond_signal(&reaction->HnotReady, &reaction->makingWater);
		lock_release(&reaction->makingWater);
	} else {
		cond_wait(&reaction->HnotReady, &reaction->makingWater);
		lock_release(&reaction->makingWater);
	}
}

void
reaction_o(struct reaction *reaction)
{
	lock_acquire(&reaction->makingWater);
	reaction->numO++;
	if((reaction->numH >= 2) & (reaction->numO >= 1)) {
		make_water();
		reaction->numH-=2;
		reaction->numO--;
		cond_signal(&reaction->OnotReady, &reaction->makingWater);
		lock_release(&reaction->makingWater);
	} else {
		cond_wait(&reaction->OnotReady, &reaction->makingWater);
		lock_release(&reaction->makingWater);
	}
}
