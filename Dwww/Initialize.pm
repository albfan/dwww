#
# $Id: Initialize.pm,v 1.3 2003/03/08 16:24:37 robert Exp $
# 
package Debian::Dwww::Initialize;

use Exporter();
use strict;

use vars qw(@ISA @EXPORT);
@ISA = qw(Exporter);
@EXPORT = qw(DwwwInitialize);

sub DwwwInitialize() {
	my $filename = shift;
	my $dwwwvars = {
		'DWWW_DOCPATH' => "/usr/share/doc:/usr/doc:/usr/share/info:/usr/info:"
				 . "/usr/share/man:/usr/man:/usr/X11R6/man:/usr/local/man:"
                                 . "/usr/local/doc:/usr/local/info:/usr/share/common-licenses",
		'DWWW_ALLOWEDLINKPATH'	=> "/usr/share:/usr/lib:/var/www",
		'DWWW_HTMLDIR'		=> "/var/lib/dwww/html",
		'DWWW_USE_CACHE'	=> "yes",
		'DWWW_KEEPDAYS'		=> 10,
		'DWWW_QUICKFIND_DB'	=> "/var/lib/dwww/quickfind.dat",
		'DWWW_TITLE'		=> 'dwww: ' . `hostname`,
		'DWWW_CGIUSER'		=> "www-data"
	};

	umask (022);
	$ENV{'PATH'} = "/usr/sbin:/usr/bin:$ENV{'PATH'}";

	return $dwwwvars  unless defined $filename;
	return $dwwwvars  unless -r $filename;

	open DWWWCONF, "<$filename" or die "Can't open $filename: $!\n";
	while (<DWWWCONF>) {
		chomp();
		if (m/^\s*[^=]+\s*=\s*(\S)+\s*$/) {
			$dwwwvars->{$1} = $2;
		}
	}
	
	return $dwwwvars;
}

1;
