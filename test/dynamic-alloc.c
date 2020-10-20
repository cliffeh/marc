#include "marc.h"

int main()
{
    marcfile *in = marcfile_open("test/testfile.marc.gz", "r");

    int count = 0, valid = 0;
    for (marcrec *rec = marcrec_read(0, in); rec; rec = marcrec_read(0, in))
    {
        if (marcrec_validate(rec) == 0)
            valid++;
        count++;
    }
    fprintf(stdout, "%i/%i valid\n", valid, count);

    return count - valid;
}
