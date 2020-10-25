# marc

A library and a set of command-line tools for manipulating MARC records.

## Building
To build the marc library and command-line tool:
> `./configure && make`

## Testing
To run unit tests:
> `make check`

## Installing
Depending on your environment this may require elevated privileges:
> `make install`

## Running
```
usage: marc COMMAND [OPTIONS] [FILES]

COMMANDS:
  dump      dump records in marc format
  help      print a brief help message and exit
  leaders   print marc leaders
  print     print marc records/fields in a human-readable format
  validate  validate marc records
  xml       print marc records in XML format

OPTIONS:
  -h, --help         print a brief help message and exit
  -f, --field SPEC   only output fields adhering to SPEC (note: this flag
                     is only used by marc-print)
  -l, --limit N      limit processing to the first N records per file
                     (default: no limit)
  -o, --output FILE  output to FILE (default: stdout)
  -v, --verbose      turn on verbose logging
  -V, --version      output version and exit

Note: if no input files are provided this program will read from stdin
```

## Examples
```
  # validate records
  marc-validate foo.marc

  # print all records
  marc-print foo.marc

  # print out the full 245 field of all records
  marc-print --field 245 foo.marc

  # print out the 245a subfield of all records
  marc-print --field 245a foo.marc

  # print out the 245 field (subfields a and b)
  marc-print --field 245ab foo.marc

  # dump the first ten records of one file into another
  marc-dump --limit 10 -o bar.marc foo.marc
```

## About
In a previous life I worked for the office of research at a non-profit that
provided services to libraries. Some of what we did involved writing small,
efficient C programs that would troll through bibliographic records and tease
out interesting bits of information. I recently got an "itch" to revisit that,
so I wrote this little C library with some nice functions for processing MARC
records.

There ia also a reference implementation viz the command-line tool `marc` with a
few useful functions described above (print, validate, etc.)

## References
* [MARC-21 documentation](http://www.loc.gov/marc/bibliographic/)
* [Library of Congress downloadable records](http://www.loc.gov/marc/bibliographic/)
