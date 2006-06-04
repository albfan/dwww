# vim:ft=perl:cindent:ts=4:sw=4:et:fdm=marker:cms=\ #\ %s
#
# $Id: Utils.pm,v 1.9 2006-05-07 18:30:08 robert Exp $
#
package Debian::Dwww::Utils;

use Exporter();
use Debian::Dwww::Version;
use Cwd qw(cwd realpath);
use POSIX qw(strftime locale_h);
use File::Path qw/rmtree mkpath/;
use File::NCopy qw/copy/;

use strict;

use vars qw(@ISA @EXPORT);
@ISA = qw(Exporter);
@EXPORT = qw(URLEncode HTMLEncode  HTMLEncodeAbstract StripDirs CheckAccess RedirectToURL ErrorMsg
         TemplateFile BeginTable AddToTable EndTable GetCommandOutput RenameDir $TRUE $FALSE);

our $TRUE  = 1;
our $FALSE = 0; 

sub URLEncode {
	my $url = shift;
	$url =~ s/([^A-Za-z0-9\_\-\.\/])/"%" . unpack("H*", $1)/eg; 
#	$url =~ tr/ /+/;
	return $url;
}

# HTMLEncode(what)
sub HTMLEncode { # {{{
	my $text = shift;


	$text =~ s/&/&amp;/g;
	$text =~ s/</&lt;/g;
	$text =~ s/>/&gt;/g;
	$text =~ s/"/&quot;/g;
	return $text;
} # }}}

