

#include "bootpack.h"

#define SHEET_USE		1

struct SHTCTL *shtctl_init(struct MEMMAN * memman,unsigned char * vram,int xsize,int ysize)
{
	struct SHTCTL *ctl;
	int i;
	ctl = (struct SHTCTL *)memman_alloc_4k(memman, sizeof (struct SHTCTL));
	if( ctl ==0){
		goto err;
		}
	ctl->vram = vram;
	ctl->xsize = xsize;
	ctl->ysize = ysize;
	ctl->top = -1;			/* default no sheet */
	for(i = 0 ;i < MAX_SHEETS;i++){
		ctl->sheets0[i].flags=0;		/* unused */
		}
err:
	return ctl;		
}

struct SHEET *sheet_alloc(struct SHTCTL *ctl)
{
	struct SHEET *sht;
	int i;
	for(i = 0; i < MAX_SHEETS; i++){
		if(ctl->sheets0[i].flags == 0){
			sht = &ctl->sheets0[i];
			sht->flags = SHEET_USE; 
			sht->height = -1;   /* hide */
			return sht;
			}
		}
	return 0;		/* all sheets have been used */
}

void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv)
{
	sht->buf = buf;
	sht->bxsize = xsize;
	sht->bysize = ysize;
	sht->col_inv = col_inv;
	return;
}


void sheet_updown(struct SHTCTL * ctl, struct SHEET * sht, int height)
{
	int h, old = sht->height;  /*save the height before set in  old*/

	/* if height > top or < -1, refine */
	if(height > ctl->top + 1){
		height = ctl->top + 1;
		}
	if(height < -1){
		height = -1;
		}
	sht->height = height;  /* set height */

/* rearrangment */
	if(old > height){     /* lower than before */
		if(height >= 0){
			for(h =old; h> height;h--){
				/* put up in the middle of the raise */
				ctl->sheets[h] = ctl->sheets[h - 1];
				ctl->sheets[h]->height = h;
				}
			ctl->sheets[height] = sht;
			}else {   /* hide */
			if(ctl->top > old){ 
				/* put the top down */
				for(h = old ; h < ctl->top; h++){
					ctl->sheets[h] = ctl->sheets[h + 1];
					ctl->sheets[h]->height = h;
					}
				}
			ctl->top--;	/*because the sheets to display - 1*/
				}
			sheet_refresh( ctl );  /* Redraw the sheets */
		}else if (old < height){ /* lower than before */
		if(old >= 0) {
			/*put down in the middle of the raise */
			for(h = old; h < height ; h++){
				ctl->sheets[h] = ctl->sheets[h + 1];
				ctl->sheets[h] ->height = h;
				}
			ctl->sheets[height] = sht;
			}else {       /* change hide to display */
			/* put the top up */
			for(h =ctl->top; h >= height; h--){
				ctl->sheets[h + 1] = ctl->sheets[h];
				ctl->sheets[h + 1] ->height = h + 1;
				}
			ctl->sheets[height] = sht;
			ctl->top++;
				}
				sheet_refresh( ctl );
		}
		return;
}

void sheet_refresh(struct SHTCTL * ctl)
{
	int h, bx, by, vx, vy;
	unsigned char *buf, c, *vram = ctl->vram;
	struct SHEET * sht;
	for(h = 0; h <= ctl->top; h++){
		sht = ctl->sheets[h];
		buf = sht->buf;
		for( by = 0; by < sht->bysize; by++){
			vy = sht->vy0 + by;
			for (bx = 0; bx < sht->bxsize; bx++) {
				vx = sht->vx0 + bx;
				c = buf[by * sht->bxsize + bx];
				if (c != sht->col_inv) {
					vram[vy * ctl->xsize + vx] = c;
					}
				}
			}
		}
	return ;
}

void sheet_slide(struct SHTCTL * ctl,struct SHEET * sht,int vx0,int vy0)
{
	sht->vx0 = vx0;
	sht->vy0 = vy0;
	if ( sht-> height >= 0){
		sheet_refresh(ctl);
		}
	return;
}

void sheet_free(struct SHTCTL * ctl,struct SHEET * sht)
{
	if( sht->height >=0){
		sheet_updown(ctl, sht ,-1);
		}
	sht->flags = 0;
	return;
}