#include "config.h"
#include "marc.h"

int main()
{
    marcfile in;
    marcrec *rec;

    int rc = marcfile_from_FILE(&in, stdin);
    if(rc) {
        fprintf(stderr, "error: could not open stdin?\n");
        exit(rc);
    }

    int count = 0, valid = 0;
    for (rec = marcrec_read(0, &in); rec; rec = marcrec_read(0, &in))
    {
        if (marcrec_validate(rec) == 0)
        {
            valid++;
        }
        count++;
        marcrec_free(rec);
    }
    marcfile_close(&in);
    fprintf(stdout, "%i/%i valid\n", valid, count);

    return count - valid;
}
