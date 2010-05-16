Events Runner
=============

This is Plasma Runner Plugin, which provides you ability to add new events or todos to your calendar from KRunner (Alt+F2). With this runner you may enter into KRunner something like following:

* `event Project deadline; 13.02.2010` - create new event with summary "Project deadline" for 13.02.2010;
* `todo Complete my work; tomorrow 12:30; Business` - create todo with given summary and deadline datetime with category "Business";
* `event Travel to mountains; from yesterday to 31.02.2010; Travel` - create event with given time range and category;
* `event Something; 13:30` - something at 13:30 today;
* `event Other event; yesterday`
* `event Meeting; today from 12:00 to 14:00`
* `todo Buy new phone; in 2 years`
* `todo Complete site design; in 2 days after 10:30`
* And others...

Besides of incident creation you may update existing incidences in some ways:

* `complete Buy new phone` - mark selected task as completed!
* `complete Fix localization issues; 30` - set task as 30%-completed;
* `comment Birthday party; It was cool!` - append some text to task description.

Features
--------

Now following date/time specifications are available:

* `today`
* `yesteday`
* `tomorrow`
* `now`
* Time format - `h:m`
* Date format - `d.M.yyyy`
* `in 2 minutes/hours [after ...]`
* `in 10 days/weeks/months/years [after ...]`
* `from ... to ...`

You can specify event categories as third argument like they are displayed in KOrganizer:

* `Business`
* `Education:Exam`
* `Travel, Holiday`

Installation from packages
--------------------------

If you are using Ubuntu Karmic you may install package *plasma-runner-events* from my PPA at https://launchpad.net/~alexey-noskov/+archive/kde

You can add this PPA to your system by adding `ppa:alexey-noskov/kde` to your system's Software Sources

This PPA can be added to your system manually by copying the lines below and adding them to your system's software sources.

    deb http://ppa.launchpad.net/alexey-noskov/kde/ubuntu karmic main 
    deb-src http://ppa.launchpad.net/alexey-noskov/kde/ubuntu karmic main 

Build instructions
------------------

    cd /where/your/runner/is/installed
    mkdir build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=$KDEDIRS .. 
    make 
    sudo make install

(your $KDEDIRS is where you install your KDE 4)

Restart krunner:

    kbuildsycoca4
    kquitapp krunner
    krunner

Alt-F2 to launch KRunner and in the runners list you will find events runner.

Copyright © 2010 Alexey Noskov, released under the GPLv3 license 
Idea by SebastianHRO, published at http://forum.kde.org/brainstorm.php#idea85167
