#include <stdio.h>
#include "candrv.h"

int main(){
	printf("Hello");
	start_can_irq();
	while(1){}
	return 0;
}
