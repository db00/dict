/**
 * @file ttf3.c
 gcc ttf3.c -lm && ./a.out 
 * @author db0@qq.com
 * @version 1.0.1
 * @date 2016-06-14
 */
//#include <graphics.h>
#include <string.h>
#include <stdio.h>
//#include <conio.h>
#include <math.h>
//#include <mem.h>
//#include <alloc.h>
//#include <dir.h>
//#include <dos.h>
#include <stdlib.h>
#define  baseSize 168
#define  portion  100
int X0,Y0,xRange,yRange,curX,curY,contourNO;
int newX0,newY0,newxRange,newyRange;
static unsigned readCount_0=0,readCount_8=0;
unsigned char *wordCont_0=NULL,*wordCont_8=NULL;
unsigned bytesNum_0,bytesNum_8;
unsigned char myRead(int call_8_0);
void control(int *xx,int *yy,unsigned char controlWord,int nowxRange,
		int nowyRange,int nowX0,int nowY0,int call_8_0);
void showHanzi(char *fontfileName,int qw,int wm);
void b3_4(int *x,int *y,int col);
void b3_3(int *x,int *y,int col);
int main(void)
{
	unsigned char qm,wm;
	int i,a,b;
	char *hzstring="矢量汉字演示放大";
	//  struct ffblk ff;
	//registerbgidriver(EGAVGA_driver);
	//initgraph(&a,&b,"");
	//initgraph(800,600);
	//setbkcolor(WHITE);
	//cleardevice();
	//setcolor(BLACK);
	X0=Y0=1;xRange=yRange=110;
	// b=findfirst("*.ps",&ff,FA_ARCH);
	// while(!b) {
	for(i=a=0;i<strlen(hzstring);i+=2) 
	{
		qm=hzstring[i]-0xa0;
		wm=hzstring[i+1]-0xa0;
		showHanzi("/home/libiao/sound/DroidSansFallback.ttf",qm,wm);
		X0+=xRange-8;a++;
		if(a==7) {xRange=yRange=140;X0=0;Y0+=150;}
		if(a==13) {xRange=yRange=288;X0=150;Y0+=150;}
	}
	getchar();
	//b=findnext(&ff);
	X0=Y0=1;
	//cleardevice();
	//closegraph();
	return 0;
}
void showHanzi(char *fontFileName,int qm,int wm)
{
	long firstIndex,secondIndex,secondIndex8;
	unsigned char indexValue[2][3];
	long wordOffset,newOffset;
	FILE *fp;
	int xx[4],yy[4],controlWord,i,j,index2;
	bytesNum_0=0;
	readCount_0=0;
	readCount_8=0;
	if(wordCont_0) {
		free(wordCont_0);
		wordCont_0=NULL;
	}
	if((fp=fopen(fontFileName,"rb"))==NULL) {
		printf("Cannot open %s file\n",fontFileName);
		exit(0);
	}
	wordOffset=(71*94+94)*3;
	fseek(fp,wordOffset,SEEK_SET);
	fread(indexValue[0],3,1,fp);
	secondIndex=(long)indexValue[0][2]*0x10000+(long)indexValue[0][1]*0x100+
		(long)indexValue[0][0];
	wordOffset=(((long)qm-16)*94+wm-1)*3;
	fseek(fp,wordOffset,SEEK_SET);
	fread(indexValue[0],3,1,fp);
	fread(indexValue[1],3,1,fp);
	firstIndex=(long)indexValue[0][2]*0x10000+(long)indexValue[0][1]*0x100+
		(long)indexValue[0][0];
	bytesNum_0=(long)indexValue[1][2]*0x10000+(long)indexValue[1][1]*0x100+
		(long)indexValue[1][0]-firstIndex;
	wordCont_0=(unsigned char *)malloc(bytesNum_0);
	if(!wordCont_0) {
		/*puts("Out of memory!");*/
		return;
	}
	fseek(fp,firstIndex,SEEK_SET);
	fread(wordCont_0,1,bytesNum_0,fp);
	while(1) {
		if(readCount_0>=(2*bytesNum_0)) break;
		controlWord=myRead(0);
		if(controlWord<0x08) control(xx,yy,controlWord,xRange,yRange,X0,Y0,0);
		else {
			bytesNum_8=0;
			readCount_8=0;
			if(wordCont_8) {
				free(wordCont_8);
				wordCont_8=NULL;
			}
			switch(controlWord) {
				case 0x08:
					contourNO=0;
					index2=(int)myRead(0)*16+myRead(0)-8;
					newX0=(int)X0+(myRead(0)*16+myRead(0))*xRange/baseSize;
					newY0=(int)Y0+(myRead(0)*16+myRead(0))*yRange/baseSize;
					newxRange=(int)xRange*(myRead(0)*16+myRead(0))/portion;
					newyRange=(int)yRange*(myRead(0)*16+myRead(0))/portion;
					break;
				case 0x09:
					contourNO=0;
					newX0=(int)X0+(myRead(0)*16+myRead(0))*xRange/baseSize;
					newY0=(int)Y0+(myRead(0)*16+myRead(0))*yRange/baseSize;
					newxRange=(int)xRange*(myRead(0)*16+myRead(0))/portion;
					newyRange=(int)yRange*(myRead(0)*16+myRead(0))/portion;
					index2=(int)myRead(0)*16+myRead(0)+0xf6;
					break;
				case 0x0a:
					contourNO=0;
					newX0=(int)X0+(myRead(0)*16+myRead(0))*xRange/baseSize;
					newY0=(int)Y0+(myRead(0)*16+myRead(0))*yRange/baseSize;
					newxRange=(int)xRange*(myRead(0)*16+myRead(0))/portion;
					newyRange=(int)yRange*(myRead(0)*16+myRead(0))/portion;
					index2=(int)myRead(0)*16+myRead(0)+0x1f6;
					break;
				default:
					/*printf("\n******** >0x0a ********");*/
					return;
			}
			newOffset=secondIndex+index2*3;
			fseek(fp,newOffset,SEEK_SET);
			fread(indexValue[0],3,1,fp);
			fread(indexValue[1],3,1,fp);
			firstIndex=(long)indexValue[0][2]*0x10000+(long)indexValue[0][1]*0x100+
				(long)indexValue[0][0];
			bytesNum_8=(long)indexValue[1][2]*0x10000+(long)indexValue[1][1]*0x100+
				(long)indexValue[1][0]-firstIndex;
			secondIndex8=firstIndex+secondIndex;
			wordCont_8=(unsigned char *)malloc(bytesNum_8);
			if(!wordCont_8) {
				/*puts("Out of memory 8!\n");*/
				return;
			}
			fseek(fp,secondIndex8,SEEK_SET);
			fread(wordCont_8,1,bytesNum_8,fp);
			while(1) {
				if(readCount_8>=(2*bytesNum_8)) break;
				controlWord=myRead(8);
				if(controlWord>=0x80) {
					/*printf("\nToo many funtion call!!");*/
					return;
				}
				control(xx,yy,controlWord,newxRange,newyRange,newX0,newY0,8);
			}
			free(wordCont_8);
			wordCont_8=NULL;
		}
	}
	free(wordCont_0);
	wordCont_0=NULL;
	fclose(fp);
	return;
}
void control(int *xx,int *yy,unsigned char controlWord,int nowxRange,
		int nowyRange,int nowX0,int nowY0,int call_8_0)
{
	int i;
	switch(controlWord) {
		case 0:
			contourNO++;
			xx[0]=(int)myRead(call_8_0)*16+myRead(call_8_0);
			yy[0]=(int)myRead(call_8_0)*16+myRead(call_8_0);
			xx[0]=nowxRange*xx[0]/baseSize;
			yy[0]=nowyRange*yy[0]/baseSize;
			xx[0]+=nowX0;yy[0]+=nowY0;
			//moveto(xx[0],yy[0]);
			curX=xx[0];curY=yy[0];
			break;
		case 1:
			xx[0]=(int)myRead(call_8_0)*16+myRead(call_8_0);
			xx[0]=nowxRange*xx[0]/baseSize;
			xx[0]+=nowX0;
			//lineto(xx[0],curY);
			curX=xx[0];
			break;
		case 2:
			yy[0]=(int)myRead(call_8_0)*16+myRead(call_8_0);
			yy[0]=nowyRange*yy[0]/baseSize;
			yy[0]+=nowY0;
			//lineto(curX,yy[0]);
			curY=yy[0];
			break;
		case 3:
			xx[0]=(int)myRead(call_8_0)*16+myRead(call_8_0);
			yy[0]=(int)myRead(call_8_0)*16+myRead(call_8_0);
			xx[0]=nowxRange*xx[0]/baseSize;
			yy[0]=nowyRange*yy[0]/baseSize;
			xx[0]+=nowX0;yy[0]+=nowY0;
			//lineto(xx[0],yy[0]);
			curX=xx[0];curY=yy[0];
			break;
		case 4:
			xx[0]=curX;yy[0]=curY;
			xx[1]=(int)myRead(call_8_0)*16+myRead(call_8_0);
			yy[1]=(int)myRead(call_8_0)*16+myRead(call_8_0);
			xx[2]=(int)myRead(call_8_0)*16+myRead(call_8_0);
			yy[2]=(int)myRead(call_8_0)*16+myRead(call_8_0);
			xx[1]=nowxRange*xx[1]/baseSize;
			yy[1]=nowyRange*yy[1]/baseSize;
			xx[2]=nowxRange*xx[2]/baseSize;
			yy[2]=nowyRange*yy[2]/baseSize;
			xx[1]+=nowX0;yy[1]+=nowY0;
			xx[2]+=nowX0;yy[2]+=nowY0;
			b3_3(xx,yy,15);
			curX=xx[2];curY=yy[2];
			//moveto(curX,curY);
			break;
		case 5:
			xx[0]=curX;yy[0]=curY;
			xx[1]=(int)myRead(call_8_0)*16+myRead(call_8_0);
			yy[1]=(int)myRead(call_8_0)*16+myRead(call_8_0);
			xx[2]=(int)myRead(call_8_0)*16+myRead(call_8_0);
			yy[2]=(int)myRead(call_8_0)*16+myRead(call_8_0);
			xx[3]=(int)myRead(call_8_0)*16+myRead(call_8_0);
			yy[3]=(int)myRead(call_8_0)*16+myRead(call_8_0);
			xx[1]=nowxRange*xx[1]/baseSize;
			yy[1]=nowyRange*yy[1]/baseSize;
			xx[2]=nowxRange*xx[2]/baseSize;
			yy[2]=nowyRange*yy[2]/baseSize;
			xx[3]=nowxRange*xx[3]/baseSize;
			yy[3]=nowyRange*yy[3]/baseSize;
			for(i=1;i<4;i++) {
				xx[i]+=nowX0;yy[i]+=nowY0;
			}
			b3_4(xx,yy,15);
			curX=xx[3];curY=yy[3];
			//moveto(curX,curY);
			break;
		case 6:
			xx[0]=(int)myRead(call_8_0)*16+myRead(call_8_0);
			yy[0]=(int)myRead(call_8_0)*16+myRead(call_8_0);
			xx[1]=(int)myRead(call_8_0)*16+myRead(call_8_0);
			yy[1]=(int)myRead(call_8_0)*16+myRead(call_8_0);
			xx[0]=nowxRange*xx[0]/baseSize;
			yy[0]=nowyRange*yy[0]/baseSize;
			xx[1]=nowxRange*xx[1]/baseSize;
			yy[1]=nowyRange*yy[1]/baseSize;
			for(i=0;i<2;i++) {
				xx[i]+=nowX0;yy[i]+=nowY0;
			}
			//rectangle(xx[0],yy[0],xx[1],yy[1]);
			curX=xx[1];curY=yy[1];
			//moveto(curX,curY);
			break;
		case 7:
			/*printf("case 7\n");*/
			return;
		default:
			/*printf("\n******** Case>7 ********");*/
			return;
	}
}
unsigned char myRead(int call_8_0)
{
	switch(call_8_0) {
		case 0:
			return(0x0f&(wordCont_0[readCount_0/2]>>((1-((readCount_0++)%2))*4)));
		case 8:
			return(0x0f&(wordCont_8[readCount_8/2]>>((1-((readCount_8++)%2))*4)));
		default:
			/*printf("\n******** myRead is wrong! ********");*/
			return 1;
	}
}
void b3_4(int *x,int *y,int col)
{
	int n,*bx,*by,sign=1,j=0,k,i;
	double a,b,c,d,dt,xx,f0_3,f_0,f_1,f_2;
	n=xx=sqrt(((double)(x[1]-x[0]))*((double)(x[1]-x[0]))+
			((double)(y[1]-y[0]))*((double)(y[1]-y[0])))+
		sqrt(((double)(x[2]-x[1]))*((double)(x[2]-x[1]))+
				((double)(y[2]-y[1]))*((double)(y[2]-y[1])))+
		sqrt(((double)(x[3]-x[2]))*((double)(x[3]-x[2]))+
				((double)(y[3]-y[2]))*((double)(y[3]-y[2])));
	if(xx>n) n++;
	n*=2;
	if(n==0) n=20;
	do {
		bx=(int *)malloc(n*sizeof(int));
		by=(int *)malloc(n*sizeof(int));
		if((!bx)||(!by)) {
			/*printf("Memory Alloction Error!");*/
			return;
		}
		dt=(double)1/(double)n;
		a=((-1)*x[0]+3*x[1]-3*x[2]+x[3]);
		b=3*(x[0]-2*x[1]+x[2]);
		c=3*(x[1]-x[0]);
		d=x[0];
		f0_3=6*a*dt*dt*dt;
		f_2=f0_3+2*b*dt*dt;
		f_1=f0_3/6+b*dt*dt+c*dt;
		f_0=d;
		bx[0]=f_0+0.5;
		for(i=0;i<n;i++) {
			f_0+=f_1;
			bx[i]=f_0+0.5;
			f_1+=f_2;
			f_2+=f0_3;
		}
		a=((-1)*y[0]+3*y[1]-3*y[2]+y[3]);
		b=3*(y[0]-2*y[1]+y[2]);
		c=3*(y[1]-y[0]);
		d=y[0];
		f0_3=6*a*dt*dt*dt;
		f_2=f0_3+2*b*dt*dt;
		f_1=f0_3/6+b*dt*dt+c*dt;
		f_0=d;
		bx[0]=f_0+0.5;
		for(i=0;i<n;i++) {
			f_0+=f_1;
			by[i]=f_0+0.5;
			f_1+=f_2;
			f_2+=f0_3;
		}
		sign=1;j=0;k=n/100+1;
		for(i=1;i<n;i++) {
			if((abs(bx[i]-bx[i-1])>1)||(abs(by[i]-by[i-1])>1)) j++;
			if(j>k) {
				free(bx); free(by);
				if(n>3) n=n+n/3;
				else n=6;
				sign=0;
			} break;
		}
	} while(sign==0);
	//for(i=1;i<n;i++) if((bx[i]!=bx[i-1])||(by[i]!=by[i-1])) putpixel(bx[i],by[i],col);
	free(bx); free(by);
}

