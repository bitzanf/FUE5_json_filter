# JSON filter for Factorio in Unreal Engine 5 (FUE5)
[![](https://avatars.githubusercontent.com/u/132253253?s=32) **The Main Project**](https://github.com/FUE5BASE/FUE5)

Because Factorio bases get very large very quickly, and because you'll probably want to render your whole base, you will inherently end up selecting a very large area for the export. This might become a problem, since the world also contains a lot of trees, cliffs, biter nests and so on. It's no problem then for the resulting JSON to be over **100 MB**. From personal experience that really doesn't go well with FUE5's json importer and renderer...

This program tries to solve that problem by selectively removing some entries from the file (`fish`, `tree`, `biter` (and spawners), `spitter` (and spawners), `worm`, `cliff`, `rock`, `item-on-ground`).


## Usage
```
OPTIONS
  -h,--help         Print a help message and exit  
  --base <path>     FUE5 base directory
  --in <path>       input file
  --out <path>      output file

FILTERS
  -f,--fish         remove fish
  -t,--tree         remove trees
  -b,--biter        remove biters
  -s,--spitter      remove spitters
  -w,--worm         remove worms
  -c,--cliff        remove cliffs
  -r,--rock         remove rocks
  -i,--item         remove ground items
```

The filter options work as *OR* flags, so you can specify more of them, even as `-rib` (removes rocks, items and biters). You can either specify only the base directory of FUE5, and the file paths will be taken from the default `Content\MyStuff\JSON\exported-entities.json` both as input and output, or you can specify input and output separately. Note that these two ways are mutually exclusive.


## Technical details
I used [CLI11](https://github.com/CLIUtils/CLI11) for command line parsing and [rapidjson](https://github.com/Tencent/rapidjson/) for JSON parsing. The filtering is done by matching special filter strings with the entity name using `strstr`. The entire json is loaded at first, then all metadata except the entity list is copied into a new json object, and then every entity that doesn't match the filter gets copied into the entity array and this new object is written to disk. For a 100MB JSON file it uses roughly 250 MB of RAM and processing takes about 50 seconds (rapidjson supports a wide range of optimizations, but I threw this together in about 20 minutes, so not much got utilized).

*also i was lazy so there is next to none error checking - if* ***anything*** *goes wrong, it will crash*