sub HTMLEncodeAbstract { # {{{
    my $text = &HTMLEncode(@_);

    $text =~ s/^\s\s+(.*)$/<BR><TT>&nbsp;$1<\/TT><BR>/gm;
    $text =~ s/^\s\.\s*$/<BR>/gm;
    $text =~ s/(<BR>\s*)+/<BR>\n/g;
    $text =~ s/(http|ftp)s?:\/([\w\/~\.%#-])+[\w\/]/<A href="$&">$&<\/A>/g;
    $text =~ s/<BR>\s*$//;
    return $text;
} # }}}

sub GetDate { # {{{
    my $old_locale = &setlocale(LC_ALL, "C");
    my $date = &strftime ("%a %b %e %H:%M:%S %Z %Y", localtime(time));
    &setlocale(LC_ALL, $old_locale) unless $old_locale eq "C";
    return $date;
} # }}}
    

sub TemplateFile { # {{{
    my $file= shift;
    my $vars= shift; # hash reference 
    my $res = '';
    local $_;


    open TEMPLATE , "<$file" or die "Can't open $file: $!";
    while (<TEMPLATE>) {
        foreach my $k (keys %{$vars}) {
            s/\%$k\%/$vars->{$k}/g
        }
        s/\%VERSION\%/$Debian::Dwww::Version::version/o;
        s/\%DATE\%/&GetDate()/eg;
        $res .= $_;
    }

    close TEMPLATE;
    return $res;
} # }}}


sub BeginTable { # {{{
    my $filehandle = shift;
        my $caption = shift;
        my $columns = shift;
    my $desc    = shift;
    my $widths  = shift;
        my $table = {};

    $desc = '' unless (defined $desc);

        $table->{'columns'}   = $columns + 0;
        $table->{'widths'}    = $widths;
        $table->{'in_column'} = 0;
        $table->{'in_row'}    = 0;

        print $filehandle "<P align=\"left\">\n";
        print $filehandle "<STRONG>$caption</STRONG>$desc\n";
        print $filehandle "<TABLE border=\"0\" width=\"98%\" align=\"center\">\n";
        return $table;
} # }}}

sub AddToTable { # {{{
    my $filehandle = shift;
    my $table = shift;
    my $what = shift;
    my ($wdth, $c, $r);
        
    $c = $table->{'in_column'};
    $r = $table->{'in_row'};
    

    if ($c == 0) {
        print $filehandle "<TR>\n"
    }
    
    if ($r == 0 && $c + 1 < $table->{'columns'}) {
    if (defined $table->{'widths'}) {
        $wdth = ' width="' . $table->{'widths'}[int($c)] .'%"';
    } else  {
        $wdth = ' width="' . int(100 / $table->{'columns'}) . '%"';
    }
    } else {
        $wdth = '';
    }

    print $filehandle "<TD align=\"left\"$wdth>$what</TD>\n";

    if (++$c >= $table->{'columns'}) {
        print $filehandle "</TR>\n";
        $c = 0;
        $r++;
    }
    $table->{'in_column'} = $c; 
    $table->{'in_row'}    = $r;
} # }}}


sub EndTable { # {{{
    my $filehandle = shift;
    my $table = shift;

    while ($table->{'in_column'} != 0) {
        &AddToTable($filehandle, $table, '');
    }
    print $filehandle "</TABLE>\n";

    undef %{$table};
} # }}}

# strips any '.' and '..' components from path
sub StripDirs { # {{{
    my $path = shift;


    $path = &cwd() . '/' . $path unless $path =~ /^\//;

    my @pc = split(/\/+/, $path);
    my @res = ();
    
    foreach my $p (@pc) {
        next             if $p eq '.' || $p eq '';
        pop(@res), next  if $p eq '..';
        push(@res, $p);
    }
    my $r = '/' . join ('/', @res);
    return $r;
    
} # }}}

#
# Print error message and exit the program
# usage: ErrorMsg status title message
sub ErrorMsg { # {{{
	my $status 	= shift;
	my $title	= shift;
	my $message     = shift;

	print "Status: $status\n";
	print "Content-type: text/html; charset=iso-8895-1\n";
	print "\n";
	print "<HTML>\n";
	print "<HEAD>\n";
	print " <TITLE>$title</TITLE>\n";
	print "</HEAD>";
	print "<BODY>";
	print " <H1 align=\"center\">$title</H1>\n";
	print "$message\n";
	print "</BODY>\n";
	print "</HTML>\n";
	exit 1;
} # }}}


# returns output of realpath($file)
sub CheckAccess() { # {{{
    my $dwwwvars    = shift;
    my $file        = shift;
    my $orig_file   = shift;
    $orig_file      = $file unless defined $orig_file;
        
    my $dwww_docpath            = $dwwwvars->{'DWWW_DOCPATH'};
    my $dwww_allowedlinkpath    = $dwwwvars->{'DWWW_ALLOWEDLINKPATH'};

    
    my $can_read_f              = -r $file;
    my $exists_f                = $can_read_f || -f $file;
    my $realp_file              = undef;        


    if ( $exists_f ) {
        $realp_file = &realpath( $file );
        
        # file does exist, check if it match any files in @dwww_docpath
        foreach my $path  (@$dwww_docpath)  {
            if ( -d $path ) {
                $path = &realpath( $path );
                if (substr($realp_file, 0, length($path)) eq $path) {
                    &ErrorMsg( "403 Access Denied",
                               "Access Denied",
                               "The $orig_file is not readable!" ) unless $can_read_f;
                    return $realp_file; # everything OK
                }
            }
        }
    }

    # if we're here, the file either does not exist
    # or does not match any @dwww_docpath
    my $ok          = 0;
    my $strip_file  = &StripDirs( $file );
    foreach my $path  (@$dwww_docpath)  {
        if ( -d $path ) {
            $path = &StripDirs( $path );
            if (substr($strip_file, 0, length($path)) eq $path) {
                $ok = 1;
                last;
            }
        }
    }
 

    # if file exists, check if it's in allowed_linkpath
    if ( $exists_f && $ok ) {
        foreach my $path (@$dwww_allowedlinkpath) {
            if ( -d $path ) {
                $path = &realpath( $path );
                if (substr($realp_file, 0, length($path)) eq $path) {
                    return $realp_file;
                }
            }
        }
    }

    # file either does not exist or is not allowed to show
    # print suitable error message   
    if ( !$exists_f && $ok ) {
        &ErrorMsg ("404 File not found",
                   "File not found" ,
                   "dwww could not find the file $orig_file" );
    } else {    
        &ErrorMsg ("403 Access denied",
                    "Access denied", 
                    "dwww will not allow you to read the file $orig_file" );
    }
    
    exit 1; ### UNREACHED ###
} # }}}

sub GetCommandOutput { # {{{
        my @args = @_;
	# fork and exec command
        open (OUT, '-|')
                || exec { $args[0] } @args;
        my @out=<OUT>;
        close OUT;
        return @out;
} # }}}
                
sub RedirectToURL() { # {{{
    my $url = shift;
    
    my $name = defined $ENV{'SERVER_NAME'} ? $ENV{'SERVER_NAME'} : 'localhost';
	my $port = defined $ENV{'SERVER_PORT'} ? ':' . $ENV{'SERVER_PORT'} : '';
    $url = "/$url" unless $url =~ m/^\//;

	print "Location: http://$name$port$url\n\n";
} # }}}

sub RenameDir() { # {{{
    my ($srcdir, $tgtdir) = @_;
    
    &rmtree($tgtdir) or die "Cannot remove old $tgtdir directory: $!\n" if -d $tgtdir;
    if (! rename($srcdir, $tgtdir)) {
        &mkpath($tgtdir) or die "Cannot create $tgtdir: $!\n";
        &copy(\1, "$srcdir/*", $tgtdir) or die "Cannot copy $srcdir to $tgtdir: $!\n";
        &rmtree($srcdir);
    }        
} # }}}
    

1;
