/**
 *
 gcc -Wall -I"../SDL2/include/" ease.c array.c  tween.c matrix.c sprite.c -lSDL2 -D debug_tween -lm && ./a.out
 gcc -D test_ease ease.c -lm && ./a.out 
 */
#include "ease.h"
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

static float const strong_power = 4;
static float const quad_power = 1;
static float const cubic_power= 2;
static float const quart_power = 3;
static float const quint_power = 4;

float easeNone_linear(float t,float b,float c,float d){
	return c*t/d + b;
}
float easeIn_linear(float t,float b,float c,float d){
	return c*t/d + b;
}
float easeOut_linear(float t,float b,float c,float d){
	return c*t/d + b;
}
float easeInOut_linear(float t,float b,float c,float d){
	return c*t/d + b;
}

float easeIn_strong(float t,float b,float c,float d) {
	t /= d;
	return c*(t)*pow(t,strong_power) + b;
}
float easeOut_strong(float t,float b,float c,float d) {
	t =t/d-1;
	return c*((t)*pow(t,strong_power) + 1) + b;
}
float easeInOut_strong(float t,float b,float c,float d) {
	if ((t/=d*0.5) < 1) return c*0.5*t*pow(t,strong_power) + b;
	t-=2;
	return c*0.5*((t)*pow(t,strong_power) + 2) + b;
}

float easeIn_sine (float t,float b,float c,float d) {
	return -c * cos(t/d * M_PI*.5) + c + b;
}
float easeOut_sine (float t,float b,float c,float d) {
	return c * sin(t/d * M_PI*.5) + b;
}
float easeInOut_sine (float t,float b,float c,float d) {
	return -c*0.5 * (cos(M_PI*t/d) - 1) + b;
}

float easeIn_quint (float t,float b,float c,float d) {
	t=t/d;
	return c*(t)*pow(t,quint_power) + b;
}
float easeOut_quint (float t,float b,float c,float d) {
	t=t/d-1;
	return c*((t)*pow(t,quint_power) + 1) + b;
}
float easeInOut_quint (float t,float b,float c,float d) {
	if ((t/=d*0.5) < 1) return c*0.5*pow(t,quint_power+1) + b;
	t-=2;
	return c*0.5*((t)*pow(t,quint_power) + 2) + b;
}

float easeIn_quad (float t,float b,float c,float d) {
	t=t/d;
	return c*pow(t,quad_power+1) + b;
}
float easeOut_quad (float t,float b,float c,float d) {
	t=t/d;
	return -c *pow(t,quad_power)*(t-2) + b;
}
float easeInOut_quad (float t,float b,float c,float d) {
	if ((t/=d*0.5) < 1) return c*0.5*pow(t,quad_power+1) + b;
	--t;
	return -c*0.5 * (pow(t,quad_power)*(t-2) - 1) + b;
}

float easeIn_quart (float t,float b,float c,float d) {
	t=t/d;
	return c*(t)*pow(t,quart_power) + b;
}
float easeOut_quart (float t,float b,float c,float d) {
	t=t/d-1;
	return -c * ((t)*pow(t,quart_power) - 1) + b;
}
float easeInOut_quart (float t,float b,float c,float d) {
	if ((t/=d*0.5) < 1) return c*0.5*pow(t,quart_power+1) + b;
	t-=2;
	return -c*0.5 * (pow(t,quart_power+1) - 2) + b;
}

float easeIn_expo(float t,float b,float c,float d) {
	return (t==0) ? b : c * pow(2, 10 * (t/d - 1)) + b - c * 0.001;
}
float easeOut_expo(float t,float b,float c,float d) {
	return (t==d) ? b+c : c * (-pow(2, -10 * t/d) + 1) + b;
}
float easeInOut_expo(float t,float b,float c,float d) {
	if (t==0) return b;
	if (t==d) return b+c;
	if ((t/=d*0.5) < 1) return c*0.5 * pow(2, 10 * (t - 1)) + b;
	return c*0.5 * (-pow(2, -10 * --t) + 2) + b;
}

