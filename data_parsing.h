//
// Created by david-bol12 on 3/6/26.
//

#ifndef DEVICEDRIVERPROJECT_DATA_PARSING_H
#define DEVICEDRIVERPROJECT_DATA_PARSING_H

struct button_array {
    short no_pressed;
    char buttons[7];
};

struct point {
    unsigned short x;
    unsigned short y;
};

void get_buttons_pressed(unsigned char* data, unsigned int length, struct button_array* location);

struct point get_pen_coordinates(unsigned char* data, unsigned int length);

short get_pen_pressure(unsigned char* data, unsigned int length);

#endif //DEVICEDRIVERPROJECT_DATA_PARSING_H