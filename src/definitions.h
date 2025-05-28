#ifndef CENGINE_DEFINITIONS_H
#define CENGINE_DEFINITIONS_H

#define TICK_RATE 20

#define CONCAT(a, b) CONCAT_(a, b)
#define CONCAT_(a, b) a ## b

#define STR(a) #a
#define XSTR(a) STR(a)

#endif //CENGINE_DEFINITIONS_H
