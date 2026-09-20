## Haiku OS "Open With" Tracker Add-on

#### Adds an "Open With" item to Tracker's Add-ons menu that lets you open the selected file with any app you've symlinked into your personal Open With folder.

On first use the add-on creates (if needed) and reads:

```
/boot/home/config/settings/testing/open with/
```

Symlink any app into that folder (native Haiku app or a ported GTK app) and
it will show up as a menu entry the next time you invoke "Open With" from a
file's context menu. Picking an entry launches that app with the selected
file(s) passed as command line arguments.

If the folder is empty, the add-on offers to open it in Tracker so you can
drop symlinks into it.

#### Tested on x86_64 and x86_gcc2

#### Build hpkg: ```make```
