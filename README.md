
#About WebNotify

WebNotify allows you to display a text message as a notiﬁcation “balloon” on the computer’s desktop by POSTing the message to 
one of the following endpoints.

    - `POST /info`: Display an informational message
    - `POST /warn`: Display a warning message
    - `POST /error`: Display an error message

The notiﬁcation message should be sent in the request body, encoded as text/plain. The maximum supported message length 
supported by Windows is *256 characters*. If the request body contains the string “!!”, the ﬁrst part of the message will be 
displayed as a title. The maximum length of the title supported by Windows is *64 characters*.

##Prerequisites

	- Windows Vista or higher
	- [Mingw32](http://mingw.org) compiler and build tools

© 2017 Kaia Limited. All rights reserved.
