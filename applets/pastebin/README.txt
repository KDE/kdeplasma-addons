<sebas> the goal is to make it beautiful, reacting nicely to user input and to notify when done
<sebas> we make the applet appearance a simple icon
<sebas> when hovering over it, and there's enough place, we put an overlay "Drop Text or Image" on top of it
<sebas> when we've too little space, we use a Tooltip
<sebas> When you drag something on top of it, we can also use some cool overlay for "drop to pastebin" or a nice graphic
<sebas> We keep the busy animation (maybe blend it with the icon in the background, again some overlay maybe
<sebas> And then done, we use knotify, instead of painting the text into the applet, then we change the icon to a check for some time so if you missed the notification, you can still see that it arrived (or errored)


TODO:
- main display should be icon: paintInterface does this (DONE)
- overlay for hovering (DONE)
- different texts / icons for overlay: idle, succeeded, error, busy (DONE)
- tooltips updated with status (see overlay) (DONE)
- notification on success / error
- middle click on it copies into Clipboard (DONE)
- why does a background show up?
- proper handling between text / applet's size, probably involves some QFontMetrics + setMinimumSize magic