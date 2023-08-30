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
Usage: marc [OPTION...] [FILE...]
Will read from stdin if no FILEs are provided.
Options:
  -f, --filter=FILTER     only print records adhering to FILTER
  -F, --format=STRING     output format; can be one of: n[one], h[uman], m[arc], x[ml] (default: "human")
  -l, --logfile=FILE      log to FILE (default: stderr)
  -L, --limit=INT         maximum number of records to process; -1 means process all available records (default: -1)
  -o, --output=FILE       output to FILE (default: "-")
  -V, --validate          log record validation statistics
  -v, --verbose           enable verbose logging
      --version           show version information and exit

Help options:
  -?, --help              Show this help message
      --usage             Display brief usage message
```

## Examples
```
  # validate records
  marc --validate foo.marc

  # print all records (human-readable)
  marc -Fhuman foo.marc

  # print out the full 245 field of all records
  marc -Fhuman --field 245 foo.marc

  # print out the 245a subfield of all records
  marc -Fhuman --field 245a foo.marc

  # print out the 245 field (subfields a, b, and c)
  marc -Fhuman --field 245abc foo.marc

  # dump the first ten records of one file into another
  marc -Fmarc --limit 10 -o bar.marc foo.marc
```

## About
In a previous life I worked for the office of research at a non-profit that
provided services to libraries. Some of what we did involved writing small,
efficient C programs that would troll through bibliographic records and tease
out interesting bits of information. I recently got an "itch" to revisit that,
so I wrote this little C library with some nice functions for processing MARC
records.

There is also a reference implementation viz the command-line tool `marc` with a
few useful functions described above (print, validate, etc.)

## References
* [MARC-21 documentation](http://www.loc.gov/marc/bibliographic/)
* [Library of Congress downloadable records](http://www.loc.gov/marc/bibliographic/)
