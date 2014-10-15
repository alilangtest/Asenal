#!/usr/bin/env python
import urllib2,cookielib
import urllib
cookie=cookielib.CookieJar()
opener=urllib2.build_opener(urllib2.HTTPCookieProcessor(cookie))
urllib2.install_opener(opener)
str=urllib.urlencode({'login':'chenzongzhi','passwd':'123456'})
login_response=urllib2.urlopen('http://www.meituan.com/acl/account/login',str)
for line in cookie:
    print line
response = urllib2.urlopen('http://www.meituan.com/acl/user/list')
print response.read()
print cookie
