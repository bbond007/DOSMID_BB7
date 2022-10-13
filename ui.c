/*
 * User interface routines of DOSMid
 * Copyright (C) 2014-2016 Mateusz Viste
 */

#include <dos.h>     /* REGS */
#include <stdlib.h>  /* ultoa() */
#include <stdio.h>   /* sprintf() */
#include <string.h>  /* strlen() */

#include "mem.h" /* MEM_XMS */
#include "ui.h"  /* include self for control */
#include "version.h"
#include "BB7.h"

/* color scheme 0xBF00 (Background/Foreground/0/0): mono, color */
/*
const unsigned short COLOR_TUI[2]       = {0x0700u, 0x1700u};
const unsigned short COLOR_NOTES[2]     = {0x0700u, 0x1E00u};
const unsigned short COLOR_NOTES_HI[2]  = {0x0000u, 0x8000u}; // a bit mask for highlighten columns 
const unsigned short COLOR_TEXT[2]      = {0x0700u, 0x1700u};
const unsigned short COLOR_TEMPO[2]     = {0x0700u, 0x1300u};
const unsigned short COLOR_CHANS[2]     = {0x0700u, 0x1200u};
const unsigned short COLOR_CHANS_DIS[2] = {0x0000u, 0x1800u};
const unsigned short COLOR_PROGRESS1[2] = {0x7000u, 0x2000u}; // elapsed time
const unsigned short COLOR_PROGRESS2[2] = {0x0700u, 0x8000u}; // not yet elapsed
const unsigned short COLOR_ERRMSG[2]    = {0x7000u, 0x4700u}; */
typedef struct tColorPref
{
	unsigned short COLOR_TUI[2];
	unsigned short COLOR_NOTES[2];
	unsigned short COLOR_NOTES_HI[2];  // a bit mask for highlighten columns
	unsigned short COLOR_TEXT[2];
	unsigned short COLOR_TEMPO[2];
	unsigned short COLOR_CHANS[2];
	unsigned short COLOR_CHANS_DIS[2];
	unsigned short COLOR_PROGRESS1[2]; // a bit mask for highlighten columns 
	unsigned short COLOR_PROGRESS2[2]; // not yet elapsed 
    unsigned short COLOR_ERRMSG[2];
} ColorPref;

ColorPref cp = 
{
	{0x0700u, 0x8700u},
    {0x0700u, 0x1E00u},
	{0x0000u, 0x8000u}, // a bit mask for highlighten columns */
	{0x0700u, 0x8700u},
	{0x0700u, 0x8E00u},
	{0x0700u, 0x8F00u},
	{0x0000u, 0x8800u},
	{0x7000u, 0x2000u}, // a bit mask for highlighten columns */
	{0x0700u, 0x7000u}, // not yet elapsed 
	{0x7000u, 0x4700u},
};
/*
const unsigned short COLOR_TUI[2]       = {0x0700u, 0x8700u};
const unsigned short COLOR_NOTES[2]     = {0x0700u, 0x1E00u};
const unsigned short COLOR_NOTES_HI[2]  = {0x0000u, 0x8000u}; // a bit mask for highlighten columns 
const unsigned short COLOR_TEXT[2]      = {0x0700u, 0x8700u};
const unsigned short COLOR_TEMPO[2]     = {0x0700u, 0x8E00u};
const unsigned short COLOR_CHANS[2]     = {0x0700u, 0x8F00u};
const unsigned short COLOR_CHANS_DIS[2] = {0x0000u, 0x8800u};
const unsigned short COLOR_PROGRESS1[2] = {0x7000u, 0x2000u}; // a bit mask for highlighten columns 
const unsigned short COLOR_PROGRESS2[2] = {0x0700u, 0x7000u}; // not yet elapsed 
const unsigned short COLOR_ERRMSG[2]    = {0x7000u, 0x4700u};
*/
unsigned short far *screenptr = NULL;
static int oldmode = 0;
static int colorflag = 0;

extern unsigned long MEM_TOTALLOC; /* total allocated memory from MEM.C */
extern unsigned short MEM_MODE; /* memory mode (MEM_XMS or MEM_MALLOC) */

