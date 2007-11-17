#include <utils/texteditor.h>
#include <memory/kmalloc.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <screen.h>
#include <keyboard.h>

#define MALLOC(x) kmalloc(x)
#define FREE(x) kfree(x)
#define REALLOC(x, y) krealloc((x),(y))

unsigned int displayw, displayh;
unsigned int cx, cy;

char message[20] = "editor running";

struct row_t
{
	char *buf;
	unsigned int len;
};

struct editfile_t
{
	FILE *stream;
	char *name;
	int writable;
	struct row_t *rows;
	fpos_t rowcount;
	int currentrow;
	int currentcol;
	int scrollpos;
} efile;

int row_get_reallength(struct row_t row)
{
	int reallength = 0, i, c;
	for(i=0; i<row.len-1; i++){
		c = row.buf[i];
		if(c=='\t'){
			reallength = (reallength + 8) & ~7;
		}
		else reallength++;
	}
	return reallength;
}

int row_get_height(struct row_t row)
{
	return row_get_reallength(row) / displayw + 1;
}

int editor_get_real_col()
{
	int realcol = 0, i;
	for(i=0; i<efile.currentcol; i++){
		if(efile.rows[efile.currentrow].buf[i] == '\t')
			realcol = (realcol + 8) & ~7;
		else
			realcol++;
	}
	return realcol;
}

void editor_print_statusline(void)
{
	set_colour(0x70);
	locate(0, 0);
	kprintf("[%s] [%d] [%d,%d(%d)] [rlen=%d rh=%d] [%s]",
			efile.writable?"writable":"read only", efile.scrollpos,
			efile.currentrow, efile.currentcol, editor_get_real_col(),
			efile.rows[efile.currentrow].len,
			row_get_height(efile.rows[efile.currentrow]),
			message);
	unsigned int cx, cy;
	getpos(&cx, &cy);
	while(cx++ < displayw) putch(' ');
	set_colour(0x07);
}

void editor_print_fileview(void)
{
	int i, y=1, rowx, realx, k, t;
	struct row_t *row;
	for(i=efile.scrollpos; i < efile.rowcount; i++){
		row = &efile.rows[i];
		rowx=0;
		for(;;){
			realx = 0;
			k = rowx;
			locate(0, y);
			while(realx < displayw){
				if(row->buf[rowx]==0) goto new_row;
				if(row->buf[rowx]=='\t'){
					t = realx;
					realx = (realx+8) & ~7;
					if(realx >= displayw) realx = displayw - 1;
					for(; t < realx; t++) putch(' ');
					rowx++;
				}
				else{
					putch(row->buf[rowx]);
					rowx++;
					realx++;
				}
			}
			y++;
			if(y >= displayh) goto outofscreen;
		}
new_row:
		while((realx++)<displayw) putch(' ');
		y++;
	}
	locate(0, y);
	set_colour(0x09);
	while(y < displayh){
		putch('~');
		rowx=1;
		while((rowx++)<displayw) putch(' ');
		y++;
	}
	set_colour(0x07);
outofscreen:;
}

void editor_set_cursor_pos_on_screen(void)
{
	cx = editor_get_real_col();
	int wraps = cx / displayw;
	cx -= wraps * displayw;
	int realrow = 1;
	int i;
	for(i=0; i<efile.currentrow; i++){
		realrow += row_get_height(efile.rows[i]);
	}
	cy = realrow - efile.scrollpos + wraps;
	locate(cx, cy);
}

int editor_write()
{
	int i;
	sprintf(message, "writing");
	editor_print_statusline();
	if(efile.stream) fclose(efile.stream);
	efile.stream = fopen(efile.name, "w");
	if(efile.stream==NULL){
		efile.writable = 0;
		sprintf(message, "can't open for w");
		return 1;
	}
	for(i=0; i<efile.rowcount; i++){
		fwrite(efile.rows[i].buf, 1, efile.rows[i].len-1, efile.stream);
		if(i!=efile.rowcount-1) fwrite("\n", 1, 1, efile.stream);
	}
	fclose(efile.stream);
	efile.stream = 0;
	sprintf(message, "wrote");
	return 0;
}

