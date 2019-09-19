/*
 *
 gcc -Wall x.c -lm && ./a.out -A test2.xls 
 */
#include <err.h>
#include <fcntl.h>
#include <getopt.h>
#include <math.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#define TAB(S,T,N) (*(T*)((char*)(S).tab + (N) * sizeof(T)))
#define BADSEC (-5)
typedef struct list
{/*{{{*/
	struct list *next, *prev;
} list_t;/*}}}*/

static inline void list_link(struct list *a, struct list *b)
{/*{{{*/
	a->next = b;
	b->prev = a;
}/*}}}*/
static inline void list_add(struct list *head, struct list *item)
{/*{{{*/
	struct list *first = head->next;
	list_link(head, item);
	list_link(item, first);
}/*}}}*/
#define LIST(L) struct list L = {&L, &L}
#define list_item(L, T, M) ((T*)((char*)(L) - (long)(&((T*)0)->M)))
/* remove first element and return it */
static inline struct list *list_get(struct list *head)
{/*{{{*/
	struct list *item = head->next;
	struct list *next = item->next;
	list_link(head, next);
	return item;
}/*}}}*/
/* remove first element, initialize and return it */
static inline struct list *list_get_init(struct list *head)
{/*{{{*/
	struct list *item = head->next;
	struct list *next = item->next;
	list_link(item, item);
	list_link(head, next);
	return item;
}/*}}}*/
struct ummap
{/*{{{*/
	list_t list;
	void *addr;
	int size;
	int (*handler)(struct ummap *, void *);
};/*}}}*/
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed int s32;
typedef unsigned long
#ifndef __LP64__
long
#endif
u64;
#ifdef __i386__
#define g16(P) (*(u16*)(P))
#define g32(P) (*(u32*)(P))
#define g64(P) (*(u64*)(P))
#define p16(P,V) (*(u16*)(P)=(V))
#else
static inline u16 g16(void *p) {return ((u8*)p)[0] | ((u8*)p)[1]<<8;}
static inline u32 g32(void *p) {return g16(p) | g16((u8*)p+2)<<16;}
static inline u64 g64(void *p) {return g32(p) | (u64)g32((u8*)p+4)<<32;}
static inline void p16(void *p, u16 v) {((u8*)p)[0]=v; ((u8*)p)[1]=v>>8;}
#endif
#define elemof(T) (sizeof T/sizeof*T)
#define endof(T) (T+elemof(T))
typedef struct 
{/*{{{*/
	u8 *ptr;
	unsigned len;
} meml_t;/*}}}*/
double ieee754(u64);
void set_codepage(int n);	// sheet codepage
u8 *print_uni(u8 *p, int l, u8 f,char *out);
u8 *print_cp_str(u8 *p, int l,char *out);
#define TRUNC errx(1, "Truncated  &%d", __LINE__)
#define BADF(T) errx(1, *T""?T"  &%d":"Format error  &%d", __LINE__);
struct g
{/*{{{*/
	int nr; // sheet number
	int row, col; // current pos
	unsigned top, bottom, left, right;
} g;/*}}}*/
struct sst
{/*{{{*/
	u8 *ptr, *rend;
};/*}}}*/
struct fmt
{/*{{{*/
	unsigned type:8; // 1:num, 3:date, 4:time, 5:date-time
	unsigned arg:8;
};/*}}}*/
struct tab
{/*{{{*/
	void *tab;
	int nelem, aelem, esize;
};/*}}}*/
struct xls
{/*{{{*/
	meml_t map;
	u8 *end;
	u8 *shptr;
	enum {BIFF2=2,BIFF3=3,BIFF4=4,BIFF5=5,BIFF8=6} biffv;
	unsigned e1904;

	struct sst *sst;
	unsigned nsst;

