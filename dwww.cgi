#!/usr/bin/perl -T
# $Id: dwww.cgi,v 1.11 2002/04/25 06:41:55 robert Exp $
#

$doc2html = '/usr/sbin/dwww-convert'; # Document-to-HTML converter
$search2html = '/usr/sbin/dwww-find'; # Search and output results as HTML
$ENV{PATH} = '/bin:/usr/bin:/usr/sbin';

$TRUE  = 1;
$FALSE = 0;

ReadParse();			# Get CGI info (see below for function)

#
# Figure out if this is a multiple word search or a straight request.
# If the former, split it on commas (,).
#
if ($in{'search'} ne '') {
    $search = $TRUE;
    @searchstring = split(/,/, $in{'search'});
} else {
    $search = $FALSE;
    $type     = $in{'type'};	 # This document is formated in $type.
    $location = $in{'location'}; # It is located at $location.
}

#
# Ok, now that we know the type, we need to perform the search or
# send them the requested document.  
#

if ($search == $TRUE) {
    open(CONVERSION,"$search2html @searchstring|")
	or error("Couldn't search for @searchstring! ($!)");

    PrintHeader();		# Print out our "This is HTML!" statement.
    while(<CONVERSION>) {
	print $_;		# just print the output straight...
    }
} elsif (($type eq "") or ($location eq "")) {

   print "Location: http://$ENV{'SERVER_NAME'}/dwww/\n\n";

} else {


    # Call doc2html, telling it that the format is of type $type, and the
    # requested document at $location. doc2html prints results to STDOUT,
    # and we pick it up and send it straight to the web client. doc2html
    # might just want to spit out HTMLized man pages straight, and send
    # out TOCs for big documents.
    open(CONVERSION,"$doc2html \'$type\' \'$location\'|")
	or error("Couldn't convert document $location! ($!)");

#   dwww-convert will return MIME header
#    PrintHeader();		# Print out our "This is HTML!" statement.
 
    while(<CONVERSION>) {
	print $_;		# just print the output straight...
    }
}

###############################################################################
# lookup() -- do keyword search and return the doc name, type, and location.
#	Output should be of the format (minus quotes):
#		"Name of the document |  Type of data | Location/on/drive"
###############################################################################
#sub lookup {
#    local(@keywords) = @_;
#    local(@matchs);
#    local($results, $string);
#    
#    # If we only have one keyword, it likely means a single man page or
#    # something, what do we do?  Right now, nothing, but this will probably
#    # change.
#
#    # We cheat for now.  We need to decide on a db format...
#    $string = join('|', @keywords);
#    open(SEARCH, "egrep $string /tmp/database|")
#	or error("egrep $string /tmp/database failed: ($!)");
#    while(<SEARCH>) {
#	s/^\s*|\s*$|\n$//g;	# remove extra whitespace and newlines.
#	push(@matchs, $_);
#    }
#    close(SEARCH);		# all done searching.
#    
#    return(@matchs);
#}

###############################################################################
# cleanup() -- HTMLize a list of doc names, types, and locations.
#	Format of input expected: "Name | Type | Location"
###############################################################################
#sub cleanup {
#    local(@input) = @_;
#    local(%links);		# our hashed array for urls.
#    local($line, $link, $location, $name, $page, $type, $url);
#
#    foreach $line (@input) {
#
#	($name, $type, $location) = split(/\s+\|\s+/, $line, 3);
#
#	$url  = "/cgi-bin/dwww";
#	$url .= "?type=$type&location=" . urlencode($location);
#
#	# This is sort of a waste, but it lends itself nicely to
#	# becoming a sorted, unique, listing.
#	$links{$name} = "<li><a href=\"$url\">$name</a>\n";
#    }    
#    
#    # This makes a nice list of documents, sorted by $name.
#    # We probably want a fancy header imported from some other
#    # file, but we can do that later.
#    $page  = "I hope these help!<p>";
#    $page .= "<ul>";		# begin the list.
#    foreach $entry (sort(keys(%links))) {
#	$page .= $links{$entry} . "\n";
#    }
#    $page .= "</ul>";		# end the list.
#
#    return($page);		# Done, return the HTMLized links!
#}

###############################################################################
# ReadParse() -- read in the data passed from the HTML form that called me.
###############################################################################
sub ReadParse {
    if (@_) {
        local (*in) = @_;
    }

    local ($i, $loc, $key, $val);

    # Read in text
    if ($ENV{'REQUEST_METHOD'} eq "GET") { # a GET -- data in encoded string
        $in = $ENV{'QUERY_STRING'};
    } elsif ($ENV{'REQUEST_METHOD'} eq "POST") { # a POST -- data in variables
        for ($i = 0; $i < $ENV{'CONTENT_LENGTH'}; $i++) {
            $in .= getc;
        }
    } elsif ($ENV{'REQUEST_METHOD'} eq "HEAD") {
        $in = $ENV{'QUERY_STRING'};
    }

    #
    # Read everything into a hashed array.
    #
    @in = split(/&/,$in);

    foreach $i (0 .. $#in) {
        # Convert plus's to spaces
        $in[$i] =~ s/\+/ /g;

        # Convert %XX from hex numbers to alphanumeric
        $in[$i] =~ s/%(..)/pack("c",hex($1))/ge;

	if ($in[$i] =~ m/^([-:a-zA-Z0-9+.=_\/ \[]*)$/) {
	    $in[$i] = $1;  # untaint
	} else {
	    error ( "Invalid characters in input: $in[$i]" );
	}

        # Split into key and value.
        $loc = index($in[$i],"=");
        $key = substr($in[$i],0,$loc);
        $val = substr($in[$i],$loc+1);
        # \0 is the multiple separator
        $in{$key} .= '\0' if (defined($in{$key}));
        $in{$key} .= $val;
    }

    return 1;			# just to keep things kosher.
}

###############################################################################
# PrintHeader() -- Use this before printing HTML data to the web-client.
###############################################################################
sub PrintHeader {
  print "Content-type: text/html; charset=ISO-8859-1\n\n";
}

###############################################################################
# error() -- call this to print error messages to the "screen" (web client)
###############################################################################
sub error {
    local($error_msg) = @_;

    print "Content-type: text/html\n\n";
    print "<HTML><HEAD><TITLE>Dwww error</TITLE></HEAD>\n";
    print "<BODY><H1>Dwww error</H1>\n";
    print "$error_msg\n";
    print "</BODY></HTML>";

    exit(1);
}

sub urlencode {
  local ($ret) =$_[0];
  $ret =~ s/([^A-Za-z0-9\_\-\.\/])/'%' . unpack("H*", $1)/eg;
  $ret =~ tr/ /+/;
  return $ret;
};

sub is_tainted {
    return ! eval {
	join('',@_), kill 0;
	1;
    };
}

