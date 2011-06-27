# vim:ft=perl:cindent:ts=4:sts=4:sw=4:et:fdm=marker:cms=\ #\ %s
#
package Debian::Dwww::Initialize;

use Exporter();
use Sys::Hostname;
use Debian::Dwww::ConfigFile;
use strict;

use vars qw(@ISA @EXPORT);
@ISA = qw(Exporter);
@EXPORT = qw(DwwwInitialize DwwwSetupDirs);

sub DwwwInitialize($) {
    my $filename = shift;
    my $hostname = &hostname();
    $hostname    =~ s/\..*$//;
    my $cfgvars  = ReadConfigFile($filename);
    my $dwwwvars = {};

    map +{ $dwwwvars->{$_} = $cfgvars->{$_}->{'cfgval'} ? $cfgvars->{$_}->{'cfgval'}
                                                       : $cfgvars->{$_}->{'defval'} }, keys %$cfgvars;

    umask (022);
    $ENV{'PATH'} = "/usr/sbin:/usr/bin:$ENV{'PATH'}";

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