float easeIn_elastic (float t,float b,float c,float d) {
	int a = 0;
	int p = 0;
	float s;
	if (t==0) return b;
	//if ((t/=d)==1) return b+c; 
	t/=d; if(t==1)return b+c;

	if (!p) p=d*.3;
	if (!a || (c > 0 && a < c) || (c < 0 && a < -c)) { a=c; s = p/4; }
	else s = p/(2*M_PI) * asin (c/a);
	t= t-1;
	return -(a*pow(2,10*(t)) * sin( (t*d-s)*(2*M_PI)/p )) + b;
}
float easeOut_elastic (float t,float b,float c,float d) {
	int a = 0;
	int p = 0;
	float s;
	if (t==0) return b;
	//if ((t/=d)==1) return b+c;
	t/=d;
	if(t==1)return b+c;

	if (!p) p=d*.3;
	if (!a || (c > 0 && a < c) || (c < 0 && a < -c)) { a=c; s = p/4; }
	else s = p/(2*M_PI) * asin (c/a);
	return (a*pow(2,-10*t) * sin( (t*d-s)*(2*M_PI)/p ) + c + b);
}
float easeInOut_elastic (float t,float b,float c,float d) {
	int a = 0;
	int p = 0;
	float s;
	if (t==0) return b;
	//if ((t/=d*0.5)==2) return b+c;
	t/=d*0.5;
	if(t==2)return b+c;

	if (!p) p=d*(.3*1.5);
	if (!a || (c > 0 && a < c) || (c < 0 && a < -c)) { a=c; s = p/4; }
	else s = p/(2*M_PI) * asin (c/a);
	if (t < 1){
		t -= 1;
		return -.5*(a*pow(2,10*(t)) * sin( (t*d-s)*(2*M_PI)/p )) + b;
	}
	t -= 1;
	return a*pow(2,-10*(t)) * sin( (t*d-s)*(2*M_PI)/p )*.5 + c + b;
}

float easeIn_circ (float t,float b,float c,float d) {
	t/= d;
	return -c * (sqrt(1 - (t)*t) - 1) + b;
}
float easeOut_circ (float t,float b,float c,float d) {
	t= t/d-1;
	return c * sqrt(1 - (t)*t) + b;
}
float easeInOut_circ (float t,float b,float c,float d) {
	t/=d*.5;
	if ((t) < 1) return -c*0.5 * (sqrt(1 - t*t) - 1) + b;
	t-=2;
	return c*0.5 * (sqrt(1 - (t)*t) + 1) + b;
}

float easeOut_bounce (float t,float b,float c,float d) {
	t/=d;
	if (t < (1/2.75)) {
		return c*(7.5625*t*t) + b;
	} else if (t < (2/2.75)) {
		t -= 1.5/2.75;
		return c*(7.5625*(t)*t + .75) + b;
	} else if (t < (2.5/2.75)) {
		t -= 2.25/2.75;
		return c*(7.5625*(t)*t + .9375) + b;
	} else {
		t -= 2.625/2.75;
		return c*(7.5625*(t)*t + .984375) + b;
	}
}
float easeIn_bounce (float t,float b,float c,float d) {
	return c - easeOut_bounce(d-t, 0, c, d) + b;
}
float easeInOut_bounce (float t,float b,float c,float d) {
	if (t < d*0.5) return easeIn_bounce (t*2, 0, c, d) * .5 + b;
	else return easeOut_bounce (t*2-d, 0, c, d) * .5 + c*.5 + b;
}

float easeIn_cubic (float t,float b,float c,float d) {
	t /= d;
	return c*(t)*pow(t,cubic_power) + b;
}
float easeOut_cubic (float t,float b,float c,float d) {
	t= t/d-1;
	return c*((t)*pow(t,cubic_power) + 1) + b;
}
float easeInOut_cubic (float t,float b,float c,float d) {
	if ((t/=d*0.5) < 1) return c*0.5*t*pow(t,cubic_power) + b;
	t-=2;
	return c*0.5*((t)*pow(t,cubic_power) + 2) + b;
}

float easeIn_back (float t,float b,float c,float d) {
	float s = 1.70158;
	t/=d;
	return c*(t)*t*((s+1)*t - s) + b;
}
float easeOut_back (float t,float b,float c,float d) {
	float s = 1.70158;
	t = t/d-1;
	return c*((t)*t*((s+1)*t + s) + 1) + b;
}
float easeInOut_back (float t,float b,float c,float d) {
	float s = 1.70158;
	s *= 1.525;
	if ((t/=d*0.5) < 1){
		return c*0.5*(t*t*((s+1)*t - s)) + b;
	}
	t-=2;
	return c/2*((t)*t*(((s)+1)*t + s) + 2) + b;
}
#ifdef test_ease
int main()
{
	printf("%f",easeIn_strong(1.,0.0,1.0,10.0));
	return 0;
}
#endif
