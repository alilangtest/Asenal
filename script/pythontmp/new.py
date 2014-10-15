#!/usr/bin/env python
#coding=utf-8


import urllib2,cookielib
import urllib
import re
import sys

cookie=cookielib.CookieJar()
opener=urllib2.build_opener(urllib2.HTTPCookieProcessor(cookie))
urllib2.install_opener(opener)
str=urllib.urlencode({'email':'baotiao@gmail.com','password':'chenzongzhi'})
login_response=urllib2.urlopen('http://www.meituan.com/account/login',str)
response = urllib2.urlopen('http://www.meituan.com/coupons/')

r2 = urllib2.urlopen('http://www.meituan.com/deal/ImSiri.html')
r3 = urllib2.urlopen('http://www.meituan.com/deal/buy/ImSiri')
r4 = urllib2.urlopen('http://www.meituan.com/lottery/return/ImSiri')
match = re.search(r'.*?357342', r4.read())
print match.group()
sys.exit(0)

for line in r4.read():
    print line
    match = re.search(r'<script>', line)
    if match:
        print match.group()
#print r4.read()