/* prints a character on screen, at position [x,y]. charbyte is a 16bit
   value where higher 8 bits contain the attributes (colors) and lower
   8bits contain the actual character to draw. */
static void ui_printchar(int y, int x, unsigned short charbyte) {
    screenptr[(y << 6) + (y << 4) + x] = charbyte;
}

static void ui_printstr(int y, int x, char *string, int staticlen, unsigned short attrib) {
    int xs;
    /* print out the string first */
    for (xs = 0; string[xs] != 0; xs++) ui_printchar(y, x++, string[xs] | attrib);
    /* if staticlen is provided, fill the rest with spaces */
    for (; xs < staticlen; xs++) ui_printchar(y, x++, ' ' | attrib);
}

static void color_init(char * exeName)
{
	char colorPrefsName[100];
	FILE * fpColorPref;
	char * ext;
	strcpy(colorPrefsName, exeName);
	ext = strchr(colorPrefsName, '.');
	if (ext != NULL)
	{
		//change extention to ".CPF"
		*(++ext) = 'C'; *(++ext) = 'P'; *(++ext) = 'F';	
		/* create default (C)olor(P)refs(F)ile 
		fpColorPref = fopen(colorPrefsName, "w");
		if (fpColorPref != NULL)
		{
			fwrite((void const *)&cp, sizeof(cp), 1, fpColorPref);
			fclose(fpColorPref);
		}
		else
		{
			ui_puterrmsg("ERROR writing...",  colorPrefsName);
			getkey();
		}
		*/
		fpColorPref = fopen(colorPrefsName, "r");
		if (fpColorPref != NULL)
		{
			fread((void *)&cp, sizeof(cp), 1, fpColorPref);
			fclose(fpColorPref);
		}
		else
		{
			ui_puterrmsg("ERROR reading...",  colorPrefsName);
			getkey();
		}
	}
}

void ui_init(char * exeName) {
    union REGS regs;
    /* remember the current mode */
    regs.h.ah = 0x0F; /* get current video mode */
    int86(0x10, &regs, &regs);
    oldmode = regs.h.al;
    /* set text mode 80x25 */
    regs.h.ah = 0x00;  /* set video mode */
    screenptr = MK_FP(0xB800, 0);
    colorflag = 0;
    switch (oldmode) {
    case 0: /* 40x25 BW */
    case 2: /* 80x25 BW */
        regs.h.al = 0x02;  /* 80x25 BW */
        break;
    case 7: /* 80x25 BW HGC */
        regs.h.al = 0x07;  /* 80x25 BW HGC */
        screenptr = MK_FP(0xB000, 0);
        break;
    default:
        colorflag = 1;
        regs.h.al = 0x03;  /* 80x25, 16 colors */
        break;
    }
    int86(0x10, &regs, &regs);
    /* disable blinking effect (enables the use of high-intensity backgrounds).
     * This doesn't change anything on DOSemu nor VBox, but DOSbox is unable to
     * display high intensity backgrounds otherwise. */
    regs.x.ax = 0x1003;  /* toggle intensity/blinking */
    regs.h.bl = 0;       /* enable intensive colors (1 would enable blinking) */
    regs.h.bh = 0;       /* to avoid problems on some adapters */
    int86(0x10, &regs, &regs);
    mouseInstalled();
	color_init(exeName);
}

void ui_close(void) {
    union REGS regs;
    regs.h.ah = 0x00;  /* set video mode */
    regs.h.al = oldmode;
    int86(0x10, &regs, &regs);
}

void ui_hidecursor(void) {
    union REGS regs;
    regs.h.ah = 0x01;
    regs.x.cx = 0x2000;
    int86(0x10, &regs, &regs);
}

