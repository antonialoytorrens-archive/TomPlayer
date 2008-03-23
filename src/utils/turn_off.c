#include <sys/reboot.h>


/* this is brute force, but works */
void switch_off()
{
	reboot(RB_POWER_OFF);
}

int main(){
switch_off();
return 0;
}
