/**
 * @file x11.c
 gcc x11.c -lX11 && ./a.out
 * @author db0@qq.com
 * @version 1.0.1
 * @date 2018-08-27
 */

#include<X11/Xlib.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main(void) {
	Display *d;
	Window w;
	XEvent e;
	char *msg = "Hello,World!中国";
	int s;


	d =XOpenDisplay(NULL);
	if (d == NULL) {
		fprintf(stderr, "Cannot open display\n");
		exit(1);
	}

	s =DefaultScreen(d);


	w =XCreateSimpleWindow(d, RootWindow(d, s), 10, 10, 100, 100, 1,
			BlackPixel(d, s),WhitePixel(d, s));


	XSelectInput(d, w,ExposureMask | KeyPressMask);


	XMapWindow(d, w);


	while (1) {

		XNextEvent(d, &e);


		if(e.type == Expose) {
			XFillRectangle(d, w, DefaultGC(d, s), 20, 20,10, 10);
			XDrawString(d, w, DefaultGC(d, s), 50, 50, msg,strlen(msg));
		}

		if(e.type == KeyPress)
			break;
	}


	XCloseDisplay(d);

	return 0;
}
