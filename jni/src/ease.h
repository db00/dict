#ifndef ease_h
#define ease_h
#include <math.h>
#include <stdio.h>
/**
 *  
 *
 * @param t	current time
 * @param b	start value
 * @param c	end value
 * @param d	duration time(total time)
 *
 * @return  current value
 */
float easeNone_linear(float t,float b,float c,float d);
float easeIn_linear(float t,float b,float c,float d);
float easeOut_linear(float t,float b,float c,float d);
float easeInOut_linear(float t,float b,float c,float d);
float easeIn_strong(float t,float b,float c,float d);
float easeOut_strong(float t,float b,float c,float d);
float easeInOut_strong(float t,float b,float c,float d);
float easeIn_sine (float t,float b,float c,float d);
float easeOut_sine (float t,float b,float c,float d);
float easeInOut_sine (float t,float b,float c,float d);
float easeIn_quint (float t,float b,float c,float d);
float easeOut_quint (float t,float b,float c,float d);
float easeInOut_quint (float t,float b,float c,float d);
float easeIn_quad (float t,float b,float c,float d);
float easeOut_quad (float t,float b,float c,float d);
float easeInOut_quad (float t,float b,float c,float d);
float easeIn_quart (float t,float b,float c,float d);
float easeOut_quart (float t,float b,float c,float d);
float easeInOut_quart (float t,float b,float c,float d);
float easeIn_expo(float t,float b,float c,float d);
float easeOut_expo(float t,float b,float c,float d);
float easeInOut_expo(float t,float b,float c,float d);
float easeIn_elastic (float t,float b,float c,float d);
float easeOut_elastic (float t,float b,float c,float d);
float easeInOut_elastic (float t,float b,float c,float d);
float easeIn_circ (float t,float b,float c,float d);
float easeOut_circ (float t,float b,float c,float d);
float easeInOut_circ (float t,float b,float c,float d);
float easeOut_bounce (float t,float b,float c,float d);
float easeIn_bounce (float t,float b,float c,float d);
float easeInOut_bounce (float t,float b,float c,float d);
float easeIn_cubic (float t,float b,float c,float d);
float easeOut_cubic (float t,float b,float c,float d);
float easeInOut_cubic (float t,float b,float c,float d);
float easeIn_back (float t,float b,float c,float d);
float easeOut_back (float t,float b,float c,float d);
float easeInOut_back (float t,float b,float c,float d);
#endif
