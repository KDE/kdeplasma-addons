#!/usr/bin/perl

# Finding today (Mocha) date which is yesterday's KDE server date ...
my ($year, $month, $day) = (localtime(time))[5,4,3];
$month++;
$year += 1900;
$date = sprintf ("%04d-%02d-%02d", $year, $month, $day);

# Running from my computer
print "Running /home/sandros/tmp/load_commits.pl $date\n";
exec "/home/sandros/tmp/load_commits.pl $date";

# Running from Mocha Host
#print "Running /home/sandros/scripts/load_commits.pl $date\n";
#exec "/home/sandros/scripts/load_commits.pl $date";