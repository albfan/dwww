#!/usr/bin/perl -T
# vim:ft=perl:cindent
#
#              IF YOU SEE THIS TEXT
#         PLEASE MAKE SURE YOUR WEB SERVER
#           HAS THE CGI SUPPORT ENABLED
#  (if you are using apache2, see /usr/share/doc/dwww/README)
#
#
# $Id: dwww.cgi,v 1.18 2005-10-05 21:25:04 robert Exp $
#

$doc2html 	= '/usr/sbin/dwww-convert'; # Document-to-HTML converter
$search2html 	= '/usr/sbin/dwww-find';    # Search and output results as HTML
@searchargs   	= ('--package');	    # Default argument for $search2html	
$ENV{PATH} 	= '/bin:/usr/bin:/usr/sbin';
delete @ENV{'IFS', 'CDPATH', 'ENV', 'BASH_ENV'}; # Delete unsafe variables

$TRUE  = 1;
$FALSE = 0;

&ReadParse();			# Get CGI info (see below for function)

#
# Figure out if this is a multiple word search or a straight request.
# If the former, split it on commas (,).
#
if ($in{'search'} ne '') {
	$search = $TRUE;
    	@searchstring = split(/,/, $in{'search'});
	
    	if ($in{'searchtype'} eq 'm') {
		@searchargs = ('--menu');
    	} 
	elsif ($in{'searchtype'} eq 'd') {
		@searchargs = ('--documentation');
		if (defined $in{'skip'}) {
			push(@searchargs, "--skip=$in{'skip'}");
		}
    	}
} 
else {
    	$search   = $FALSE;
    	$type     = $in{'type'};	 # This document is formated in $type.
    	$location = $in{'location'}; # It is located at $location.
}

#
# Ok, now that we know the type, we need to perform the search or
# send them the requested document.  
#

if ($search == $TRUE) {
	&PrintHeader();

	exec { "$search2html" } "$search2html", @searchargs, @searchstring;
	&error($FALSE, "Couldn't search for @searchstring! ($!)");
} 

elsif (($type eq "") or ($location eq "")) {
	local $port = defined $ENV{'SERVER_PORT'} ? ':' . $ENV{'SERVER_PORT'} : '';

	print "Location: http://$ENV{'SERVER_NAME'}$port/dwww/\n\n";

} 

else {

    # Execute $doc2html, telling it that the format is of type $type, and the
    # requested document at $location. 
 
	exec { "$doc2html" } "$doc2html", "$type", "$location";
	&error($TRUE, "Couldn't convert document $location! ($!)");
}


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
    	} 
	elsif ($ENV{'REQUEST_METHOD'} eq "POST") { # a POST -- data in variables
        	for ($i = 0; $i < $ENV{'CONTENT_LENGTH'}; $i++) {
            	$in .= getc;
        	}
    	} 
	elsif ($ENV{'REQUEST_METHOD'} eq "HEAD") {
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
		} 
		else {
	    		$in[$i] =~ s/&/&amp;/g;	
	    		$in[$i] =~ s/</&lt;/g;	
	    		$in[$i] =~ s/>/&gt;/g;
	    		&error ($TRUE, "Invalid characters in input: $in[$i]" );
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
	local($print_header) = shift;;
    	local($error_msg) 	 = @_;

    	&PrintHeader() unless $print_header eq $FALSE;
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

