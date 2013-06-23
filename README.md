chpass.cgi
==========

Kerberos password change CGI for OpenBSD

author
======

Chris Maxwell @WrathOfChris <chris@wrathofchris.com>

requirements
============

Requires mod_auth_kerb or another authentication plugin such as cosign.

install
=======

	make obj
	make
	sudo make install

then copy into apache directory:

	sudo cp -PRp /usr/local/share/chpass /var/www/htdocs/

config
======

Customize front.html and success.html as desired.

Extract a keytab for mod_auth_kerb

	ktutil ext -k /var/www/conf/httpd.keytab http/yourdomainname.com@YOURREALM.COM

httpd.conf:

	<Directory "/var/www/htdocs/chpass">
		AuthType kerberos
		AuthName "Kerberos username and password"
		KrbMethodNegotiate off
		KrbMethodK5Passwd on
		KrbAuthoritative on
		KrbAuthRealms YOURREALM.COM
		KrbVerifyKDC on
		Krb5Keytab /var/www/conf/httpd.keytab
		KrbServiceName http
		KrbSaveCredentials on
		AllowOverride None
		Options ExecCGI
		require valid-user
	</Directory>
