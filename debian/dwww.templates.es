Template: dwww/nosuchdir
Type: note
Description: Directory does not exists!
 Directory ${dir} does not exists.
Description-es: �No existe el directorio!
 El directorio ${dir} no existe.

Template: dwww/servertype
Type: select
Choices: ${choices}
Choices-es: ${choices}
Description: Which www server do you use?
 dwww found more than one server installed and needs to know which server
 do you use. Please select one from the list.
  Previously dwww was configured to: ${dwwwcfg}
Description-es: �Qu� servidor web utiliza?
 dwww ha encontrado m�s de un servidor instalado y necesita saber cu�l es
 el que usa. Por favor, elija uno de la lista.
  Anteriormente dwww estaba configurado as�: ${dwwwcfg}

Template: dwww/nosuchuser
Type: note
Description: User not found!
 User ${user} does not exists.
Description-es: �Usuario no encontrado!
 El usuario ${user} no existe.

Template: dwww/serverport
Type: string
Default: 80
Default-es: 80
Description: Webserver's port.
 dwww needs to know what port your webserver is running on.  Normally web
 servers run on port 80.
  Previously dwww was configured to: ${dwwwcfg}
  Your webserver is set to: ${httpd}
Description-es: Puerto del servidor web.
 dwww necesita conocer en qu� puerto est� escuchando el servidor web.
 Normalmente, los servidores web utilizan el puerto 80.
  Anteriormente dwww estaba configurado as�: ${dwwwcfg}
  Su servidor web es: ${httpd}

Template: dwww/cgiuser
Type: string
Default: www-data
Default-es: www-data
Description: Name of CGI user.
 dwww now needs to know what user will be running the dwww CGI script,
 since that user needs to have ownership of the cache directory.
  Previously dwww was configured to: ${dwwwcfg}
  Your webserver is set to: ${httpd}
Description-es: Nombre del usuario que ejecuta el CGI.
 dwww tiene que saber qu� usuario va a ejecutar el script CGI dwww, porque
 ese usuario tiene que ser el propietario del directorio de cach�.
  Anteriormente dwww estaba configurado para: ${dwwwcfg}
  Su servidor web es: ${httpd}

Template: dwww/noserver
Type: note
Description: No www server found!
 dwww could not find any www server. Probably you are preconfiguring dwww
 and your www server has not been unpacked yet.
 .
 Please install web server and run
  dpkg-reconfigure dwww
 to continue configuration of dwww.
Description-es: �No hay ning�n servidor web instalado!
 dww no ha encontrado ning�n servidor web. Es probable que est�
 preconfigurando dwww y a�n no se haya desempaquetadp el servidor web.
 .
 Por favor, instale el servidor web y ejecute
  dpkg-reconfigure dwww
 para continuar la configuraci�n de dwww.

Template: dwww/servername
Type: string
Default: localhost
Default-es: localhost
Description: Host name of the webserver.
 dwww needs to know what the host name of your webserver is.
  Previously dwww was configured to: ${dwwwcfg}
  Your webserver is set to: ${httpd}
Description-es: Nombre del sistema que aloja el servidor web.
 dwww necesita saber el nombre de la m�quina en la que se encuentra el
 servidor web.
  Anteriormente dwww estaba configurado as�: ${dwwwcfg}
  Su servidor web es: ${httpd}

Template: dwww/docrootdir
Type: string
Default: /var/www
Default-es: /var/www
Description: Location of web server's document root.
 dwww now needs to know where is the directory which contains the document
 root for your web server. The web standard suggests /var/www.
  Previously dwww was configured to: ${dwwwcfg}
  Your webserver is set to: ${httpd}
Description-es: Ubicaci�n del documento ra�z del servidor web.
 dwww necesita que se le indique d�nde se encuentra el direcotrio que
 contiene el documento ra�z del servidor web. El est�ndar web sugiere
 /var/www.
  Anteriormente dwww estaba configurado as�: ${dwwwcfg}
  Su servidor web es: ${httpd}

Template: dwww/badport
Type: note
Description: Port value should be a number!
 Value entered for port: ${port} is invalid.
Description-es: �El puerto debe ser un n�mero!
 El valor proporcionado: ${port} no es v�lido.

Template: dwww/cgidir
Type: string
Default: /usr/lib/cgi-bin
Default-es: /usr/lib/cgi-bin
Description: Location of web server's cgi directory.
 dwww now needs to know where the directory which contains the CGI scripts
 for your web server exists.  The web standard suggests /usr/lib/cgi-bin,
 but your web server may already be configured for a different location.
  Previously dwww was configured to: ${dwwwcfg}
  Your webserver is set to: ${httpd}
Description-es: Ubicaci�n del directorio cgi del servidor web.
 dwww necesita conocer cu�l es el directorio que contiene los scripts cgi
 del servidor web. El est�ndar web sugiere /usr/lib/cgi-bin, pero puede que
 haya configurado el servidor web para que sea otro distinto.
  Anteriormente dwww estaba configurado as�: ${dwwwcfg}
  Su servidor web es: ${httpd}
