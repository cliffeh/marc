#include <stdio.h>
#include "marc.h"

const char *specific_usage = "print marc records in XML format";

extern int __marc_main_limit;
extern int __marc_main_fieldspec_count;
extern const char **__marc_main_fieldspec;
extern int __marc_main_infile_count;
extern const char **__marc_main_infiles;

void print_result(FILE *out, marcrec *rec, int current_file, gzFile in)
{
    if (current_file == 0)
    {
        fprintf(out, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                     "<collection\n"
                     "  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
                     "  xsi:schemaLocation=\"http://www.loc.gov/MARC21/slim http://www.loc.gov/standards/marcxml/schema/MARC21slim.xsd\"\n"
                     "  xmlns=\"http://www.loc.gov/MARC21/slim\">\n");
    }
    int count = 0;
    while (marcrec_read(rec, in) != 0 && (__marc_main_limit - count) != 0)
    {
        count++;
        marcrec_xml(out, rec);
    }
    if(current_file + 1 == __marc_main_infile_count) {
        fprintf(out, "</collection>");
    }
}
