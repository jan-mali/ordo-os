#include <Core/TextConsole.hpp>
#include <Core/memory.hpp>
#include <default8x16_psfu.hpp>

TextConsole text_console;

// TODO: Maybe add a Spinlock somewhere here

void TextConsole::init(int fb_pitch, int fb_height, volatile uint32_t* fb) {
	columns = fb_pitch / 4 / 8;
	rows = fb_height / 16;

	pitch = fb_pitch;
	height = fb_height;

	if (columns > MAX_CONSOLE_WIDTH)
		columns = MAX_CONSOLE_WIDTH;

	if (rows > MAX_CONSOLE_HEIGHT)
		rows = MAX_CONSOLE_HEIGHT;

	set_cursor(0, 0);
	set_color(0xffffff, 0x000000);

	framebuffer_data = fb;

	initialized = true;
}

void TextConsole::putc(char c) {
	if (!initialized)
		return;

	if (c == '\r') {
		cursor_x = 0;
		return;
	}

	if (c == '\t') {
		for (int i = 0; i < 4; i++)
			putc(' ');
		return;
	}

	if (c == '\n') {
		cursor_x = 0;
		if (++cursor_y >= rows) {
			memmove(text, text + MAX_CONSOLE_WIDTH, (MAX_CONSOLE_HEIGHT - 1) * MAX_CONSOLE_WIDTH * sizeof(char));
			memmove(foreground, foreground + MAX_CONSOLE_WIDTH, (MAX_CONSOLE_HEIGHT - 1) * MAX_CONSOLE_WIDTH * sizeof(uint32_t));
			memmove(background, background + MAX_CONSOLE_WIDTH, (MAX_CONSOLE_HEIGHT - 1) * MAX_CONSOLE_WIDTH * sizeof(uint32_t));
			cursor_y = rows - 1;
			draw();
		}
		return;
	}

	text[cursor_y * MAX_CONSOLE_WIDTH + cursor_x] = c;
	foreground[cursor_y * MAX_CONSOLE_WIDTH + cursor_x] = current_fg_color;
	background[cursor_y * MAX_CONSOLE_WIDTH + cursor_x] = current_bg_color;

	draw(cursor_y, cursor_x);

	if (++cursor_x >= columns) {
		cursor_x = 0;
		if (++cursor_y >= rows) {
			memmove(text, text + MAX_CONSOLE_WIDTH, (MAX_CONSOLE_HEIGHT - 1) * MAX_CONSOLE_WIDTH * sizeof(char));
			memmove(foreground, foreground + MAX_CONSOLE_WIDTH, (MAX_CONSOLE_HEIGHT - 1) * MAX_CONSOLE_WIDTH * sizeof(uint32_t));
			memmove(background, background + MAX_CONSOLE_WIDTH, (MAX_CONSOLE_HEIGHT - 1) * MAX_CONSOLE_WIDTH * sizeof(uint32_t));
			cursor_y = rows - 1;
			draw();
		}
	}
}

void TextConsole::write(const char* str) {
	if (!str)
		return;

	while (*str) {
		putc(*str++);
	}
}

void TextConsole::set_cursor(int row, int column) {
	cursor_y = row;
	cursor_x = column;
}

void TextConsole::set_color(uint32_t fg, uint32_t bg) {
	current_fg_color = fg;
	current_bg_color = bg;
}

void TextConsole::draw() {
	if (!initialized)
		return;

	for (int row = 0; row < rows; row++) {
		for (int col = 0; col < columns; col++) {
			draw(row, col);
		}
	}
}

void TextConsole::draw(int row, int column) {
	if (!initialized)
		return;

	// We are assuming a 0x00RRGGBB format of the framebuffer
	const PSF2Header* font = reinterpret_cast<const PSF2Header*>(default8x16_psfu);

	char c = text[row * MAX_CONSOLE_WIDTH + column];
	uint32_t fg = foreground[row * MAX_CONSOLE_WIDTH + column];
	uint32_t bg = background[row * MAX_CONSOLE_WIDTH + column];
	const uint8_t* glyph = default8x16_psfu + font->headersize + c * font->bytesperglyph;
	for (int y = 0; y < 16; y++) {
		for (int x = 0; x < 8; x++) {
			volatile uint32_t* pixel = &framebuffer_data[(row * 16 + y) * (pitch / 4) + (column * 8 + x)];
			*pixel = (glyph[y] & (0b10000000 >> x)) > 0 ? fg : bg;
		}
	}
}
