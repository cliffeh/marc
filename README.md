# marc

A library and a set of command-line tools for manipulating MARC records.

## Building
> `make`

## Running
```usage: marc ACTION [OPTIONS] [FILES] | marc [-h|--help]

ACTIONS:
  help      print a brief help message and exit
  print     print marc records in a human-readable format
  validate  validate marc records for correctness (default action)

OPTIONS:
  -h, --help         print a brief help message and exit
  -f, --field SPEC   only output fields according to SPEC
  -o, --output FILE  write output to FILE (default: stdout)
  -t, --threads NUM  run using NUM threads (default: 8)
Examples:

  # validate records
  marc validate foo.marc

  # print all records
  marc print foo.marc

  # print out the full 245 field of all records
  marc print --field 245 foo.marc

  # print out the 245a subfield of all records
  marc print --field 245a foo.marc

  # print out the 245 field (subfields a and b, space-delimited)
  marc print --field 245ab foo.marc
  ```
## About
In a previous life I worked for the office of research at a non-profit that provided services to libraries. Some of what we did involved writing small, efficient C programs that would troll through bibliographic records and tease out interesting bits of information. I recently got an "itch" to revisit that, so I wrote this little C library with some nice functions for processing MARC records. There's also a command-line tool `marc` that exposes a couple of functions (print, validate...more to come...see [TODO.md](TODO.md).)

## References
* [MARC-21 documentation](http://www.loc.gov/marc/bibliographic/)
* [Library of Congress downloadable compressed records](http://www.loc.gov/marc/bibliographic/)
