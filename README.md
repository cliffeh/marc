# marc

A library and a set of command-line tools for manipulating MARC records.

## Building
> `make`

## Running
```$ ./marc help
usage: marc ACTION [OPTIONS] | marc [-h|--help]

ACTIONS:
  help      print a brief help message and exit
  print     pretty-print marc records
  validate  validate marc records for correctness (default action)

OPTIONS:
  -h, --help         print a brief help message and exit
  -i, --input  FILE  read input from FILE; can be specified multiple times (default: stdin)
  -o, --output FILE  write output to FILE (default: stdout)
  -t, --threads NUM  run using NUM threads (default: 8)
  ```
## About
In a previous life I worked for the office of research at a non-profit that provided services to libraries. Some of what we did involved writing small, efficient C programs that would troll through bibliographic records and tease out interesting bits of information. I recently got an "itch" to revisit that, so I wrote this little C library with some nice functions for processing MARC records. There's also a command-line tool `marc` that exposes (at present) a validation function (more to come...see [TODO.md](TODO.md).)

## References
* [MARC-21 documentation](http://www.loc.gov/marc/bibliographic/)
* [Library of Congress downloadable compressed records](http://www.loc.gov/marc/bibliographic/)
