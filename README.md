# C_OpeningHours

This reposiitory contains the code which allow you to manipulate [the opening hours format as described for Open Street Map](https://wiki.openstreetmap.org/wiki/Key:opening_hours), in a low-level C library.

### Installation:

Everything should be in the Makefile.

```
$ make
$ sudo make install
```


### Uninstall:

As simple as before:

```
$ sudo make uninstall
```

### Tests:

If you simply want to test the availability of some features, you can do:

```
$ make standalone
$ ./libopening-hours "9:00-19:00"

-------- SEPARATOR --------
  Separator: 1

-------- SELECTORS --------
  Anyway: 0

  WIDE_RANGE_SELECTOR -----
     Type: 0
     Years:      1900+
     Monthdays:  Jan - Dec
     Weeknums:   1 - 53

  SMALL_RANGE_SELECTOR ----
     Type: 0
     Weekdays:    Mo - Su
     Hours:       09:00 - 19:00

--------   STATE   --------
   That's open
```

## Notes

This project needs a huge amount of updates. Even if I can't update it for now, I won't give up the development of the project.

You're welcome to submit any issue or pull request.
