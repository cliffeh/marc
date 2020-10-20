#include "config.h"
#include "marc.h"

int main()
{
    marcfile *in = marcfile_from_FILE(stdin, "r");
    marcrec *rec;

    int count = 0, valid = 0;
    for (rec = marcrec_read(0, in); rec; rec = marcrec_read(0, in))
    {
        if (marcrec_validate(rec) == 0)
            valid++;
        count++;
        marcrec_free(rec);
    }
    fprintf(stdout, "%i/%i valid\n", valid, count);

    return count - valid;
}
