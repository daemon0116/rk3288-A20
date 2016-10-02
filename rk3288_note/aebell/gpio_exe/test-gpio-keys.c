#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>   
#include <errno.h>

#include <sys/types.h>   
#include <sys/time.h>   
#include <fcntl.h>   
#include <sys/ioctl.h>   
#include <linux/input.h>
int main ()  
{  
	int keys_fd;  
	char ret[2];  
	struct input_event t;  

	keys_fd = open ("/dev/input/event2", O_RDONLY);  
	if (keys_fd <= 0)  
	{  
		printf ("open /dev/input/event2 device error!\n");  
		return 0;  
	}  
	while (1)  
	{  
		if (read (keys_fd, &t, sizeof (t)) == sizeof (t))  
		{  
			if (t.type == EV_KEY)  
				if (t.value == 0 || t.value == 1)  
				{  
					printf ("key %d %s\n", t.code,  (t.value) ? "up" : "down");  
				}  
		}  
		usleep(100000);
	}  
	close (keys_fd);  

	return 0;  
}  