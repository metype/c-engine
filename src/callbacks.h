#ifndef METYPE_CALLBACKS_H
#define METYPE_CALLBACKS_H

#define CALLBACK(name, ret, ...) ret (*name)(__VA_ARGS__)
#define CALLBACK_TYPE(name, ret, ...) typedef CALLBACK(name, ret, __VA_ARGS__)
#define CALLBACK_LIST(name, ret, ...) CALLBACK_TYPE(name, ret, __VA_ARGS__); name* name##s
#define ANON_CALLBACK(ret, ...) CALLBACK(,ret,__VA_ARGS__)

#endif //METYPE_CALLBACKS_H
