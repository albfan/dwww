# vim:ft=perl:cindent:ts=4:sw=4:et:fdm=marker:cms=\ #\ %s
#
# $Id: Utils.pm,v 1.8 2005/06/23 20:58:55 robert Exp $
#
package Debian::Dwww::Utils;

use Exporter();
use Debian::Dwww::Version;
use POSIX qw(strftime locale_h);
use strict;

use vars qw(@ISA @EXPORT);
@ISA = qw(Exporter);
@EXPORT = qw(URLEncode HTMLEncode  HTMLEncodeAbstract
	     TemplateFile BeginTable AddToTable EndTable $TRUE $FALSE);

my $TRUE  = 1;
my $FALSE = 0; 

sub URLEncode {
	my $url = shift;
	$url =~ s/([^A-Za-z0-9\ \_\-\.\/])/"%" . unpack("H*", $1)/eg; 
	$url =~ tr/ /+/;
	return $url;
}

# HTMLEncode(what, is_abstract)
sub HTMLEncode {
	my $text = shift;


	$text =~ s/&/&amp;/g;
	$text =~ s/</&lt;/g;
	$text =~ s/>/&gt;/g;
	$text =~ s/"/&quot;/g;
	return $text;
}

sub HTMLEncodeAbstract {
	my $text = &HTMLEncode(@_);

	$text =~ s/^\s\s+(.*)$/<BR><TT>&nbsp;$1<\/TT><BR>/gm;
	$text =~ s/^\s\.\s*$/<BR>/gm;
	$text =~ s/(<BR>\s*)+/<BR>\n/g;
	$text =~ s/(http|ftp)s?:\/([\w\/~\.%-])+[\w\/]/<A href="$&">$&<\/A>/g;
	$text =~ s/<BR>\s*$//;
	return $text;
}

sub GetDate {
    my $old_locale = &setlocale(LC_ALL, "C");
    my $date = &strftime ("%a %b %e %H:%M:%S %Z %Y", localtime(time));
    &setlocale(LC_ALL, $old_locale) unless $old_locale eq "C";
    return $date;
}
    

sub TemplateFile {
	my $file= shift;
	my $vars= shift; # hash reference 
 	my $res = '';		
	local $_;
    
	open TEMPLATE , "<$file" or die "Can't open $file: $!";
	while (<TEMPLATE>) {
		foreach my $k (keys %{$vars}) {
			s/\%$k\%/$vars->{$k}/g
		}
		s/\%VERSION\%/$Debian::Dwww::Version::version/go;
		s/\%DATE\%/&GetDate()/eg;
		$res .= $_;
	}

	close TEMPLATE;
	return $res;
}


sub BeginTable {
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
}

sub AddToTable {
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
		} else	{
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
}

sub EndTable {
	my $filehandle = shift;
        my $table = shift;

        while ($table->{'in_column'} != 0) {
                &AddToTable($filehandle, $table, '');
        }
        print $filehandle "</TABLE>\n";

        undef %{$table};
}



1;
