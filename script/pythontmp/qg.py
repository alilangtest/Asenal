#!/usr/bin/env python
import urllib,urllib2,cookielib,re,os,sys

cj = cookielib.CookieJar()
opener = urllib2.build_opener(urllib2.HTTPCookieProcessor(cj))
exheaders = [("User-Agent","Mozilla/4.0 (compatible; MSIE 7.1; Windows NT 5.1; SV1)"),]
opener.addheaders=exheaders
url_login = 'http://www.meituan.com/account/login'
body = (('email','baotiao@gmail.com'), ('password','chenzongzhi'))
req1 = opener.open(url_login, urllib.urlencode(body))


response = urllib2.urlopen('http://www.meituan.com/coupons/')
print response.info()
print response.read()