int editor_main(char *filename)
{
	int i, r;

	//otetaan ruudun koko talteen
	getdisplaysize(&displayw, &displayh);
	//kprintf("displayw=%d, displayh=%d\n", displayw, displayh);

	//tehdään jotain

	memset(&efile, 0, sizeof(efile));
	int len = strlen(filename);
	efile.name = MALLOC(len + 1);
	memcpy(efile.name, filename, len + 1);

	//avataan filu

	efile.stream = fopen(filename, "r+");
	if(efile.stream==NULL){
		efile.stream = fopen(filename, "r");
		if(efile.stream==NULL){
			kprintf("texteditor: can't open file\n");
			return 1;
		}
		efile.writable = 0;
	}
	else efile.writable = 1;

	//katsotaan filun koko

	fseek(efile.stream, 0, SEEK_END);
	fpos_t size;
	fgetpos(efile.stream, &size);
	if(size > 2000000000){
		kprintf("Can't handle that big file\n");
		goto quiteditor;
	}

	//luetaan filu

	fseek(efile.stream, 0, SEEK_SET);
	kprintf("reading...\n");
	char *buf = (char*)MALLOC(size);
	size = fread(buf, 1, size, efile.stream);
	kprintf("size=%d\n", size);
	efile.rowcount = 1;
	for(i=0; i<size; i++) if(buf[i]=='\n') efile.rowcount++;
	kprintf("rowcount=%d\n", efile.rowcount);
	efile.rows = (struct row_t*)MALLOC(sizeof(struct row_t)*efile.rowcount);
	memset(efile.rows, 0, sizeof(struct row_t)*efile.rowcount);
	i=0;
	for(r=0; r<efile.rowcount; r++){
		int rowlength = 0;
		while(i + rowlength < size){
			if(buf[i + rowlength] == '\n') break;
			rowlength++;
		}
		rowlength++;
		efile.rows[r].buf = (char*)MALLOC(rowlength);
		memcpy(efile.rows[r].buf, buf + i, rowlength);
		efile.rows[r].buf[rowlength-1] = 0;
		efile.rows[r].len = rowlength;
		//kprintf("row is \"%s\", len=%d\n", efile.rows[r].buf, efile.rows[r].len);
		i+= rowlength;
	}

	fclose(efile.stream);
	efile.stream = 0;

	//näyttöjuttuja

	for(i=0; i<displayh; i++) putch('\n');

	editor_print_statusline();
	editor_print_fileview();
	editor_set_cursor_pos_on_screen();

	//looppi

	for(;;){
		int ch = kb_get();
		//if(hex == 0) continue;
		//if(hex & 0x100) continue; //joku näppäin menee ylös
		//int ch = hex;//ktoasc(hex);

		if(ch == KEY_ESC){
			//kb_get();
			locate(0, 1);
			set_colour(0x70);
			print("[waiting command (q/w)]");
			set_colour(0x07);
			while((ch = /*ktoasc(*/kb_get()/*)*/)==0);
			kprintf("[%c]", ch);
			if(ch == 'q') goto quiteditor;
			else if(ch == 'w'){
				editor_write();
			}
			editor_print_fileview();
		}
		else if(ch == KEY_UP){
			key_up:
			if(efile.currentrow > 0){
				efile.currentrow--;
				if(efile.rows[efile.currentrow].len <= efile.currentcol)
					efile.currentcol = efile.rows[efile.currentrow].len - 1;
				if(efile.scrollpos > efile.currentrow){
					efile.scrollpos = efile.currentrow;
					editor_print_fileview();
				}
			}
		}
		else if(ch == KEY_DOWN){
			key_down:
			if(efile.currentrow < efile.rowcount - 1){
				efile.currentrow++;
				if(efile.rows[efile.currentrow].len <= efile.currentcol)
					efile.currentcol = efile.rows[efile.currentrow].len - 1;
				if(efile.scrollpos + (displayh - 2) < efile.currentrow){
					efile.scrollpos = efile.currentrow
							+ row_get_height(efile.rows[efile.currentrow])
							- 1 - (displayh - 2);
					editor_print_fileview();
				}
			}
		}
		else if(ch == KEY_PGUP){
			efile.currentrow -= displayh-1;
			if(efile.currentrow < 0) efile.currentrow = 0;
			efile.scrollpos -= displayh-1;
			if(efile.scrollpos < 0) efile.scrollpos = 0;
			editor_print_fileview();
		}
		else if(ch == KEY_PGDOWN){
			efile.currentrow += displayh-1;
			if(efile.currentrow > efile.rowcount - 1)
				efile.currentrow = efile.rowcount - 1;
			efile.scrollpos += displayh-1;
			if(efile.scrollpos + (displayh - 1) > efile.rowcount - 1)
				efile.scrollpos = efile.rowcount - 1 - (displayh - 1);
			if(efile.scrollpos < 0) efile.scrollpos = 0;
			editor_print_fileview();
		}
		else if(ch == KEY_LEFT){
			if(efile.currentcol > 0) efile.currentcol--;
			else if(efile.currentrow > 0){
				efile.currentcol = INT32_MAX;
				goto key_up;
			}
		}
		else if(ch == KEY_RIGHT){
			if(efile.currentcol < efile.rows[efile.currentrow].len-1)
				efile.currentcol++;
			else if(efile.currentrow < efile.rowcount - 1) {
				efile.currentcol = 0;
				goto key_down;
			}
		}
		else if(ch == KEY_HOME){
			efile.currentcol = 0;
		}
		else if(ch == KEY_END){
			efile.currentcol = efile.rows[efile.currentrow].len-1;
		}
		else if(ch == '\b' || ch == KEY_DEL){
			if(ch == KEY_DEL){
				efile.currentcol++;
				if(efile.currentcol >= efile.rows[efile.currentrow].len){
					if(efile.currentrow == efile.rowcount - 1) continue;
					efile.currentrow++;
					efile.currentcol = 0;
				}
			}
			if(efile.currentcol>0){
				efile.currentcol--;
				efile.rows[efile.currentrow].len--;
				for(i=efile.currentcol; i < efile.rows[efile.currentrow].len; i++){
					efile.rows[efile.currentrow].buf[i] = efile.rows[efile.currentrow].buf[i+1];
					if(efile.rows[efile.currentrow].buf[i] == 0) break;
				}
				efile.rows[efile.currentrow].buf = REALLOC(efile.rows[efile.currentrow].buf, efile.rows[efile.currentrow].len);

				editor_print_fileview();
			}
			else if(efile.currentrow>0){
				//uusi pituus on edellisen rivin pituus + tämän rivin pituus - 1
				//(toinen nollamerkki)
				//(tämä rivi laitetaan edellisen jatkoksi)
				int newlength = efile.rows[efile.currentrow-1].len
						+ efile.rows[efile.currentrow].len - 1;
				int oldlength = efile.rows[efile.currentrow-1].len;
				//varataan uudelleen tarpeeksi
				efile.rows[efile.currentrow-1].buf
						= REALLOC(efile.rows[efile.currentrow-1].buf,
						newlength);
				//kopioidaan jatko
				memcpy(efile.rows[efile.currentrow-1].buf
						+ efile.rows[efile.currentrow-1].len-1,
						efile.rows[efile.currentrow].buf, efile.rows[efile.currentrow].len);
				//pituudet ja nollamerkin varmistus
				efile.rows[efile.currentrow-1].len = newlength;
				efile.rows[efile.currentrow-1].buf[efile.rows[efile.currentrow-1].len-1] = 0;
				//vapautetaan tämä rivi
				FREE(efile.rows[efile.currentrow].buf);
				//siirretään alhaalla olevat rivit
				for(i=efile.currentrow; i<efile.rowcount-1; i++){
					efile.rows[i] = efile.rows[i+1];
				}
				//rivejä yksi vähemmän, reallokoidaan sen verran
				efile.rowcount--;
				efile.rows = (struct row_t*)REALLOC(efile.rows,
						efile.rowcount*sizeof(struct row_t));
				efile.currentrow--;
				efile.currentcol = oldlength - 1;

				editor_print_fileview();
			}
		}
		else if(ch == '\n'){
			efile.rowcount++;
			efile.rows = (struct row_t*)REALLOC(efile.rows,
					(efile.rowcount)*sizeof(struct row_t));
			//memset(&efile.rows[efile.rowcount-1], 0, sizeof(struct row_t));
			//vaihdetaan nykyisen rivin jälkeisten rivien bufferit aiemman rivin bufferiin.
			//nykyinen rivi ja sen jälkeinen rivi jäävät koskemattomiksi.
			for(i=efile.rowcount-1; i>efile.currentrow+1; i--){
				//kopioidaan row_t, eli bufferipointteri ja bufferin pituus
				efile.rows[i] = efile.rows[i-1];
			}
			//nykyisen rivin jälkeisen rivin pituus tulee olemaan tämän rivin lopun pituus
			efile.rows[efile.currentrow+1].len = efile.rows[efile.currentrow].len - efile.currentcol;
			//...tai ainakin 1
			if(efile.rows[efile.currentrow+1].len <= 0) efile.rows[efile.currentrow+1].len = 1;
			//varataan nykyisen rivin jälkeiselle riville sopiva tila
			efile.rows[efile.currentrow+1].buf = (char*)MALLOC(efile.rows[efile.currentrow+1].len);
			//kopioidaan nykyisen rivin loppupätkä siihen
			memcpy(efile.rows[efile.currentrow+1].buf, efile.rows[efile.currentrow].buf + efile.currentcol, efile.rows[efile.currentrow+1].len);
			//varmistetaan että sen lopussa on 0
			efile.rows[efile.currentrow+1].buf[efile.rows[efile.currentrow+1].len-1] = 0;
			//uudelleenvarataan nykyisen rivin pituudeksi kiva pituus
			efile.rows[efile.currentrow].len = efile.currentcol + 1;
			efile.rows[efile.currentrow].buf = REALLOC(efile.rows[efile.currentrow].buf, efile.rows[efile.currentrow].len);
			//nykyisen rivin viimeiseksi merkiksi 0
			efile.rows[efile.currentrow].buf[efile.rows[efile.currentrow].len-1] = 0;

			efile.currentcol = 0;
			efile.currentrow++;

			editor_print_fileview();
		}
		else if(ch >= ' ' || ch == '\t'){
			efile.rows[efile.currentrow].len++;
			efile.rows[efile.currentrow].buf = (char*)REALLOC(
					efile.rows[efile.currentrow].buf, efile.rows[efile.currentrow].len);
			for(i=efile.rows[efile.currentrow].len-1; i>efile.currentcol; i--){
				efile.rows[efile.currentrow].buf[i] = efile.rows[efile.currentrow].buf[i-1];
			}
			efile.rows[efile.currentrow].buf[efile.currentcol] = ch;
			efile.currentcol++;

			editor_print_fileview();
		}

		editor_print_statusline();
		editor_set_cursor_pos_on_screen();
	}

quiteditor:
	locate(displayw-1, displayh-1);
	print("\neditor quitting\n");
	/*for(;;){
		print("save? (y/n)");
		char ch = ktoasc(kb_get());
		print("\n");
		if(ch=='y'){
			editor_write();
			break;
		}
		else if(ch=='n'){
			print("ok. not saved.\n");
			break;
		}
		kb_get();
	}
freethings:*/
	if(efile.name) FREE(efile.name);
	if(efile.rows){
		for(i=0; i<efile.rowcount; i++){
			if(efile.rows[i].buf) FREE(efile.rows[i].buf);
		}
		FREE(efile.rows);
	}
	return 0;
}


