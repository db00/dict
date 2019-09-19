/**
 *
 gcc display.c mystring.c array.c -o ~/display && ~/display

motherboard:Intel Koloe G41+ICH6 Series
cpu:Intel Xeon E5430 @2.66GHz
mem: kingston 4GB DDR3 1333MHz
vga: G45/G43 Express Chipset(64MB)
audio:Intel 82801FB Realtek
net:RTL810x/8139 Family Fast Ethernet NIC
 */
#include "mystring.h"
int main()
{
	int run = 1;
	char * mode = mysystem("cvt 1366 768",NULL);
	//char * mode = mysystem("cvt 1440 900",NULL);
	printf("%s\n",mode);
	Array * array = string_split(mode,"\n");
	if(array->length!=3)
		return 1;
	//printf("%d\n",array->length);
	char * s = getStrBtw(Array_getByIndex(array,1)," ",NULL,1);
	char * newmode = contact_str("xrandr --newmode ",s);
	printf("%s\n",newmode);
	if(run)
		system(newmode);
	char * s1 = getStrBtw(Array_getByIndex(array,1)," "," ",1);
	//printf("%s\n",s1);
	char * addmode = contact_str("xrandr --addmode VGA1 ",s1);
	printf("%s\n",addmode);
	if(run)
		system(addmode);
	char * output = contact_str("xrandr --output VGA1 --mode ",s1);
	printf("%s\n",output);
	if(run)
		system(output);

	return 0;
}
