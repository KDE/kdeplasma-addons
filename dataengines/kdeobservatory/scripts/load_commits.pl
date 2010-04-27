#!/usr/bin/perl

use LWP::Simple;
use DBI;

if (@ARGV == 0)
{
    print "Usage: load_commits.pl <YYYY-MM-DD>\n";
    exit();
}

#$hostname = ""; # Running from Mocha Host
$hostname = ""; # Running from my computer
$database = "";
$user = "";
$password = "";

$driver = "mysql";
$dsn = "DBI:$driver:database=$database;host=$hostname";
$dbh = DBI->connect($dsn, $user, $password)
       or die "Couldn't connect to database: " . DBI->errstr;
$dbh->{AutoCommit} = 0;

# In some cases a duplicated entry error will be raised because of just appeared commits
# that make the last commit in one page pass to the first one in previous page. I use the
# REPLACE statement to prevent such errors. Anyway, commits won't be missed.
my $commits_handle = $dbh->prepare_cached('REPLACE INTO commits VALUES (?,?,?,?)')
                     or die "Couldn't create commit handle";

my $develselect_handle = $dbh->prepare_cached('SELECT COUNT(*) FROM developers WHERE svn_account = ?')
                         or die "Couldn't create develselect handle";

my $develinsert_handle = $dbh->prepare_cached('INSERT INTO developers VALUES (?,?,?,?)')
                         or die "Couldn't create develinsert handle";

my $develupdate_handle = $dbh->prepare_cached('UPDATE developers SET last_commit = IF (? > last_commit, ?, last_commit), first_commit = IF (? < first_commit, ?, first_commit) WHERE svn_account = ?')
                         or die "Couldn't create develupdate handle";

my $year  = substr $ARGV[0], 0, 4;
my $month = substr $ARGV[0], 5, 2;

my $r = 1;
my $commitCounter  = 0;
my $invalidCounter = 0;

my $foundData = 0;
my $finished  = 0;

while (!$finished)
{
    my $url = 'http://lists.kde.org/?l=kde-commits&r=' . $r . '&b=' . $year . $month . '&w=4';
    my $foundCommit = 0;

    print "Getting url $url\n";
    my $content = get $url;
    if (!defined $content)
    {
        print "Failed to get url. 2nd attempt !\n";
        $content = get $url;
    }
    if (!defined $content)
    {
        print "Failed to get url. 3rd attempt !\n";
        $content = get $url;
    }
    die "Couldn't get $url" unless defined $content;

    my @lines = split('\n', $content);

    foreach my $line (@lines)
    {
        if ($line =~ /^\s*(\d+)\.\s(\d\d\d\d-\d\d-\d\d)\s\s<a href="(.+?)">(.+?)<\/a>\s*<a href="\?l=kde-commits&w=4">kde-commits<\/a>\s\s(.+)/)
        {
            $foundCommit = 1;
            my ($commitNumber, $commitDate, $commitLink, $commitPath, $commitDeveloper)  = ($1, $2, $3, $4, $5);

            if ($commitDate eq $ARGV[0])
            {
                $foundData = 1;
                $commitDeveloper =~ s/\s+$//;

                my $commitContent = get "http://lists.kde.org/" . $commitLink;
                if (!defined $commitContent)
                {
                    print "Failed to get content url. 2nd attempt !\n";
                    $commitContent = get "http://lists.kde.org/" . $commitLink;
                }
                if (!defined $commitContent)
                {
                    print "Failed to get content url. 3rd attempt !\n";
                    $commitContent = get "http://lists.kde.org/" . $commitLink;
                }
                die "Couldn't get content $url" unless defined $commitContent;

                my @contentLines = split('\n', $commitContent);
                
                my $commitDateTime = "";
                my $commitSVNAccount = "";
                my $commitLog = "";
                my $enteredLogSection = 0;
                foreach my $contentLine (@contentLines)
                {
                    if ($contentLine =~ /Date:\s{7}<a href="\?l=kde-commits&r=\d+&w=4&b=\d{6}">(\d\d\d\d)-(\d\d)-(\d\d)\s(\d{1,2}):(\d\d):(\d\d)<\/a>/)
                    {
                        my ($year, $month, $day, $hour, $min, $sec) = ($1, $2, $3, $4, $5, $6);
                        $commitDateTime = sprintf ("%04d-%02d-%02d %02d:%02d:%02d", $year, $month, $day, $hour, $min, $sec);
                    }
                    elsif ($contentLine =~ /SVN commit \d+ by (.+):/)
                    {
                        $commitSVNAccount = $1;
                        $enteredLogSection = 1;
                    }
                    elsif ($contentLine =~ /^\s_?[AMDUGC]\s*(\+\d+\s*\-\d+)?\s*\S+/)
                    {
                        $enteredLogSection = 0;
                        last;
                    }
                    elsif ($enteredLogSection)
                    {
                        if ($commitLog ne "" and $contentLine ne "")
                        {
                            $commitLog = $commitLog . "\n";
                        }
                        $commitLog = $commitLog . $contentLine;
                    }
                }

                print "\"$commitNumber\"\n";
                #print "\"$commitNumber\"\n\"$commitDateTime\"\n\"$commitLink\"\n\"$commitPath\"\n\"$commitDeveloper\"\n\"$commitSVNAccount\"\n\"$commitLog\"\n\n";
                if ($commitSVNAccount ne "")
                {
                    $commitCounter++;

                    $develselect_handle->execute($commitSVNAccount)
                    or die "Couldn't select developer: " . $dbh->errstr;

                    if ($develselect_handle->fetchrow() eq 0)
                    {
                        print "Devel nao existe, inserindo\n";
                        $develinsert_handle->execute($commitSVNAccount, $commitDeveloper, $commitDateTime, $commitDateTime)
                        or die "Couldn't insert developer: " . $dbh->errstr;
                    }
                    else
                    {
                        print "Devel ja existe, atualizando\n";
                        $develupdate_handle->execute($commitDateTime, $commitDateTime, $commitDateTime, $commitDateTime, $commitSVNAccount)
                        or die "Couldn't update developer: " . $dbh->errstr;
                    }

                    $commits_handle->execute($commitDateTime, $commitSVNAccount, $commitPath, $commitLog)
                    or die "Couldn't insert commit: " . $dbh->errstr;
                }
                else
                {
                    $invalidCounter++;
                }
            }
            elsif ($foundData)
            {
                $finished = 1;
                last;
            }
        } 
    }
    if (!$foundCommit)
    {
        $finished = 1;
    }
    $r++;
}
$dbh->commit
or die "Couldn't commit: " . $dbh->errstr;

print "$commitCounter commits inserted\n";
print "$invalidCounter invalid commits\n";
