volatile int a;
struct Test {
    const int data, *pointer;
    volatile char a, b[10];
    struct Test *next;
};
void func(volatile int *p, const char y, struct Test *guest) {
    short const data, *pointer;
    struct Test use, *p_use;
}
volatile char x, *b;

////
volatile int a;
struct Test {
    const int data, *pointer;
    volatile char a, b[10];
    struct Test *next;
};
void func(volatile int *p, const char y, struct Test *guest);
volatile char x, *b;
void func(volatile int *p, const char y, struct Test *guest) {
    const int x, *point;
    volatile char abc[10];
}