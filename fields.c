#include <ctype.h>
#include "fields.h"

int validate_fieldspec(fieldspec *fs)
{
    // null fieldspec is legit 
    if(!fs) return 0;

    if(fs->len < 3) return 1;
    if(!isdigit(fs->spec[0])) return 1;
    if(!isdigit(fs->spec[1])) return 1;
    if(!isdigit(fs->spec[2])) return 1;
    
    // looks legit
    return 0;
}