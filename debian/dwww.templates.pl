Template: dwww/servertype
Type: select
Choices: ${choices}
Choices-pl: ${choices}
Description: Which www server do you use?
 dwww found more than one server installed and needs to know which server
 do you use. Please select one from the list.
  Previously dwww was configured to: ${dwwwcfg}
Description-pl: Jaki serwer www jest u�ywany?
 dwww wykry�, �e w systemie jest zainstalowany wi�cej ni� jeden serwer www
 i musi wiedzie�, kt�ry serwer jest u�ywany. Prosz� wybra� jeden z listy.

Template: dwww/docrootdir
Type: string
Default: /var/www
Default-pl: /var/www
Description: Location of web server's document root.
 dwww now needs to know where is the directory which contains the document
 root for your web server. The web standard suggests /var/www.
  Previously dwww was configured to: ${dwwwcfg}
  Your webserver is set to: ${httpd}
Description-pl: Lokalizacja g��wnego katalogu dokument�w serwera www.
 dwww musi wiedzie�, gdzie znajduje si� katalog b�d�cy g��wnym katalogiem
 serwera www. Standardowo jest to /var/www.
  Poprzednia konfiguracja dwww: ${dwwwcfg}
  Konfiguracja serwera www: ${httpd}

Template: dwww/cgidir
Type: string
Default: /usr/lib/cgi-bin
Default-pl: /usr/lib/cgi-bin
Description: Location of web server's cgi directory.
 dwww now needs to know where the directory which contains the CGI scripts
 for your web server exists.  The web standard suggests /usr/lib/cgi-bin,
 but your web server may already be configured for a different location.
  Previously dwww was configured to: ${dwwwcfg}
  Your webserver is set to: ${httpd}
Description-pl: Lokalizacja katalogu CGI serwera.
 dwww musi zna� lokalizacj� katalogu zawieraj�cego skrypty CGI.  W
 standardowej konfiguracji jest to katalog /usr/lib/cgi-bin, ale Tw�j
 serwer www mo�e by� inaczej skonfigurowany.
  Poprzednia konfiguracja dwww: ${dwwwcfg}
  Konfiguracja serwera www: ${httpd}

Template: dwww/cgiuser
Type: string
Default: www-data
Default-pl: www-data
Description: Name of CGI user.
 dwww now needs to know what user will be running the dwww CGI script,
 since that user needs to have ownership of the cache directory.
  Previously dwww was configured to: ${dwwwcfg}
  Your webserver is set to: ${httpd}
Description-pl: Nazwa u�ytkownika wykonuj�cego programy CGI
 dwww musi zna� nazw� u�ytkownika, kt�ry b�dzie wykonywa� skrypty CGI
 u�ywane w programie dwww, poniewa� ten u�ytkownik b�dzie w�a�cicielem
 katalogu cache dwww. 
  Poprzednia konfiguracja dwww: ${dwwwcfg}
  Konfiguracja serwera www: ${httpd}

Template: dwww/servername
Type: string
Default: localhost
Default-pl: localhost
Description: Host name of the webserver.
 dwww needs to know what the host name of your webserver is.
  Previously dwww was configured to: ${dwwwcfg}
  Your webserver is set to: ${httpd}
Description-pl: Nazwa sieciowa serwera www.
 dwww musi zna� nazw� sieciow� Twojego serwera www.
  Poprzednia konfiguracja dwww: ${dwwwcfg}
  Konfiguracja serwera www: ${httpd}

Template: dwww/serverport
Type: string
Default: 80
Default-pl: 
Description: Webserver's port.
 dwww needs to know what port your webserver is running on.  Normally web
 servers run on port 80.
  Previously dwww was configured to: ${dwwwcfg}
  Your webserver is set to: ${httpd}
Description-pl:  Port serwera www.
 dwww musi zna� numer portu, na kt�rym nas�uchuje serwer www. Zazwyczaj
 serwer www jest uruchamiany na porcie 80.
  Poprzednia konfiguracja dwww: ${dwwwcfg}
  Konfiguracja serwera www: ${httpd}

Template: dwww/noserver
Type: note
Description: No www server found!
 dwww could not find any www server. Probably you are preconfiguring dwww
 and your www server has not been unpacked yet.
 .
 Please install web server and run
  dpkg-reconfigure dwww
 to continue configuration of dwww.
Description-pl: Nie znaleziono serwera www!
 dwww nie znalaz� �adnego serwera www. Prawdopodobnie w tej chwili 
 prekonfigurujesz dwww i Tw�j webserwer nie zosta� jeszcze rozpakowany.
 .
 Prosz� zainstalowa� serwer www i uruchomi� 
  dpkg-reconfigure dwww
 aby kontynuowa� konfiguracj� programu dwww.

Template: dwww/nosuchdir
Type: note
Description: Directory does not exists!
 Directory ${dir} does not exists.
Description-pl: Katalog nie istnieje!
 Katalog ${dir} nie istnieje.

Template: dwww/nosuchuser
Type: note
Description: User not found!
 User ${user} does not exists.
Description-pl: Nie znaleziono u�ytkownika!
 U�ytkownik ${user} nie istnieje.

Template: dwww/badport
Type: note
Description: Port value should be a number!
 Value entered for port: ${port} is invalid.
Description-pl: Port powinien by� typu numerycznego!
 Warto�� podana w polu port: ${port} jest niepoprawna.
