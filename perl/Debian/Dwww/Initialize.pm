# vim:ft=perl:cindent:ts=4:sw=4:et:fdm=marker:cms=\ #\ %s
#
# $Id: Initialize.pm,v 1.11 2006-06-04 14:38:20 robert Exp $
# 
package Debian::Dwww::Initialize;

use Exporter();
use Sys::Hostname;
use strict;

use vars qw(@ISA @EXPORT);
@ISA = qw(Exporter);
@EXPORT = qw(DwwwInitialize DwwwSetupDirs);

sub DwwwInitialize() {
	my $filename = shift;
	my $hostname = &hostname();
	$hostname    =~ s/\..*$//;
	my $dwwwvars = {
		'DWWW_DOCPATH' => "/usr/share/doc:/usr/doc:/usr/share/info:/usr/info:"
				 . "/usr/share/man:/usr/man:/usr/X11R6/man:/usr/local/man:"
                                 . "/usr/local/doc:/usr/local/info:/usr/share/common-licenses",
		'DWWW_ALLOWEDLINKPATH'	=> "/usr/share:/usr/lib:/var/www",
		'DWWW_TMPDIR'		=> "/var/lib/dwww",
		'DWWW_USE_CACHE'	=> "yes",
		'DWWW_KEEPDAYS'		=> 10,
		'DWWW_QUICKFIND_DB'	=> "/var/cache/dwww/quickfind.dat",
		'DWWW_REGDOCS_DB'	=> "/var/cache/dwww/regdocs.dat",
		'DWWW_DOCBASE2PKG_DB'	=> "/var/cache/dwww/docbase2pkg.dat",
		'DWWW_TITLE'		=> 'dwww: ' . $hostname,
		'DWWW_DOCROOTDIR' 	=> '/var/www',
		'DWWW_CGIDIR' 		=> '/usr/lib/cgi-bin',
		'DWWW_CGIUSER' 		=> 'www-data',
		'DWWW_SERVERNAME' 	=> 'localhost',
		'DWWW_SERVERPORT' 	=> 80

	};

	umask (022);
	$ENV{'PATH'} = "/usr/sbin:/usr/bin:$ENV{'PATH'}";

	return $dwwwvars  unless defined $filename;
	return $dwwwvars  unless -r $filename;

	open DWWWCONF, "<$filename" or die "Can't open $filename: $!\n";
	while (<DWWWCONF>) {
		chomp();
		if (m/^\s*([^=]+)\s*=\s*(\S+)\s*$/) {
			$dwwwvars->{$1} = $2;
		}
	}
    close DWWWCONF or die "Can't close $filename: $!\n";
	foreach my $k ( 'DWWW_DOCPATH', 'DWWW_ALLOWEDLINKPATH' ) {
        my @paths =  split( /:/, $dwwwvars->{$k} ); 
		$dwwwvars->{$k} = \@paths;
    }
	
	return $dwwwvars;
}

sub DwwwSetupDirs() {
	my $dwwwvars = shift;

	my $dir = "/var/cache/dwww";
	if ( ! -d "$dir" ) {
		mkdir "$dir", 0755 or die "Cannot create directory $dir";
		chown 0, 0, "$dir"
	}
	if ( ! -d "$dir/db" ) {
		mkdir "$dir/db", 0755 or die "Cannot create directory $dir/db";
		my $uid = (getpwnam("$dwwwvars->{'DWWW_CGIUSER'}"))[2] or die "User $dwwwvars->{'DWWW_CGIUSER'} does not exist\n";
		chown $uid, 0, "$dir/db";
	}
}
1;