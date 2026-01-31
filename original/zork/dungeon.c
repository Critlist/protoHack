#include "zork.h"
main()
{
	if(init()) game();
	else {
 puts("Suddenly a sinister, wraithlike figure appears before you");
 puts("seeming to float in the air.  In a low, sorrowful voice he says,");
 puts("\"Alas, the very nature of the world has changed, and the dungeon");
 puts("cannot be found.  All must now pass away.\"  Raising his oaken staff");
 puts("in farewell, he fades into the spreading darkness.  In his place");
 puts("appears a tastefully lettered sigh reading:\n");
 puts("		INITIALIZATION FAILURE\n");
 puts("The darkness becomes all encompassing, and your vision fails.");
		exit(2);
	}
}