	struct tab fmt;
	struct tab xf_ptr;
	struct tab xf_fmt;
};/*}}}*/
static struct xls x;
void print_xls(char*out);
static inline void *tab_ptr(struct tab *tab, unsigned n)
{/*{{{*/
	return (char *)tab->tab + n * tab->esize;
}/*}}}*/
void *tab_alloc(struct tab *tab, unsigned n, const void *dflt)
{/*{{{*/
	u8 *p;

	if(n < tab->nelem)
		return tab_ptr(tab, n);
	if(n >= tab->aelem) {
		int sz = (n+16) & ~15;
		tab->aelem = sz;
		tab->tab = realloc(tab->tab, sz * tab->esize);
		if(!tab->tab) err(1, "realloc");
	}
	p = tab_ptr(tab, tab->nelem);
	n -= tab->nelem;
	tab->nelem += n + 1;
	if(n) do {
		memcpy(p, dflt, tab->esize);
		p += tab->esize;
	} while(--n);
	return p;
}/*}}}*/
void check_biffv(u8 *p)
{/*{{{*/
	int v;
	if(p[0]!=9)
		errx(1, "Format not recognized");
	switch(p[1]) {
		case 0:
biff2:
			errx(1, "BIFF2 not supported");
		case 2: v = BIFF3; goto ok;
		case 4: v = BIFF4; goto ok;
		case 8: break;
		default:
nsupp:
				errx(1, "Format not supported");
	}
	switch(p[5]) {
		case 0: v = BIFF5; break;
		case 2: goto biff2;
		default:
				v = p[5];
				if(v<BIFF3 || v>BIFF8)
					goto nsupp;
	}
ok:
	x.biffv = v;
}/*}}}*/
static u8 *print_str(u8 *p, int l,char * out)
{/*{{{*/
	if(x.biffv < BIFF8) {
		p = print_cp_str(p, l,out+strlen(out));
	} else {
		u8 f = *p++;
		int a=0;
		if(f&8) {a += 4*g16(p); p += 2;}
		if(f&4) {a += g32(p); p += 4;}
		p = a + print_uni(p, l, f,out+strlen(out));
	}
	return p;
}/*}}}*/
static void print_sst(int n,char *out)
{/*{{{*/
	u8 *p, *re, f;
	unsigned l;

	if(n<0 || n>=x.nsst)
		BADF("Wrong string index");

	p = x.sst[n].ptr;
	re = x.sst[n].rend;
	l = g16(p); f = p[2]; p += 3;
	p += (f&8 ? 2 : 0) + (f&4 ? 4 : 0);
	for(;;) {
		int s = re - p;
		f &= 1;
		if(l <= s>>f)
			break;

		if(re[0] != 0x3C) // CONTINUE
			BADF("String truncated");

		l -= s>>f;
		if(s&f)
			BADF("String cut at the middle of a char");
		print_uni(p, s>>f, f,out+strlen(out));

		p = re + 4;
		re = p + g16(re+2);
		f = *p++;
	}
	print_uni(p, l, f,out+strlen(out));
}/*}}}*/
static u8 *read_sst(u8 *p, u8 *re, u8 *fe)
{/*{{{*/
	unsigned nsst;

	x.nsst = g32(p+4);
	if(!x.nsst)
		return re;

	x.sst = calloc(x.nsst, sizeof *x.sst);
	if(!x.sst) err(1, "calloc");

	p += 8;

	for(nsst = 0;;) {
		unsigned l, a;
		u8 f;

		if(re-p < 3)
			BADF("String table truncated");

		x.sst[nsst].ptr = p;
		x.sst[nsst].rend = re;
		if(++nsst == x.nsst)
			break;

		l = g16(p);
		f = p[2]; p += 3;
		a = 0;
		if(f&8) {a = 4*g16(p); p += 2;}
		if(f&4) {a += g32(p); p += 4;}
		//		fmt_assert(p<re);
		for(;;) {
			int s = re - p;
			f &= 1;
			if(s >= l<<f)
				break;
			//			fmt_assert(!(s&f));
			//			fmt_assert(re < fe);
			l -= s>>f;
			if(re[0] != 0x3C) // CONTINUE
				BADF("String truncated");
			p = re + 4;
			re = p + g16(re+2);
			//			fmt_assert(re < fe);
			f = *p++;
		}
		p += l<<f;
		for(;;) {
			int s = re - p;
			if(s > a) break;
			a -= s;

			if(re[0] != 0x3C) // CONTINUE
				BADF("String truncated");
			p = re + 4;
			re = p + g16(re+2);
			//			fmt_assert(re < fe);
		}
		p += a;
	}
	return re;
}/*}}}*/
static const struct fmt default_fmt;
static const u8 *null_ptr;
static void xls_init_struc()
{/*{{{*/
	static u8 t[] = {0,0x10,0x12,0x10,0x12,0x10,0x10,0x12,0x12,0x12,0x14,
		0x22,0,0,0x30,0x30,0x30,0x30,0x40,0x40,0x40,0x40,0x50,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0x10,0x10,0x12,0x12,0x10,0x10,0x12,0x12,
		0x40,0x40,0x40,0x21};
	struct fmt *tab;
	int i;

	x.fmt.esize = sizeof default_fmt;
	x.fmt.nelem = 0;
	x.xf_ptr.esize = sizeof null_ptr;
	x.xf_ptr.nelem = 0;
	x.xf_fmt.esize = sizeof null_ptr;
	x.xf_fmt.nelem = 0;
	x.e1904 = 0;

	tab_alloc(&x.fmt, elemof(t)-1, &default_fmt);
	tab = x.fmt.tab;
	for (i=0; i < elemof(t); i++) {
		tab[i].type = t[i] >> 4;
		tab[i].arg = t[i] & 0xf;
	}
}/*}}}*/
static void getstr(u16 *d, u8 *p, int l)
{/*{{{*/
	int v = 0;
	if (x.biffv >= BIFF8) {
		v = *p++ & 1;
	}
	// XXX
	if (v) {
		while (--l>=0) {
			d[l] = g16(p+2*l);
		}
	} else {
		while (--l>=0) {
			d[l] = p[l];
		}
	}
	return;
}/*}}}*/
static void parse_fmt(struct fmt *f, u16 *p, int l)
{/*{{{*/
	u16 *e = p + l;
	u16 *q, *d;

	f->type = 0;
	f->arg = 0;

	if (e == p) {
		return;
	}
	q = p;
	while (*q=='[') {
		do {
			if (++q == e) {
				return;
			}
		} while(*q != ']');
		if (++q == e) {
			return;
		}
	}
	if (*p == 'Y' || *p == 'M' || *p == 'D' || *p == 'd' || *p == 'm') {
		f->type = 5;
		return;
	}
	if (*p == 'h') {
		f->type = 4;
		return;
	}

	p = q;
	d = 0;
	for (;;) {
		if (*q == '.') {
			d = q;
			break;
		}
		if (*q>=128 || !strchr("0#?, ", *q) || ++q==e) {
			break;
		}
	}
	if (!d) {
		if(p!=q && (q==e || *q!='/')) {
			//			f->arg = 0;
			f->type = 1;
		}
		return;
	}
	while (++q < e) {
		if (*q != '0' && *q != '#') {
			break;
		}
	}

	f->arg = q - d - 1;
	f->type = 1;
	return;
}/*}}}*/
static void set_fmt(u8 *p)
{/*{{{*/
	u8 *q;
	int n, l;
	struct fmt *fmt;
	u16 t[128];

	q = p+1;
	if (x.biffv >= BIFF4) {
		q += 2;
	}
	n = x.biffv < BIFF5 ? x.fmt.nelem : g16(p);
	l = q[-1];
	if (x.biffv >= BIFF8) {
		l = g16(p+2), q++;
	}

	if (l > elemof(t)) {
		return;
	}

	getstr(t, q, l);
	fmt = (struct fmt*)tab_alloc(&x.fmt, n, &default_fmt);
	parse_fmt(fmt, t, l);
	return;
}/*}}}*/
static const struct fmt* fmt_from_xf(int xf)
{/*{{{*/
	const struct fmt *fmt = &default_fmt;
	int n, st, ua, org_xf;
	u8 *p;

	if (xf >= x.xf_ptr.nelem) {
bad_xf:
		warnx("Strange XF index %u -- ignored", xf);
		return fmt;
	}

	org_xf = xf;

again:
	p = TAB(x.xf_ptr, u8*, xf);
	if (!p) {
		goto bad_xf;
	}

	if (x.biffv < BIFF5) {
		/* 0x02 */
		n = p[1];
		st = 2;
		ua = x.biffv < BIFF4 ? 3 : 5;
	} else {
		/* 0xE0 */
		n = g16(p+2);
		st = 4;
		ua = x.biffv < BIFF8 ? 7 : 9;
	}
	st = p[st];
	ua = p[ua];

	if (!((st ^ ua) & 4) && (st + ua != 1)) {
		/* format not present */
		if (!(st & 4) || xf!=org_xf) {
			/* not a style or loop */
			p += x.biffv!=BIFF4 ? 4 : 2;
			xf = g16(p) >> 4;
			if (xf!=org_xf && xf < x.xf_ptr.nelem) {
				goto again;
			}
		}
	} else if (n < x.fmt.nelem) {
		fmt = &TAB(x.fmt, struct fmt, n);
	}

	*(const struct fmt**)tab_alloc(&x.xf_fmt, org_xf, &null_ptr) = fmt;
	return fmt;
}/*}}}*/
static void print_time(int m, int f, double v,char * out);
static void print_fmt(unsigned xf, double v,char * out)
{
	const struct fmt *f;

	int nofmt=1;
	if (nofmt) {
		sprintf(out+strlen(out),"%f", v);
		return;
	}

	if (xf < x.xf_fmt.nelem) {
		f = TAB(x.xf_fmt, struct fmt*, xf);
		if (f) {
			goto have_fmt;
		}
	}
	f = fmt_from_xf(xf);
have_fmt:

	switch (f->type) {
		case 0:
			if (ceil(v) == v) {
				sprintf(out+strlen(out),"%.f", v);
				break;
			}
		default:
			sprintf(out+strlen(out),"%f", v);
			break;
		case 1:
			sprintf(out+strlen(out),"%.*f", f->arg, v);
			break;
		case 2:
			sprintf(out+strlen(out),"%.*E", f->arg, v);
			break;
		case 3:
		case 4:
		case 5:
			print_time(f->type-2, f->arg, v,out + strlen(out));
			break;
	}
	return;
}
static void print_time(int m, int f, double v,char *out)
{/*{{{*/
	int d;
	time_t t;
	struct tm *tm;

	d = v;
	v -= d;
	if (x.e1904) {
		d += 4*365;
	} else if (d <= 60) {
		d++;
	}
	d -= 25569;

	t = d*24*60*60 + (unsigned)(v*24*60*60);
	tm = gmtime(&t);
	if (!tm) {
		sprintf(out+strlen(out),"#BAD"); // XXX
		return;
	}
	if (m==3 && !f && !v) {
		m = 1;
	}
	if (m&1) {
		sprintf(out+strlen(out),"%04u-%02u-%02u",
				tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday);
		if (m==1) {
			return;
		}
		sprintf(out+strlen(out)," ");
	}
	sprintf(out+strlen(out),"%2u:%02u:%02u", tm->tm_hour, tm->tm_min, tm->tm_sec);
	return;
}/*}}}*/
static void print_rk(unsigned xf, u32 rk,char * out)
{/*{{{*/
	double v=0.0;
	if (rk & 2) {
		v = (s32)rk>>2;
	} else {
		v = ieee754((u64)(rk&~3) << 32);
	}
	if (rk & 1) {
		v /= 100;
	}
	print_fmt(xf, v,out + strlen(out));
	return;
}/*}}}*/
struct rr
{/*{{{*/
	int o, l, id;
};/*}}}*/
#define GETRR(P)			 \
	if (4 > x.map.len-rr.o) {	 \
		TRUNC;			 \
	}				 \
