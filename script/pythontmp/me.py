#!/usr/bin/env python
import mechanize
import cookielib

# Browser
br = mechanize.Browser()

# Cookie Jar
cj = cookielib.LWPCookieJar()
br.set_cookiejar(cj)

# Browser options
br.set_handle_equiv(True)
br.set_handle_gzip(True)
br.set_handle_redirect(True)
br.set_handle_referer(True)
br.set_handle_robots(False)

# Follows refresh 0 but not hangs on refresh > 0
br.set_handle_refresh(mechanize._http.HTTPRefreshProcessor(), max_time=1)

# Want debugging messages?
#br.set_debug_http(True)
#br.set_debug_redirects(True)
#br.set_debug_responses(True)

# User-Agent (this is cheating, ok?)
br.addheaders = [('User-agent', 'Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9.0.1) Gecko/2008071615 Fedora/3.0.1-1.fc9 Firefox/3.0.1')]


r = br.open('http://meituan.com')
html = r.read()
print html

print br.title()
print r.info()

br.select_form(nr=0)


br.add_password('https://passport.meituan.com/account/signin?continue=http://www.meituan.com/account/setssid?continue=http%253A%252F%252Fbj.meituan.com%252F&service=www', 'baotiao@gmail.com', 'chenzongzhi')

result = br.open('http://www.meituan.com/account/points')
# print result.read()

req = br.find_link(text='a')
br.open(req)
print br.response().read()

# br.find_link(text='Weekend codes')


# br.form['q']='weekend codes'
# br.sumit()
# print br.response().read()
