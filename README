# PasswordSender

PasswordSender is a simple FastCGI web app to put behind Apache or Nginx (although Nginx support needs minor tweaks, see below), which allows you to send passwords (or other secrets) and files securely. The secret text is stored only in RAM on the server, and any files are stored encrypted on disk. The AES encryption key is also only stored in RAM. That means that restarting the service loses any unopened secrets, but that's the price you pay.

The secrets expire after a few days, or after an hour after opening it.

It's provided as is, and was conceived of mostly for the fun of making a C++ web app.

## Building

PasswordSender was developed using Qt 5.9.5 [Vmime](https://www.vmime.org/) 0.9.2 and openssl 1.1. The Qt version was merely chosen because the development server was Ubuntu 18.04. There are no foreseeable breaking changes with newer releases.

Once Qt is installed (in this example your home dir, using the online installer), building should consist of almost nothing more than:

```
export CMAKE_PREFIX_PATH=/home/you/Qt/5.9.5/gcc_64
cd <projectdir>
mkdir build
cd build
cmake ..
make
```

Or you can use the QtCreator kits.

## Apache

Example Apache config:

```
<VirtualHost *:443>
        RewriteEngine on
        ServerName passwords.foobar.com

        SSLEngine on
        SSLCertificateFile    /bla/cert.pem
        SSLCertificateKeyFile /bla/privkey.pem
        SSLCertificateChainFile /bla/chain.pem

        ProxyPass "/passwordsender/" "fcgi://localhost:8000/"

        DocumentRoot /var/www/html/password_sender

        ErrorLog ${APACHE_LOG_DIR}/passwords.foobar.com/error.log
        CustomLog ${APACHE_LOG_DIR}/passwords.foobar.com/access.log combined
</VirtualHost>
```

## Nginx

Nginx sends different FastCGI parameters, to the code will likely not work as is. I didn't have an Nginx server on hand, so I left a TODO in the code that it properly detects both.