rr.l = g16(x.map.ptr+rr.o+2);	 \
rr.id = x.map.ptr[rr.o];	 \
rr.o += 4;			 \
if (rr.l > x.map.len-rr.o) {	 \
	TRUNC;			 \
}				 \
(P) = x.map.ptr + rr.o;		 \
rr.o += rr.l;

#define EXPLEN(L) if(rr.l < (L)) errx(1, "Record too short  &%d", __LINE__);
static int skip_substream(int o)
{/*{{{*/
	struct rr rr;
	int d = 1;
	rr.o = o;
	for (;;) {
		u8 *p, sv;
		GETRR(p)
			sv = p[-3];
		switch(rr.id) {
			case 0x09:
				if (sv<0x10) {
					d++;
				}
				break;
			case 0x0A:
				if (!sv && !--d) {
					return rr.o;
				}
		}
	}
	TRUNC;
}/*}}}*/
static int read_init_rr(int o)
{/*{{{*/
	struct rr rr;
	int sh, nr;
	u8 *p;

	xls_init_struc();
	rr.o = o;
	nr = g.nr; sh = 0;

	for (;;) {
		GETRR(p)

			switch(rr.id) {
				case 0x42: // CODEPAGE
					set_codepage(g16(p));
					break;
				case 0xFC: // SST
					rr.o = read_sst(p, x.map.ptr+rr.o, x.end) - x.map.ptr;
					break;
				case 0x1E: // FORMAT
					set_fmt(p);
					break;
				case 0x43:
				case 0xE0: // XF
					*(u8**)tab_alloc(&x.xf_ptr, x.xf_ptr.nelem, &null_ptr) = p;
					break;
				case 0x04: // LABEL
				case 0x03: // NUMBER
				case 0x06: // FORMULA
				case 0x07: // STRING
				case 0x7E: // RK
					return sh;
				case 0x09: // BOF
					if (p[-3]>=0x10) {
						break;
					}
					rr.o = skip_substream(rr.o);
					break;
				case 0x0A: // EOF
					if (p[-3]) {
						break;
					}
					return sh;
				case 0x85: // SHEET
					if(!nr--) {
						sh = p - 4 - x.map.ptr;
					}
					break;
				case 0x22: // DATEMODE
					x.e1904 = p[0];
					break;
			}
	}
}/*}}}*/
int to_cell(int r, int c,char *out)
{/*{{{*/
	if(r < g.top || r > g.bottom) {
		g.row = r;
		return 0;
	}
	if(g.row < g.top)
		g.row = g.top;
	if(g.row < r) {
		g.col = 0;
		do {
			sprintf(out+strlen(out),"\n");
			g.row++;
		} while(g.row < r);
	}
	if(c < g.left || c > g.right) {
		g.col = c;
		return 0;
	}
	if(g.col < g.left)
		g.col = g.left;
	while(g.col < c) {
		sprintf(out+strlen(out),"\t");
		g.col++;
	}
	return 1;
}/*}}}*/
static int to_cell_p(u8 *p,char *out) {return to_cell(g16(p), g16(p+2),out + strlen(out));}
static inline int to_nx_cell(char *out) {return to_cell(g.row, g.col+1,out+strlen(out));}
int print_sheet(int o, u8 *name, int nr,char * out)
{/*{{{*/
	struct rr rr;
	u8 pvrec;

	if(nr){
		sprintf(out+strlen(out),"\f");
	}
	if(name) print_str(name+1, *name,out+strlen(out));
	sprintf(out+strlen(out),"\n");

	rr.o = o;
	g.col = g.row = 0;
	pvrec = 0;

	for(;;) {
		u8 *p;

		GETRR(p)
			if (rr.id == 0x0A && !p[-3]) {
				// EOF
				break;
			}

		switch(rr.id) {
			case 0x09: // BOF
				if (p[-3]>=0x10) {
					break;
				}
				rr.o = skip_substream(rr.o);
				break;
			case 0x04: // LABEL
				if (to_cell_p(p,out+strlen(out))) {
					print_str(p+8, g32(p+6),out+strlen(out));
				}
				break;
			case 0xFD: // LABELSST
				if (to_cell_p(p,out+strlen(out))) {
					print_sst(g32(p+6),out+strlen(out));
				}
				break;
			case 0x7E: // RK
				if (to_cell_p(p,out+strlen(out))) {
					print_rk(g16(p+4), g32(p+6),out+strlen(out));
				}
				break;
			case 0xBD: { // MULRK
						   u8 *q = p + rr.l - 11;
						   int f = to_cell_p(p,out+strlen(out));
						   for(;;) {
							   unsigned xf = g16(p+4);
							   p += 6;
							   if (f) {
								   print_rk(xf, g32(p),out+strlen(out));
							   }
							   if (p>=q) {
								   break;
							   }
							   f = to_nx_cell(out+strlen(out));
						   }
					   } break;
			case 0x03: // NUMBER
					   if(!to_cell_p(p,out+strlen(out))) {
						   break;
					   }
number:
					   print_fmt(g16(p+4), ieee754(g64(p+6)),out + strlen(out));
					   break;
			case 0x06: // FORMULA
					   if(!to_cell_p(p,out+strlen(out))) {
						   pvrec = 0;
						   break;
					   }
					   if (g16(p+6+6) != 0xFFFF) {
						   pvrec = 0;
						   goto number;
					   }
					   break;
			case 0x07: // STRING
					   if (pvrec==0x06) {
						   print_str(p+2, g16(p),out+strlen(out));
					   }
					   break;
			case 0xD6: // RSTRING
					   if (to_cell_p(p,out+strlen(out))) {
						   print_str(p+8, g16(p+6),out+strlen(out));
					   }
					   break;
		}
		pvrec = rr.id;

		if(g.row > g.bottom) {
			break;
		}
	}
	out[strlen(out)]='\n';
	return strlen(out);
}/*}}}*/
void print_xls(char *out)
{/*{{{*/
	struct rr rr;
	int done;
	u8 *p;

	done = 0;
	rr.o = 0;
	GETRR(p)

		switch(g16(p+2)) {
			case 0x10: // single sheet
				if(g.nr) goto not_found;
				read_init_rr(rr.o);
				print_sheet(rr.o, 0, 0,out+strlen(out));
				return ;
			case 0x100: goto workbook;
			case 5: goto globals;
			default:
					BADF("Bad content");
		}

	/* BIFF5+ */
globals:
	rr.o = read_init_rr(rr.o);
	if(!rr.o)
		goto not_found;
	for(;;) {
		u32 o;
		GETRR(p)
			if(rr.id != 0x85) // SHEET
				break;
		o = rr.o;
		rr.o = g32(p);
		if(rr.o >= x.map.len)
			TRUNC;
		if(rr.o <= p-x.map.ptr)
			BADF( );
		if(p[4]==0) {
			u8 *q=NULL;
			GETRR(q)
				if(rr.id != 0x09) BADF( );
			print_sheet(rr.o, p+6, done++,out+strlen(out));
			printf("%s",q);
		}
		rr.o = o;
	}
	return ;

	/* BIFF4W */
workbook:
	for(;;) {
		GETRR(p)
			switch(rr.id) {
				u32 o;
				case 0x42: // CODEPAGE
				EXPLEN(2)
					set_codepage(g16(p));
				break;
				case 0x8E: // SHEETOFFSET
				EXPLEN(4)
					o = g32(p);
				if(o >= x.map.len) TRUNC;
				rr.o = o;
				goto found;
				case 0x0A: // EOF
				if(p[-3]) break;
				BADF( );
			}
	}
found:
	GETRR(p)
		if(rr.id != 0x8F) // SHEETHDR
			goto not_found;
	EXPLEN(5)
		for(;;) {
			u32 o = g32(p);
			if(o >= x.map.len-rr.o)
				TRUNC;
			o += rr.o;
			if(!g.nr--) {
				u8 *name = p+4;
				GETRR(p)
					if(rr.id != 0x09) // BOF
						BADF( )
							if(g16(p+2) == 0x10) {
								read_init_rr(rr.o);
								print_sheet(rr.o, name, done++,out+strlen(out));
							}
				g.nr = 0;
			}
			rr.o = o;
			GETRR(p)
				if(rr.id != 0x8F) {
					if(!done)
						goto not_found;
					break;
				}
		}
	return ;

not_found:
	errx(1, "No such sheet");
	return ;
}/*}}}*/
unsigned um_page_sz, um_page_sc;
static void um_sig(int n, siginfo_t *i, void *c);
static struct sigaction um_sa;
static LIST(maps);
static void um_init()
{/*{{{*/
	um_page_sz = getpagesize();
	um_page_sc = ffs(um_page_sz) - 1;

	um_sa.sa_sigaction = um_sig;
	um_sa.sa_flags = SA_SIGINFO|SA_RESETHAND;
}/*}}}*/
static void um_sig(int n, siginfo_t *i, void *c)
{/*{{{*/
	struct ummap *um;
	unsigned long o;

	{
		list_t *l;
		for(l=maps.next; l!=&maps; l=l->next) {
			um = list_item(l, struct ummap, list);
			o = (char*)i->si_addr - (char*)um->addr;
			if(o < um->size)
				goto found;
		}
	}
	return;

found:
	if(um->handler(um, (char*)um->addr + (o & -um_page_sz)) >= 0)
		sigaction(SIGSEGV, &um_sa, 0);
}/*}}}*/
int um_access_page(void *p)
{/*{{{*/
	return mprotect(p, um_page_sz, PROT_READ|PROT_WRITE);
}/*}}}*/
int um_map(struct ummap *um)
{/*{{{*/
	void *p;
	int v;

	if(!um_page_sz)
		um_init();

	p = mmap(0, um->size, PROT_NONE, MAP_PRIVATE|MAP_ANON, -1, 0);
	if(p==MAP_FAILED)
		return -1;
	um->addr = p;
	v = sigaction(SIGSEGV, &um_sa, 0);
	if(v>=0) list_add(&maps, &um->list);
	else munmap(p, um->size);
	return v;
}/*}}}*/
#ifndef __i386__
double ieee754(u64 v)
{/*{{{*/
	int s, e;
	double r;

	s = v>>52;
	v &= 0x000FFFFFFFFFFFFFull;
	e = s & 0x7FF;
	if(!e)
		goto denorm;
	if(e < 0x7FF) {
		v += 0x0010000000000000ull, e--;
denorm:
		r = ldexp(v, e - 0x3FF - 52 + 1);
	} else if(v) {
		r = NAN; s ^= 0x800;
	} else
		r = INFINITY;
	if(s & 0x800)
		r = -r;
	return r;
}/*}}}*/
#else
double ieee754(u64 v)
{/*{{{*/
	union {
		u64 v;
		double d;
	} u;
	u.v = v;
	return u.d;
}/*}}}*/
#endif
struct stream_kind
{/*{{{*/
	unsigned secsc;
	unsigned secsz;
	u32 maxsec;
	s32 (*sat_get)(struct stream_kind *sk, u32 n);
	u8 *(*sec_ptr)(struct stream_kind *sk, u32 n);
};/*}}}*/
struct stream
{/*{{{*/
	struct stream_kind *kind;
	s32 start;
	s32 c_sec;
	unsigned c_pos;
	u8 *c_ptr;
};/*}}}*/
struct ole
{/*{{{*/
	meml_t map;
	//char *name;

