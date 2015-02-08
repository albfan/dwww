## dwww

> To boldly go where no doc has gone before

### Intro
dwww is a web interface to all on-line documentation on a Debian system.  It builds some web pages that list all installed documents, and converts all documents to HTML. The conversion is done when the user requests the document. 

dwww requires running a web server with CGI support (i.e. apache, boa, roxen, wn, etc., but NOT dhttpd or fnord.) Sorry.

For more information, read the manual page dwww(7), and the other manual pages it refers to.

  Automatic fancy index of documents
--------------------------------------

### alternatives
Debian now has a "doc-base" package that provides a uniform way of registering documentation; it supports other documentation systems besides dwww. Please read install-docs(8) for further reference.

> dwww is still under development!

dwww is already quite usable, however.  Please use it, and report bugs via the Debian bug tracking system so that they can be fixed.
