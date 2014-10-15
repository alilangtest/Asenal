#!/usr/bin/env python
import urllib,urllib2,re,sys,os,cookielib

class login:
    header = {'User-Agent':'Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.1.6) Gecko/20091201 Firefox/3.5.6'}
    username = ''
    passwd = ''
    cookie = None
    cookiefile = './cookies'
    user = ''

    def __init__(self, username, passwd):
        self.username = username
        self.passwd = passwd
        self.cookie = cookielib.LWPCookieJar()
        opener = urllib2.build_opener(urllib2.HTTPCookieProcessor(self.cookie))
        urllib2.install_opener(opener)

    def lg(self):
        postdata = {
            'login':self.username,
            'passwd':self.passwd,
            'type':1
        }
        postdata = urllib.urlencode(postdata)

        req = urllib2.Request(url='http://www.zzmt.dev.sankuai.com/acl/account/login', data = postdata, headers = self.header)
        result = urllib2.urlopen(req).read()
        result = str(result)
        self.cookie.save(self.cookiefile)

    def acl(self):
        result = urllib2.urlopen('http://www.zzmt.dev.sankuai.com/acl/user/list')
        res = result.read()
        print res
        f = open('cookies', 'r')
        print f.read()
        print self.cookie
        #for line in res:
        #    print line
        #    match = re.findall(r'.*?data-username=\"(.*?)\".*', line)
        #    if match:
        #        print match.group(1)

l = login('13552744710', '123456')
l.lg()
l.acl()