void b3_3(int *x,int *y,int col)
{
	int n,*bx,*by,sign=1,j=0,k,i;
	double a,b,c,d,dt,xx,f0_3,f_0,f_1,f_2;
	n=xx=sqrt(((double)(x[1]-x[0]))*((double)(x[1]-x[0]))+
			((double)(y[1]-y[0]))*((double)(y[1]-y[0])))+
		sqrt(((double)(x[2]-x[1]))*((double)(x[2]-x[1]))+
				((double)(y[2]-y[1]))*((double)(y[2]-y[1])));
	if(xx>n) n++;
	n*=4;
	if(n==0) n=20;
	do {
		bx=(int *)malloc(n*sizeof(int));
		by=(int *)malloc(n*sizeof(int));
		if((!bx)||(!by)) {
			/*printf("Memory Alloction Error!");*/
			return;
		}
		dt=(double)1/(double)n;
		b=x[0]-2*x[1]+x[2];
		c=2*(x[1]-x[0]);
		d=x[0];
		f_2=2*b*dt*dt;
		f_1=b*dt*dt+c*dt;
		f_0=d;
		bx[0]=f_0+0.5;
		for(i=0;i<n;i++) {
			f_0+=f_1;
			bx[i]=f_0+0.5;
			f_1+=f_2;
		}
		b=y[0]-2*y[1]+y[2];
		c=2*(y[1]-y[0]);
		d=y[0];
		f_2=2*b*dt*dt;
		f_1=b*dt*dt+c*dt;
		f_0=d;
		by[0]=f_0+0.5;
		for(i=0;i<n;i++) {
			f_0+=f_1;
			by[i]=f_0+0.5;
			f_1+=f_2;
		}
		sign=1;j=0;k=n/100+1;
		for(i=1;i<n;i++) {
			if((abs(bx[i]-bx[i-1])>1)||(abs(by[i]-by[i-1])>1)) j++;
			if(j>k) {
				free(bx); free(by);
				if(n>3) n=n+n/3;
				else n=6;
				sign=0;
			} break;
		}
	} while(sign==0);
	//for(i=1;i<n;i++) if((bx[i]!=bx[i-1])||(by[i]!=by[i-1])) putpixel(bx[i],by[i],col);
	free(bx); free(by);
}
