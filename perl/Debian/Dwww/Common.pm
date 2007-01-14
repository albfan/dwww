# vim:ft=perl:cindent:ts=4:sw=4:et:fdm=marker:cms=\ #\ %s
#
# $Id: Common.pm,v 1.2 2006-05-07 18:30:08 robert Exp $
# 
package Debian::Dwww::Common;

use Exporter();
use strict;

use Debian::Dwww::Utils;
use vars qw(@ISA @EXPORT);
@ISA = qw(Exporter);
@EXPORT = qw(GetURL);


my $dwww_url = "/cgi-bin/dwww";
my %href  = # {{{
    (
        'debiandoc-sgml'=>   "$dwww_url#FILE#?type=application/sgml",
        'docbook-xml'   =>   "$dwww_url#FILE#?type=text/xml",
        'dvi'           =>   "$dwww_url#FILE#?type=application/x-dvi",
        'html'          =>   "$dwww_url#FILE#?type=html",
        'info'          =>   "/cgi-bin/info2www?file=",
        'latex'         =>   "$dwww_url#FILE#?type=application/x-latex",
        'linuxdoc-sgml' =>   "$dwww_url#FILE#?type=application/sgml",
        'pdf'           =>   "$dwww_url#FILE#?type=application/pdf",
        'postscript'    =>   "$dwww_url#FILE#?type=application/postscript",
        'ps'            =>   "$dwww_url#FILE#?type=application/postscriptps",
        'rtf'           =>   "$dwww_url#FILE#?type=text/rtf",
        'sgml'          =>   "$dwww_url#FILE#?type=application/sgml",
        'tar'           =>   "$dwww_url#FILE#?type=application/tar",
        'texinfo'       =>   "$dwww_url#FILE#?type=application/x-texinfo",
        'dwww-url'      =>   "",
        'text'          =>   "$dwww_url#FILE#?type=text/plain",
        'pkgsearch'     =>   "$dwww_url?search=",
        'man'           => '/cgi-bin/dwww#FILE#?type=man', 
        'runman'        => '/cgi-bin/dwww?type=runman&amp;location=',
        'dir'           => '/cgi-bin/dwww#FILE#/?type=dir',
        'info'          => '/cgi-bin/info2www?file=',
        'file'          => '/cgi-bin/dwww',
        'menu'          => '/dwww/menu/',
        'search'        => '/cgi-bin/dwww?search=',
        'dpkg'          => '/cgi-bin/dpkg?query='
    ); # }}}

sub GetURL{ # {{{
    my $format_name = shift;
    my $format_url  = shift;
    my $dont_encode = shift;
    $dont_encode    = $FALSE unless defined $dont_encode;
        

    $format_url = &URLEncode($format_url) unless ($dont_encode || $format_name eq 'dwww-url');

    if ($href{$format_name} =~ /#FILE#/) {
        return $` . $format_url . $';
    } else {
        return $href{$format_name} . $format_url;
    }


} # }}}

1;
