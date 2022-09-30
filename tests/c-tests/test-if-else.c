#include <assert.h>
#include <stdio.h>

int
main()
{
    int x = 2;
    int in1 = 1, in2 = 2, in3 = 3;
    int out;

    if (x) {
        out = in1 + in2;
    } else {
        out = in2 + in3;
    }

    assert(out == 3);

    return 0;
}