	s32 root;
	unsigned sec_tshld;
	struct stream ssat;
	struct stream container;

	s32 msat[109];
	s32 msat_start;

	struct stream_kind large_sec;
	struct stream_kind small_sec;
} ole;/*}}}*/
static s32 sat_get_lg(struct stream_kind *sk, u32 n);
static u8 *sec_ptr_lg(struct stream_kind *sk, u32 n);
static s32 sat_get_sm(struct stream_kind *sk, u32 n);
static u8 *sec_ptr_sm(struct stream_kind *sk, u32 n);
int ole_open(char *name,char *data ,int datalen)
{/*{{{*/
	u8 h[0x200];

	//ole.name = name;

	memcpy(h,data,sizeof h);
	ole.map.ptr = 0;

	if(memcmp(data, "\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1", 8))
		return 0;

	if(g16(h+0x1C) != 0xFFFE)
	{
		printf("Not LE file");
		return 0;
	}

	{
		u8 *s = h+76;
		int i;
		ole.msat_start = g32(h+68);
		//		ole.msat_size = g32(h+72);
		for(i=0; i<elemof(ole.msat); i++)
			ole.msat[i] = (s32)g32(s), s+=4;
	}

	meml_t m;
	m.ptr = (u8*)data;
	m.len = datalen;
	ole.map = m;
	ole.map.ptr += 512;

	{
		struct stream_kind *sk = &ole.large_sec;
		sk->secsc = g16(h+30);
		sk->secsz = 1<<sk->secsc;
		sk->maxsec = g32(h+44) << (ole.large_sec.secsc-2);
		sk->sat_get = sat_get_lg;
		sk->sec_ptr = sec_ptr_lg;
	}

	ole.sec_tshld = g32(h+56);
	{
		struct stream_kind *sk = &ole.small_sec;
		sk->secsc = g16(h+32);
		sk->secsz = 1<<sk->secsc;
		sk->maxsec = g32(h+64) << (ole.large_sec.secsc-2);
		sk->sat_get = sat_get_sm;
		sk->sec_ptr = sec_ptr_sm;
	}

	ole.ssat.start = g32(h+60);

	ole.root = g32(h+48);
	if(ole.root < 0)
	{
		//oleerr("There's no root stream");
		printf("There's no root stream");
	}

	return 1;
}/*}}}*/
static void str_open(struct stream *str, struct stream_kind *sk, s32 start)
{/*{{{*/
	str->start = start;
	str->c_sec = start;
	str->c_pos = 0;
	str->kind = sk;
	str->c_ptr = sk->sec_ptr(sk, start);
}/*}}}*/
#define SID_OK(K,N) ((u32)(N)<=(K)->maxsec)
#define SID_GET(P,I) ((s32)g32((s32*)(P)+(I)))
static s32 sat_get_lg(struct stream_kind *sk, u32 n)
{/*{{{*/
	unsigned m, maxsecidx;
	s32 b;

	maxsecidx = (1 << (sk->secsc-2)) - 1;
	m = n >> (sk->secsc-2); n &= maxsecidx;
	if(m < elemof(ole.msat))
		b = ole.msat[m];
	else {
		u8 *p;
		b = ole.msat_start;
		m -= elemof(ole.msat);
		for(;;) {
			if(!SID_OK(sk, b))
				return BADSEC;
			p = sk->sec_ptr(sk, b);
			if(m < maxsecidx)
				break;
			b = SID_GET(p, maxsecidx);
			m -= maxsecidx;
		}
		b = SID_GET(p, m);
	}
	if(SID_OK(sk, b)) {
		u8 *p = sk->sec_ptr(sk, b);
		return SID_GET(p, n);
	}
	return BADSEC;
}/*}}}*/
static int str_seek(struct stream *str, unsigned o);
static u8 *sec_ptr_lg(struct stream_kind *sk, u32 n)
{/*{{{*/
	return ole.map.ptr + (n<<sk->secsc);
}/*}}}*/
static s32 sat_get_sm(struct stream_kind *sk, u32 n)
{/*{{{*/
	int o = str_seek(&ole.ssat, 4*n);
	if(o<0) return BADSEC;
	return g32(ole.ssat.c_ptr + o);
}/*}}}*/
static u8 *sec_ptr_sm(struct stream_kind *sk, u32 n)
{/*{{{*/
	int o = str_seek(&ole.container, n<<sk->secsc);
	if(o<0){
		//oleerr("small sector not found");
		printf ("small sector not found");
	}
	return ole.container.c_ptr + o;
}/*}}}*/
static int str_seek(struct stream *str, unsigned o)
{/*{{{*/
	struct stream_kind *sk = str->kind;
	unsigned e = str->c_pos + sk->secsz;
	s32 b = str->c_sec;

	if(o < e) {
		if(o >= str->c_pos)
			goto ret;
		e = sk->secsz;
		b = str->start;
		if(o < e) goto found;
	}
	do {
		b = sk->sat_get(sk, b);
		if(!SID_OK(sk, b)) return -1;
		e += sk->secsz;
	} while(o >= e);

found:
	str->c_sec = b;
	str->c_pos = e - sk->secsz;
	str->c_ptr = sk->sec_ptr(sk, b);
ret:
	return o - str->c_pos;
}/*}}}*/
static void open_small_streams()
{/*{{{*/
	struct stream_kind *sk = &ole.large_sec;
	u8 *p = sec_ptr_lg(sk, ole.root);

	if(!SID_OK(sk, ole.ssat.start) ||
			!SID_OK(sk, g32(p+0x74))){
		//oleerr("Small sector storage empty");
		printf("Small sector storage empty");
	}

	str_open(&ole.container, &ole.large_sec, g32(p+0x74));
	str_open(&ole.ssat, &ole.large_sec, ole.ssat.start);
}/*}}}*/
static struct ummap wbk_um;
static struct stream wbk_str;
/* this is executed by the signal handler */
static int str_get_page(struct ummap *um, u8 *d)
{/*{{{*/
	struct stream_kind *sk = wbk_str.kind;
	int n, c, l;
	u8 *s;

	n = str_seek(&wbk_str, d - (u8*)um->addr);
	if(n<0) return n;

	sk = wbk_str.kind;
	c = sk->secsz - n;
	s = wbk_str.c_ptr + n;

	n = um_access_page(d);
	if(n<0) return n;

	l = um_page_sz - c;
	if(l <= 0) {
		memcpy(d, s, um_page_sz);
		return 0;
	}
	memcpy(d, s, c);
	d += c;

	for(;;) {
		s32 b = sk->sat_get(sk, wbk_str.c_sec);
		if(!SID_OK(sk, b)) return 0;
		s = sk->sec_ptr(sk, b);
		wbk_str.c_sec = b;
		wbk_str.c_pos += sk->secsz;
		wbk_str.c_ptr = s;

		if(l <= sk->secsz) break;
		l -= sk->secsz;
		memcpy(d, s, sk->secsz);
		d += sk->secsz;
	}
	memcpy(d, s, l);

	return 0;
}/*}}}*/
static u8 *find_slot(char *name)
{/*{{{*/
	struct stream_kind * const sk = &ole.large_sec;
	s32 b;
	u8 *p, *e;
	u16 l;

	b = ole.root;
	p = sk->sec_ptr(sk, b);
	l = 2*(strlen(name) + 1);
	e = p + sk->secsz;
	for(;;) {
		if(p[0x42]==2 && g16(p+0x40)==l) {
			unsigned i = 0;
			for(;; i++) {
				if(2*i >= l)
					return p; // found
				if(p[2*i] != (u8)name[i] || p[2*i+1])
					break;
			}
		}
		p += 0x80;
		if(p < e) continue;

		b = sk->sat_get(sk, b);
		if(!SID_OK(sk, b)) break;
		p = sk->sec_ptr(sk, b);
		e = p + sk->secsz;
	}
	return 0;
}/*}}}*/
meml_t get_workbook(char *data,int datalen)
{/*{{{*/
	struct stream_kind *sk;
	u32 len, sid;
	u8 *p;
	p = find_slot("Workbook");
	if(!p) {
		p = find_slot("Book");
		if(!p)
		{
			//oleerr("No Workbook found");
			printf("No Workbook found");
		}
	}

	sid = g32(p+0x74);
	len = g32(p+0x78);

	sk = &ole.large_sec;
	if(len < ole.sec_tshld) {
		if(!ole.container.c_ptr)
			open_small_streams();
		sk = &ole.small_sec;
	}

	if(!SID_OK(sk, sid))
	{
		//oleerr("Stream is empty");
		printf("Stream is empty");
	}

	str_open(&wbk_str, sk, sid);

	wbk_um.size = len;
	wbk_um.handler = (int(*)(struct ummap*,void*))str_get_page;

	um_map(&wbk_um);


	return (meml_t){wbk_um.addr, wbk_um.size};
}/*}}}*/
static u8 uni2cs[0x2E0-0xA0];
static u8 *cs = 0;
static char badchar = '?';
static u8 fallbacks[] = " "
" !cL\1Y|\4<\1-\6'\6>\3?AAAAAA\1CEEEEIIII\1NOOOOO\1OUUUUY\2aa"
"aaaa\1ceeeeiiii\1nooooo\1ouuuuy\1yAaAaAaCcCcCcCcDdDdEeEeEeEe"
"EeGgGgGgGgHhHhIiIiIiIiIi\2JjKk\1LlLlLlLlLlNnNnNnn\2OoOoOo\2R"
"rRrRrSsSsSsSsTtTtTtUuUuUuUuUuUuWwYyYZzZzZzsbBBb\3Cc\2Dd\4FfG"
"\3IKkl\2NnOOo\2Pp\5tTtTUu\1VYyZz\26AaIiOoUuUuUuUuUu\1AaAa\2G"
"gGgKkOoOo\2j\3Gg\2NnAa\2OoAaAaEeEeIiIiOoOoRrRrUuUuSsTt\2HhNd"
"\2ZzAaEeOoOoOoOoYy";
static u8 latin2[] = 
{160,/*{{{*/
	0xA0,3,0xA4,2,0xA7,0xA8,4,0xAD,2,0xB0,3,0xB4,3,0xB8,8,0xC1,0xC2,
	1,0xC4,2,0xC7,1,0xC9,1,0xCB,1,0xCD,0xCE,4,0xD3,0xD4,1,0xD6,0xD7,
	2,0xDA,1,0xDC,0xDD,1,0xDF,1,0xE1,0xE2,1,0xE4,2,0xE7,1,0xE9,1,
	0xEB,1,0xED,0xEE,4,0xF3,0xF4,1,0xF6,0xF7,2,0xFA,1,0xFC,0xFD,4,
	0xC3,0xE3,0xA1,0xB1,0xC6,0xE6,4,0xC8,0xE8,0xCF,0xEF,0xD0,0xF0,6,
	0xCA,0xEA,0xCC,0xEC,29,0xC5,0xE5,2,0xA5,0xB5,2,0xA3,0xB3,0xD1,
	0xF1,2,0xD2,0xF2,7,0xD5,0xF5,2,0xC0,0xE0,2,0xD8,0xF8,0xA6,0xB6,2,
	0xAA,0xBA,0xA9,0xB9,0xDE,0xFE,0xAB,0xBB,8,0xD9,0xF9,0xDB,0xFB,7,
	0xAC,0xBC,0xAF,0xBF,0xAE,0xBE,159,159,10,0xB7,16,0xA2,0xFF,1,
	0xB2,1,0xBD,0
};/*}}}*/
int find_charset(char *name)
{/*{{{*/
	const char names[] = "utf8asc\0iso1iso2";
	int l, p;
	l = strlen(name);
	if(l<3 || l>4) return -1;
	for(p=0; names[p]; p+=4)
		if(memcmp(names+p, name, 4)==0)
			return p>>2;
	return -1;
}/*}}}*/
static void expand(u8 *s)
{/*{{{*/
	u8 *d = uni2cs;
	u8 m = *s++;
	do {
		u8 c = *s++;
		if(c<m) d += c;
		else if(d>=endof(uni2cs)) break;
		else *d++ = c;
	} while(*s);
}/*}}}*/
void set_charset(int n)
{/*{{{*/
	cs = 0;
	if(n==0) // utf8
		return;

	//	memset(uni2cs, 0, sizeof uni2cs);
	expand(fallbacks);

	switch(n) {
		int u;
		case 1: // ascii
		break;
		case 2: // latin 1
		for(u=0x00A0; u<0x0100; u++)
			uni2cs[u-0xA0] = u;
		break;
		case 3: // latin 2
		expand(latin2);
		break;
	}
	cs = uni2cs;
}/*}}}*/
static void print_uni_char(u16 u,char * out)
{/*{{{*/
	unsigned v = u;
	if(v<0x00A0) {
		if(v<0x20 || v>=0x7F)
			v = v==10 ? ' ' : badchar;
	} else if(cs) {
		v -= 0xA0;
		if(v >= sizeof uni2cs || !(v = cs[v]))
			v = badchar;
	} else {
		v = v>>6 | 0xC0;
		if(u >= 0x800) {
			//putchar(u>>12 | 0xE0);
			out[strlen(out)] = (u>>12 | 0xE0);
			v = (v&077) | 0x80;
		}
		//putchar(v);
		out[strlen(out)] = v;
		v = (u&077) | 0x80;
	}
	//putchar(v);
	out[strlen(out)] = v;
	return ;
}/*}}}*/
u8 *print_uni(u8 *p, int l, u8 f,char * out)
{/*{{{*/
	if(f&1)
		while(--l >= 0) {
			print_uni_char(g16(p),out+strlen(out));
			p += 2;
		}
	else
		while(--l >= 0)
			print_uni_char(*p++,out+strlen(out));
	return p;
}/*}}}*/
static u16 *cp = 0;
static u16 cp1250[128] = 
{/*{{{*/
	0,0,0x201A,0,0x201E,0x2026,0x2020,0x2021,0,0x2030,0x0160,0x2039,
	0x015A,0x0164,0x017D,0x0179,0,0x2018,0x2019,0x201C,0x201D,0x2022,
	0x2013,0x2014,0,0x2122,0x0161,0x203A,0x015B,0x0165,0x017E,0x017A,
	0x00A0,0x02C7,0x02D8,0x0141,0x00A4,0x0104,0x00A6,0x00A7,0x00A8,
	0x00A9,0x015E,0x00AB,0x00AC,0x00AD,0x00AE,0x017B,0x00B0,0x00B1,
	0x02DB,0x0142,0x00B4,0x00B5,0x00B6,0x00B7,0x00B8,0x0105,0x015F,
	0x00BB,0x013D,0x02DD,0x013E,0x017C,0,0x00C1,0x00C2,0x0102,0x00C4,
	0x0139,0x0106,0x00C7,0x010C,0x00C9,0x0118,0x00CB,0x011A,0x00CD,
	0x00CE,0x010E,0x0110,0x0143,0x0147,0x00D3,0x00D4,0x0150,0x00D6,
	0x00D7,0x0158,0x016E,0x00DA,0x0170,0x00DC,0x00DD,0x0162,0x00DF,
	0x0155,0x00E1,0x00E2,0x0103,0x00E4,0x013A,0x0107,0x00E7,0x010D,
	0x00E9,0x0119,0x00EB,0x011B,0x00ED,0x00EE,0x010F,0x0111,0x0144,
	0x0148,0x00F3,0x00F4,0x0151,0x00F6,0x00F7,0x0159,0x016F,0x00FA,
	0x0171,0x00FC,0x00FD,0x0163,0x02D9,
};/*}}}*/
static u16 cp1252[128] = 
{/*{{{*/
	0x20AC,0x0081,0x201A,0x0192,0x201E,0x2026,0x2020,0x2021,0x02C6,
	0x2030,0x0160,0x2039,0x0152,0x008D,0x017D,0x008F,0x0090,0x2018,
	0x2019,0x201C,0x201D,0x2022,0x2013,0x2014,0x02DC,0x2122,0x0161,
	0x203A,0x0153,0x009D,0x017E,0x0178,0x00A0,0x00A1,0x00A2,0x00A3,
	0x00A4,0x00A5,0x00A6,0x00A7,0x00A8,0x00A9,0x00AA,0x00AB,0x00AC,
	0x00AD,0x00AE,0x00AF,0x00B0,0x00B1,0x00B2,0x00B3,0x00B4,0x00B5,
	0x00B6,0x00B7,0x00B8,0x00B9,0x00BA,0x00BB,0x00BC,0x00BD,0x00BE,
	0x00BF,0x00C0,0x00C1,0x00C2,0x00C3,0x00C4,0x00C5,0x00C6,0x00C7,
	0x00C8,0x00C9,0x00CA,0x00CB,0x00CC,0x00CD,0x00CE,0x00CF,0x00D0,
	0x00D1,0x00D2,0x00D3,0x00D4,0x00D5,0x00D6,0x00D7,0x00D8,0x00D9,
	0x00DA,0x00DB,0x00DC,0x00DD,0x00DE,0x00DF,0x00E0,0x00E1,0x00E2,
	0x00E3,0x00E4,0x00E5,0x00E6,0x00E7,0x00E8,0x00E9,0x00EA,0x00EB,
	0x00EC,0x00ED,0x00EE,0x00EF,0x00F0,0x00F1,0x00F2,0x00F3,0x00F4,
	0x00F5,0x00F6,0x00F7,0x00F8,0x00F9,0x00FA,0x00FB,0x00FC,0x00FD,
	0x00FE,0x00FF,
};/*}}}*/
static u16 cp1200[128]; // not initialized
void set_codepage(int n)
{/*{{{*/
	if(n==1200) {
		int i;
		for(i=0x80; i<=0xFF; i++) cp1200[i-0x80] = i;
		cp = cp1200;
	} else if(n==1250 || n==0x8001) cp = cp1250;
	else if(n==1252) cp = cp1252;
	else if(n!=0x16F) warnx("%d: Codepage not supported", n);
}/*}}}*/
u8 *print_cp_str(u8 *p, int l, char * out)
{/*{{{*/
	u8 *e = p + l;
	while(p<e) {
		u8 c = *p++;
		if(c<=0x7F) {
			if(c==0x7F) goto badchar;
			if(c<0x20) {
				if(c!=10) goto badchar;
				c=' ';
			}
		} else if(cp) {
			print_uni_char(cp[c-0x80],out+strlen(out));
			continue;
		} else{
badchar:
			c = badchar;
		}
		//putchar(c);
		out[strlen(out)] = c;
	}
	return p;
}/*}}}*/
int main(int argc, char *argv[])
{
	FILE * file = fopen("xls.xls","rb");
	int fileLen = fseek(file,0,SEEK_END);
	fileLen = ftell(file);
	rewind(file);
	char * data = malloc(fileLen);
	fread(data,1,fileLen,file);
	fclose(file);

	ole_open("xls.xls",data,fileLen);

	g.right = g.bottom = 0xFFFF;
	x.map = get_workbook(data,fileLen);
	x.end = x.map.ptr + x.map.len;
	check_biffv(x.map.ptr);

	char *out = malloc(fileLen);
	memset(out,0,fileLen);
	print_xls(out);
	printf("%s",out);

	return 0;
}