/* outputs an error message onscreen (title can be NULL) */
void ui_puterrmsg(char *title, char *errmsg) {
    int x, y;
    int msglen, titlelen, maxlen;
    int xstart;
    msglen = strlen(errmsg);
    maxlen = msglen;
    if (title != NULL) {
        titlelen = strlen(title);
        if (titlelen > msglen) maxlen = titlelen;
    }
    xstart = 40 - (maxlen >> 1);
    /* draw a red 'box' first */
    for (y = 8; y < 13; y++) {
        for (x = maxlen + 3; x >= 0; x--) {
            ui_printchar(y, xstart + x - 2, ' ' | cp.COLOR_ERRMSG[colorflag]);
        }
    }
    /* print out the title (if any), and the actual error string */
    if (title != NULL) ui_printstr(8, 40 - (titlelen >> 1), title, titlelen, cp.COLOR_ERRMSG[colorflag]);
    ui_printstr(10, 40 - (msglen >> 1), errmsg, msglen, cp.COLOR_ERRMSG[colorflag]);
}

/* draws the UI screen */
void ui_draw(struct trackinfodata *trackinfo, unsigned short *refreshflags, unsigned short *refreshchans, char *devname, unsigned int mpuport, int volume) {
#include "gm.h"  /* GM instruments names */
    int x, y;
    /* draw ascii graphic frames, etc */
    if (*refreshflags & UI_REFRESH_TUI) {
        char tempstr[32];
        for (x = 0; x < 80; x++) {
            ui_printchar(0, x, 205 | cp.COLOR_TUI[colorflag]);
            ui_printchar(17, x, 205 | cp.COLOR_TUI[colorflag]);
            ui_printchar(24, x, 205 | cp.COLOR_TUI[colorflag]);
        }
        for (y = 1; y < 17; y++) ui_printchar(y, 15, 179 | cp.COLOR_TUI[colorflag]);
        for (y = 18; y < 24; y++) {
            ui_printchar(y, 0, 186 |cp.COLOR_TUI[colorflag]);
            ui_printchar(y, 79, 186 | cp.COLOR_TUI[colorflag]);
        }
        ui_printchar(0, 15, 209 | cp.COLOR_TUI[colorflag]);
        ui_printchar(17, 15, 207 | cp.COLOR_TUI[colorflag]);
        ui_printchar(17, 0, 201 | cp.COLOR_TUI[colorflag]);
        ui_printchar(17, 79, 187 | cp.COLOR_TUI[colorflag]);
        ui_printchar(24, 0, 200 | cp.COLOR_TUI[colorflag]);
        ui_printchar(24, 79, 188 | cp.COLOR_TUI[colorflag]);
        ui_printstr(24, 78 - strlen("[ DOSMid v" PVER " ]"), "[ DOSMid v" PVER " ]", -1, cp.COLOR_TUI[colorflag]);
        /* clear out the background on the 'messages' part of the screen */
        for (y = 18; y < 23; y++) {
            ui_printstr(y, 1, "", 78, cp.COLOR_TEXT[colorflag]);
        }
        /* print static strings */
        sprintf(tempstr, "%s port: %03Xh", devname, mpuport);
        ui_printstr(18, 79 - strlen(tempstr), tempstr, -1, cp.COLOR_TEMPO[colorflag]);
        ui_printstr(19, 67, "Volume:", 7, cp.COLOR_TEMPO[colorflag]);
        ui_printstr(20, 67, "Format:", 7, cp.COLOR_TEMPO[colorflag]);
        ui_printstr(21, 67, "Tracks:", 7, cp.COLOR_TEMPO[colorflag]);
        ui_printstr(22, 68, "Tempo:", 6, cp.COLOR_TEMPO[colorflag]);
        if (MEM_MODE == MEM_XMS) {
            ui_printstr(22, 50, "XMS", 4, cp.COLOR_TEMPO[colorflag]);
        } else {
            ui_printstr(22, 50, "MEM", 4, cp.COLOR_TEMPO[colorflag]);
        }
        ui_printstr(22, 54, "used:", 9, cp.COLOR_TEMPO[colorflag]);
    }
    /* print notes states on every channel */
    if (*refreshflags & UI_REFRESH_NOTES) {
        for (y = 0; y < 16; y++) {
            if ((*refreshchans & (1 << y)) == 0) continue; /* skip channels that haven't changed */
            for (x = 0; x < 64; x++) {
                int noteflag = 0;
                if (trackinfo->notestates[x << 1] & (1 << y)) noteflag = 2;
                if (trackinfo->notestates[1 + (x << 1)] & (1 << y)) noteflag |= 1;
                switch (noteflag) {
                case 0:
                    ui_printchar(1 + y, 16 + x, ' ' | cp.COLOR_NOTES[colorflag] | ((~x << 13) & cp.COLOR_NOTES_HI[colorflag]));
                    break;
                case 1:
                    ui_printchar(1 + y, 16 + x, 0xde | cp.COLOR_NOTES[colorflag] | ((~x << 13) & cp.COLOR_NOTES_HI[colorflag]));
                    break;
                case 2:
                    ui_printchar(1 + y, 16 + x, 0xdd | cp.COLOR_NOTES[colorflag] | ((~x << 13) & cp.COLOR_NOTES_HI[colorflag]));
                    break;
                case 3:
                    ui_printchar(1 + y, 16 + x, 0xdb | cp.COLOR_NOTES[colorflag] | ((~x << 13) & cp.COLOR_NOTES_HI[colorflag]));
                    break;
                }
            }
        }
        *refreshchans = 0;
    }
    /* filename and props (format, tracks) */
    if (*refreshflags & UI_REFRESH_FNAME) {
        char tempstr[4], *sptr;
        /* print filename (unless NULL - might happen early at playlist load) */
        if (trackinfo->filename != NULL) {
            ui_printstr(18, 50, trackinfo->filename, 12, cp.COLOR_TEMPO[colorflag]);
        } else {
            ui_printstr(18, 50, "", 12, cp.COLOR_TEMPO[colorflag]);
        }
        /* total allocated memory */
        sprintf(tempstr, "%luK", MEM_TOTALLOC >> 10);
        ui_printstr(22, 60, tempstr, 7, cp.COLOR_TEMPO[colorflag]);

        /* print format */
        switch ((trackinfo->fileformat << 1) | trackinfo->midiformat) {
        case FORMAT_MIDI << 1:
            sptr = "MID0";
            break;
        case (FORMAT_MIDI << 1) | 1:
            sptr = "MID1";
            break;
        case FORMAT_RMID << 1:
            sptr = "RMI0";
            break;
        case (FORMAT_RMID << 1) | 1:
            sptr = "RMI1";
            break;
        case FORMAT_MUS << 1:
            sptr = "MUS";
            break;
        default:
            sptr = "-";
            break;
        }
        ui_printstr(20, 75, sptr, 4, cp.COLOR_TEMPO[colorflag]);
        /* print number of tracks */
        utoa(trackinfo->trackscount, tempstr, 10);
        ui_printstr(21, 75, tempstr, 4, cp.COLOR_TEMPO[colorflag]);
    }
    /* tempo */
    if (*refreshflags & UI_REFRESH_TEMPO) {
        char tempstr[16];
        unsigned long miditempo;
        /* print tempo */
        if (trackinfo->tempo > 0) {
            miditempo = 60000000lu / trackinfo->tempo;
        } else {
            miditempo = 0;
        }
        ultoa(miditempo, tempstr, 10);
        /*strcat(tempstr, "bpm");*/
        ui_printstr(22, 75, tempstr, 4, cp.COLOR_TEMPO[colorflag]);
    }
    /* volume */
    if (*refreshflags & UI_REFRESH_VOLUME) {
        char tempstr[8];
        sprintf(tempstr, "%d%%", volume);
        ui_printstr(19, 75, tempstr, 4, cp.COLOR_TEMPO[colorflag]);
    }
    /* elapsed/total time */
    if (*refreshflags & UI_REFRESH_TIME) {
        char tempstr1[24];
        char tempstr2[16];
        unsigned long perc;
        unsigned int curcol;
        int rpos;
        if (trackinfo->totlen > 0) {
            perc = (trackinfo->elapsedsec * 100) / trackinfo->totlen;
        } else {
            perc = 0;
        }
        sprintf(tempstr1, " %lu:%02lu (%lu%%)     ", trackinfo->elapsedsec / 60, trackinfo->elapsedsec % 60, perc);
        rpos = 78 - sprintf(tempstr2, "%lu:%02lu ", trackinfo->totlen / 60, trackinfo->totlen % 60);
        /* draw the progress bar */
        if (trackinfo->totlen > 0) {
            perc = (trackinfo->elapsedsec * 78) / trackinfo->totlen;
        } else {
            perc = 0;
        }
        for (x = 0; x < 78; x++) {
            if (x < perc) {
                curcol = cp.COLOR_PROGRESS1[colorflag];
            } else {
                curcol = cp.COLOR_PROGRESS2[colorflag];
            }
            if (x < 15) {
                ui_printchar(23, 1 + x, tempstr1[x] | curcol);
            } else if (x >= rpos) {
                ui_printchar(23, 1 + x, tempstr2[x - rpos] | curcol);
            } else {
                ui_printchar(23, 1 + x, ' ' | curcol);
            }
        }
        /* if we have more title nodes than fits on screen, scroll them down now */
        if (trackinfo->titlescount > 5) *refreshflags |= UI_REFRESH_TITLECOPYR;
    }
    /* title and copyright notice */
    if (*refreshflags & UI_REFRESH_TITLECOPYR) {
        int scrolloffset = 0, i;
        if ((trackinfo->titlescount <= 5) || (trackinfo->elapsedsec < 8)) {
            /* simple case */
            for (i = 0; i < 5; i++) {
                ui_printstr(18 + i, 1, trackinfo->title[i], UI_TITLEMAXLEN, cp.COLOR_TEXT[colorflag]);
            }
        } else { /* else scroll down one line every 2s */
            scrolloffset = (trackinfo->elapsedsec >> 1) % (trackinfo->titlescount + 4);
            scrolloffset -= 4;
            for (i = 0; i < 5; i++) {
                if ((i + scrolloffset >= 0) && (i + scrolloffset < trackinfo->titlescount)) {
                    ui_printstr(18 + i, 1, trackinfo->title[i + scrolloffset], UI_TITLEMAXLEN, cp.COLOR_TEXT[colorflag]);
                } else {
                    ui_printstr(18 + i, 1, "", UI_TITLEMAXLEN, cp.COLOR_TEXT[colorflag]);
                }
            }
        }
    }
    /* programs (patches) names */
    if (*refreshflags & UI_REFRESH_PROGS) {
        unsigned int color;
        for (y = 0; y < 16; y++) {
            color = cp.COLOR_CHANS_DIS[colorflag];
            if (trackinfo->channelsusage & (1 << y)) color = cp.COLOR_CHANS[colorflag];
            if (y == 9) {
                ui_printstr(y + 1, 0, "Percussion", 15, color);
            } else {
                ui_printstr(y + 1, 0, gmset[trackinfo->chanprogs[y]], 15, color);
            }
        }
    }
    /* all refreshed now */
    *refreshflags = 0;
}


/* waits for a keypress and return it. Returns 0 for extended keystroke, then
   function must be called again to return scan code. */
int og_getkey(void) 
{
    union REGS regs;
    int res;
    regs.h.ah = 0x08;
    int86(0x21, &regs, &regs);
    res = regs.h.al;
    if (res == 0) 
	{ /* extended key - poll again */
        regs.h.ah = 0x08;
        int86(0x21, &regs, &regs);
        res = regs.h.al | 0x100;
    }
    return(res);
}


/* poll the keyboard, and return the next input key if any, or -1 */
int getkey_ifany(void)
{ 
	union REGS regs;
    regs.h.ah = 0x0B;
    int86(0x21, &regs, &regs);
    if (regs.h.al == 0xFF)
        return(og_getkey());
	else
		if (JoystickButton(JoystickAButton2) || rightPressed())
			return (0x1B);
        else
			return(-1);
}

/* waits for a keypress(and return it) or right mouse or joystickA button #2 as ESC */
int getkey(void)
{
    int res;
    do
    {
        res = getkey_ifany();
    } while(res == -1);
    return res;
}
