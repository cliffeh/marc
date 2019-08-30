* error checking/recovery
* debug/verbose flag
* `marc field` command for fetching subfields
* refactor such that each subcommand doesn't need to do its own thread/memmory management
* refactor such that not so much code is repeated in action_* (mutex locking/unlocking, etc.)
* unit testing
* check for "-" being specified multiple times
* multiple output files
* MARCXML output?