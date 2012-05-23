#include "gl_print.h"
#include "gl.h"
#include "font_define.h"

static screen_info* scrn_info_arr;

static void gl_scroll( unsigned int l, screen_info* curr_si );
static void _gl_do_putchar( char c, screen_info* curr_si );
static void _gl_do_putchar_at( unsigned int x, unsigned int y, char c, screen_info* curr_si );

void gl_print_init( screen_info* ptr ) {
	scrn_info_arr = ptr;
}

static void gl_scroll( unsigned int l, screen_info* curr_si ) {
	char* to = curr_si->lines[0];
	char* from = curr_si->lines[1];
	int c_line = 1;
	int cpy = 0;
	//just clear the screan and make it go to the top.
	draw_rect(0,0,curr_si->w, curr_si->h, FONT_COLOR_BLACK);
	
	curr_si->curr_y = 0;
	
	for ( c_line = 1; c_line <= curr_si->y_max; c_line++ ) {
		to = curr_si->lines[c_line-1];
		
		//clear the lines
		for(cpy = 0; cpy < curr_si->x_max; cpy++)
			to[cpy] = '\0';
		from = curr_si->lines[c_line];
		//copy the lines
		for(cpy = 0; cpy < curr_si->x_max; cpy++) {
			to[cpy] = from[cpy];
		}
		to[cpy] = '\0';
		draw_string(to, 0, (c_line-1)*FONT_HEIGHT, FONT_COLOR);
	}
	//TODO: set cursor also mark dirty if we do that
}

//////////
// CHAR
//////////
void gl_putchar( char c ) {

	screen_info* curr_si;
	Pid pid = 0;
	Status s;

	s = get_pid( &pid );
	curr_si = ( get_screen_info( pid ) );
	_gl_do_putchar( c, curr_si );
}

void gl_putchar_s( char c, screen_info* curr_si ) {
	_gl_do_putchar( c, curr_si );
}


static void _gl_do_putchar( char c, screen_info* curr_si ) {
	
	/*
	** If we're off the bottom of the screen, scroll the window.
	*/
	if( curr_si->curr_y > curr_si->y_max ){
		gl_scroll( 1, curr_si );
		curr_si->curr_y = curr_si->y_max;
	}
	
	switch( c ) {
		case '\n':
			curr_si->curr_x = 0;
			curr_si->curr_y++;
			break;
		case '\r':
			curr_si->curr_x = 0;
			break;
		case '\t':
			curr_si->curr_x +=4;
			break;
		default:
			curr_si->lines[curr_si->curr_y][curr_si->curr_x] = c;
			do_draw_character(c, curr_si->curr_x*FONT_WIDTH, curr_si->curr_y*FONT_HEIGHT, FONT_COLOR, curr_si);
			curr_si->curr_x += 1;
			if( curr_si->curr_x > curr_si->x_max ){
				curr_si->curr_x = 0;
				curr_si->curr_y += 1;
			}
			break;
	}
	
	//TODO: set cursor
}

void gl_putchar_at( unsigned int x, unsigned int y, char c ) {
	screen_info* curr_si;
	Pid pid = 0;
	Status s;

	s = get_pid( &pid );
	curr_si = ( get_screen_info( pid ) );
	_gl_do_putchar_at( x, y, c, curr_si );
}

static void _gl_do_putchar_at( unsigned int x, unsigned int y, char c, screen_info* curr_si ) {
	/*
	** If x or y is too big or small, don't do any output.
	*/
	if( x <= curr_si->x_max && y <= curr_si->y_max ){
		curr_si->lines[y][x] = c;
		draw_character(c, x, y, FONT_COLOR);
	}
}

void gl_puts_at(  unsigned int x, unsigned int y, char *str ){
	unsigned int	ch;

	while( (ch = *str++) != '\0' ){
		gl_putchar_at( x, y, ch );
	}
}

void gl_puts( char *str ){
	unsigned int	ch;

	while( (ch = *str++) != '\0' ){
		gl_putchar( ch );
	}
}

void gl_puts_s( char *str, screen_info* curr_si ){
	unsigned int	c;

	while( (c = *str++) != '\0' ){
		gl_putchar_s( c, curr_si );
	}
}
