# vim:ft=perl:cindent
#
# $Id: DocBase.pm,v 1.5 2003-05-16 17:22:33 robert Exp $
#
package Debian::Dwww::DocBase;

use Exporter();
use Debian::Dwww::Version;
use strict;

use vars qw(@ISA @EXPORT $ErrorProc);
@ISA 	= qw(Exporter);
@EXPORT = qw(ParseDocBaseFile DwwwSection2Section $ErrorProc);



sub ParseDocBaseFile {
    my $file    = shift;
    my $format  = undef;
    my $entry   = {};
    my ($fld, $val, $lastfld) = ('', '', ''); 
    my $line  = 0;
    local $_;

    if (not open DOCFILE, $file) {
        &$ErrorProc($file, "Can't be opened: $!");
        return undef;
    }

    while (<DOCFILE>) {
        chomp;
	s/\s+$//;
        $line++;    
        if (/^\s*$/) {
            # empty lines separate sections
            $format  = '';  # here we define $format
            $lastfld = '';    
        } elsif (/^(\S+)\s*:\s*(.*)\s*$/) {
            ($fld, $val) = (lc $1, $2);


            if (not defined $format) {
                    $entry->{$fld} = $val;
            } elsif ($format eq '' and $fld eq 'format') {
                    $format = lc $val;
            } elsif ($format ne '' and $fld eq 'index') {
                    $entry->{'formats'}->{$format}->{'index'} = $val;
            } elsif ($format ne '' and $fld eq 'files') {
                    $entry->{'formats'}->{$format}->{'files'} = $val;
            } else {
                    goto PARSE_ERROR;
            }
            $lastfld = $fld;
        } elsif (/^\s+/ and $lastfld ne '') {
            $entry->{$lastfld} .= "\n$_";
        } else {
                goto PARSE_ERROR;
        }
    }
            
    close DOCFILE;

    return $entry;
    

PARSE_ERROR:
    &$ErrorProc($file, "Parse error at line $line");
    close DOCFILE;
    return undef;
}


sub DwwwSection2Section {
        my $entry = shift;
       	
	my $sec   = $entry->{'dwww-section'} if defined $entry->{'dwww-section'};
     	my $title = defined $entry->{'dwww-title'} ? $entry->{'dwww-title'} : 
		      defined $entry->{'title'} ? $entry->{'title'} : undef;
	
        return unless defined $sec and defined $title;

	if (length($sec) > length($title) &&
	     substr ($sec, -length($title))  eq $title) {
		$sec = substr ($sec, 0, -length($title));
	} else {
		return;
	}
	
	$sec =~ s|^/+||;
	$sec =~ s|/+$||;
	$entry->{'section'} = $sec;
                        
}


1;
