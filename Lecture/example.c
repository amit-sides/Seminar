#include <string.h>
#include <stdio.h>

#define NAME_LENGTH (18)

typedef struct user_s {
	int 	x;
	int 	y;
	char	name[NAME_LENGTH];
} user_t;

int example()
{
	user_t user = {0};
	char *name = "Mister Elon Musk";
	int x = 0x53;
	int y = 0xA9;

	// Init user
	user.x = x;
	user.y = y;
	if (strlen(name) >= NAME_LENGTH)
		return -1;

	memcpy(&user.name, name, strlen(name) * sizeof(*name));

	// Print user
	printf("User's X is :\t%d\n", user.x);
	printf("User's Y is :\t%d\n", user.y);
	printf("User's name is:\t%s\n", user.name);
	return 0;
}




int main()
{
	//printf("Result: %d\n", example());
	example();
	return 0;
}