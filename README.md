# TomPlayer
From https://sourceforge.net/projects/tomplayer/ . Migrated to Git. Project by srafin (Stephan Rafin).

A multimedia player for tomtom GPS.

Tested on my TomTom ONE v8. Later versions work but has a LOT of memory limitations, because it has 30 MB of RAM only.
Recommended version is v0.220. 

**Recommendations**:
* `-INTERNAL` release files: Place the contents inside `INTERNAL` TomTom memory.
* `-SDCARD` release files: Place the contents inside an external SD Card and plug it into your TomTom. It should boot automatically to TomPlayer, as long as the SD Card is plugged. 

* From sources (alternative download, more prone to make some mistake): If you pick some `-src` file, please place the contents inside `distrib` folder into your ROOT of the TomTom.
If you use internal memory (like me), please remove `ttsystem`, as it can replace original `ttsystem` from TomTom and leave your device unusable.

The patches folder contains an updated initial splash screen (optional), because I found unintuitive tapping the screen to enter the program at first. Despite of that, I've put a more clarifying subtitle to this initial splash screen.
