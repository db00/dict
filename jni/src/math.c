/**
 * @file math.c
 gcc math.c && ./a.out  
 * @author db0@qq.com
 * @version 1.0.1
 * @date 2017-07-17
 */

#include "math.h"
#include "stdlib.h"
#include "stdio.h"
int main()
{
	printf("%f\r\n",8280.09*1.06*1.06*1.06*1.06);
	printf("%f\r\n",log(6.0)/log(1.06));
	printf("%f\r\n",log(4.0)/log(1.048));
	return 0;
}
