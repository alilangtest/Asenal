#!/usr/bin/env python
import cookielib, urllib2
import re
import sys,os

cj = cookielib.LWPCookieJar()
cj.revert = ('renren.cookie')
opener = urllib2.build_opener(urllib2.HTTPCookieProcessor(cj))
r = opener.open('http://www.meituan.com')
print cj
sys.exit(0)

homeurl = 'http://www.baidu.com'
content = urllib2.urlopen(homeurl).info()
print content
print cj
sys.exit(0)
rex = r'<input type="hidden" name="([^"]+)" id="[^"]+" value="([^"]*)" />'
match = re.findall(rex, content)
print match.group()
