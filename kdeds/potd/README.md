# Picture Of The Day (POTD) Caching Daemon

The lock screen runs in a special mode where it cannot access the internet. So
when it trys to load a POTD wallpaper that isn't cached, it fails.

This daemon caches POTD before screen locking. The lock screen always loads POTD
from cache. Whenever the lock screen configuration changes, the daemon will read
the configuration and cache POTD if necessary.
