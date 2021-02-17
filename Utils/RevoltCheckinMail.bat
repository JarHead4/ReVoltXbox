@rem = '--*-Perl-*--
@echo off
perl.exe -S RevoltCheckinMail.bat %*
goto endofperl
@rem ';


$user      = $ENV{USERNAME};
$changenum = "";
$subject   = "";
$bWideDistro = 1;

#
# Process command-line arguments
#
while( $szArg = shift @ARGV )
{
    ProcessOption( $szArg );
}

sub PrintUsageInfo()
{
    print "USAGE:  RevoltCheckinMail [OPTIONS]\n";
    print "        -u USERNAME (uses USERNAME environment var by default)\n";
    print "        -c CHANGENUM (uses last recent checkin from USERNAME by default)\n";
    print "        -s \"SUBJECT GOES HERE\" (or will prompt for short subject-line desc)\n";
    print "        -w (disable wide distribution; send to 'revoltdv' instead of 'xcheckin')\n";
}

sub ProcessOption()
{
    if( $_[0] eq "" )  { return; }

   
    $szArg = $_[0];

    if(    $szArg =~ /^-u$/i )  # user name
    {
        $user = shift @ARGV;
    }
    elsif( $szArg =~ /^-c$/i )  # change number
    {
        $changenum = shift @ARGV;
    }
    elsif( $szArg =~ /^-s$/i )  # subject for email message
    {
        $subject = shift @ARGV;
    }
    elsif( $szArg =~ /^-w$/i )  # disable wide distribution
    {
        $bWideDistro = 0;
    }
    elsif( $szArg =~ /^-h$/i || $szArg =~ /^-\?/i )  # print help/usage info
    {
        PrintUsageInfo();
        exit(1);
    }
    else
    {
        PrintUsageInfo();
        die "ERROR: unrecognized option '$szArg'\n";
        exit(1);
    }
}

if( $user eq "" )
{
    PrintUsageInfo();
    die "ERROR: user not defined.  Use '-u USERNAME'.\n";
}
if( $changenum eq "" )
{
    # No changenum specified, so look for a recent checkin by this user
    open( RECENTCHANGES, "sd changes -m 200 |" );
    while( $line = <RECENTCHANGES> )
    {
        if( $line =~ /^Change (\d+) .*$user/i )
        {
            $changenum = $1;
            last;
        }
    }
    close RECENTCHANGES;
}
if( $changenum eq "" )
{
    PrintUsageInfo();
    die "ERROR: change number not specified.  Use '-c CHANGENUM'.\n";
}
if( $changenum ne int($changenum) )
{
    PrintUsageInfo();
    die "ERROR: invalid change number.  (Example usage: -c 99)\n";
}

$subject =~ s/^\s*//;  # remove leading whitespace
$subject =~ s/\s*$//;  # remove trailing whitespace
while( $subject eq "" )
{
    # Require a brief description for the subject line
    print "Please enter a brief description to be included in the subject line.\n";
    print "Or on the command line, use -s \"SUBJECT GOES HERE\" (note quotation marks).\n";
    $subject = <STDIN>;
    chomp $subject;
    $subject =~ s/^\s*//;  # remove leading whitespace
    $subject =~ s/\s*$//;  # remove trailing whitespace
}


#
# Get the description of this checkin
#

$description = "";
open( CHANGEINFO, "sd describe -s $changenum |" );
while( $line = <CHANGEINFO> )
{
    if( $line =~ /^Differences/ ) { last; }
    $description .= $line;
}
close CHANGEINFO;

if( $description eq "" )
{
    die "ERROR: command 'sd describe $changenum' failed.\n";
}


#
# Write mail file and send it
#
if( $bWideDistro ) {
    @RECIPIENT_LIST = ( "xcheckin" );
} else {
    @RECIPIENT_LIST = ( "revoltdv" );
}
$MailSubject = "Re-Volt checkin [$user, #$changenum]  $subject";

$szTempFile = $ENV{TEMP} . "\\RevoltCheckinMail.txt";
open( MAILFILE, ">$szTempFile" ) ||
    die "ERROR: Cannot open temp mail file: $!\n";

if(1)
{
    ## Version A: use 'sendmail'
    foreach $recipient (@RECIPIENT_LIST)
    {
        print MAILFILE "TO:$recipient\n";
    }
    print MAILFILE "SUBJECT:$MailSubject\n";
    print MAILFILE $description;
    close MAILFILE;

    $command = "sendmail < $szTempFile";
}
else
{
    ## Version B: use 'smartmail'
    print MAILFILE $description;
    close MAILFILE;

    $MailRecips = join( ";" , @RECIPIENT_LIST );
    $command =
       "smartmail SMTP"
      ." server:smarthost"
      ." from:#mail"
      ." \"to:$MailRecips\""
      ." \"subject:$MailSubject\""
      ." \"body:$szTempFile\"" ;
}

if( ! system($command) )
{
    print "Sent checkin mail: [$user, #$changenum] $subject\n";
}

unlink $szTempFile;




__END__
:endofperl
