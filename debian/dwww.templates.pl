Template: dwww/servertype
Type: select
Choices: ${choices}
Choices-pl: ${choices}
Description: Which www server do you use?
 dwww found more than one server installed and needs to know which server
 do you use. Please select one from the list.
  Previously dwww was configured to: ${dwwwcfg}
Description-pl: Jaki serwer www jest u¿ywany?
 dwww wykry³, ¿e w systemie jest zainstalowany wiêcej ni¿ jeden serwer www
 i musi wiedzieæ, który serwer jest u¿ywany. Proszê wybraæ jeden z listy.

Template: dwww/docrootdir
Type: string
Default: /var/www
Default-pl: /var/www
Description: Location of web server's document root.
 dwww now needs to know where is the directory which contains the document
 root for your web server. The web standard suggests /var/www.
  Previously dwww was configured to: ${dwwwcfg}
  Your webserver is set to: ${httpd}
Description-pl: Lokalizacja g³ównego katalogu dokumentów serwera www.
 dwww musi wiedzieæ, gdzie znajduje siê katalog bêd±cy g³ównym katalogiem
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
 dwww musi znaæ lokalizacjê katalogu zawieraj±cego skrypty CGI.  W
 standardowej konfiguracji jest to katalog /usr/lib/cgi-bin, ale Twój
 serwer www mo¿e byæ inaczej skonfigurowany.
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
Description-pl: Nazwa u¿ytkownika wykonuj±cego programy CGI
 dwww musi znaæ nazwê u¿ytkownika, który bêdzie wykonywa³ skrypty CGI
 u¿ywane w programie dwww, poniewa¿ ten u¿ytkownik bêdzie w³a¶cicielem
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
 dwww musi znaæ nazwê sieciow± Twojego serwera www.
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
 dwww musi znaæ numer portu, na którym nas³uchuje serwer www. Zazwyczaj
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
 dwww nie znalaz³ ¿adnego serwera www. Prawdopodobnie w tej chwili 
 prekonfigurujesz dwww i Twój webserwer nie zosta³ jeszcze rozpakowany.
 .
 Proszê zainstalowaæ serwer www i uruchomiæ 
  dpkg-reconfigure dwww
 aby kontynuowaæ konfiguracjê programu dwww.

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
Description-pl: Nie znaleziono u¿ytkownika!
 U¿ytkownik ${user} nie istnieje.

Template: dwww/badport
Type: note
Description: Port value should be a number!
 Value entered for port: ${port} is invalid.
Description-pl: Port powinien byæ typu numerycznego!
 Warto¶æ podana w polu port: ${port} jest niepoprawna.
